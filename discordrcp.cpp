// discordrcp.cpp

#include "discordrcp.h"
#include <iostream>
#include <sstream>
#include <ctime>
#include <string>
#include <tchar.h>
#include <chrono>

KMOD k_discord = {MODID, _T(NAMELONG), _T(NAMESHORT), DEFAULT_DEBUG};
HINSTANCE hInst;

static struct IDiscordCore *g_core = nullptr;
static struct IDiscordActivityManager *g_activity = nullptr;
static struct IDiscordApplicationManager *g_application = nullptr;
static bool g_discordInitResult;

// Game state tracking
static __int8 matchStarted = 0;
static __int8 matchStartedBuf = 0;
static __int8 matchStadyBuf = 0;
static int teamHomeScore = 0;
static int teamGuestScore = 0;
static int teamHomeEXScore = 0;
static int teamGuestEXScore = 0;
static int teamHomePenaltiesScore = 0;
static int teamGuestPenaltiesScore = 0;
static int matchMinute = 0;
static int matchStady = 0;
static TCHAR teamNameHome[50];
static TCHAR teamNameGuest[50];
static std::basic_string<TCHAR> matchStartTime;
static int64_t matchStartTimestamp = 0;

// Rate limiting
static int64_t lastUpdateTime = 0;
static const int64_t UPDATE_INTERVAL = 2; // Update every 2 seconds

// Memory addresses
const DWORD MATCH_STARTED_ADDR = 0x010CF2EC;
const DWORD TEAM_NAME_HOME_ADDR = 0x010D3C16;
const DWORD TEAM_NAME_GUEST_ADDR = 0x010D6E3E;
const DWORD TEAM_HOME_SCORE_ADDR = 0x01017B38;
const DWORD TEAM_GUEST_SCORE_ADDR = 0x01017E2C;
const DWORD MATCH_MINUTE_ADDR = 0x010D2986;
const DWORD MATCH_STADY_ADDR = 0x010D298C;

// Function declarations
bool InitDiscord();
void InitModule();
void DiscordRCPPresent(IDirect3DDevice8 *self, CONST RECT *src, CONST RECT *dest, HWND hWnd, LPVOID unused);
void UpdateGameState();
void UpdateDiscordActivity();
std::basic_string<TCHAR> GetCurrentDateTime();
bool ReadMemory(HANDLE process, DWORD address, LPVOID buffer, SIZE_T size);

// Memory reading helper function
bool ReadMemory(HANDLE process, DWORD address, LPVOID buffer, SIZE_T size) {
    SIZE_T bytesRead;
    return ReadProcessMemory(process, (LPCVOID)address, buffer, size, &bytesRead) && bytesRead == size;
}

std::basic_string<TCHAR> GetCurrentDateTime() {
    time_t now = time(0);
    struct tm timeinfo;
    TCHAR buffer[80];
    localtime_s(&timeinfo, &now);
    _tcsftime(buffer, sizeof(buffer)/sizeof(TCHAR), _T("%Y-%m-%d %H:%M:%S"), &timeinfo);
    return std::basic_string<TCHAR>(buffer);
}

void DISCORD_CALLBACK OnUpdateActivity(void *data, enum EDiscordResult result)
{
    if (result == DiscordResult_Ok)
    {
        Log(&k_discord, "OnUpdateActivity::Discord activity OK");
    }
    else
    {
        LogWithNumber(&k_discord, "OnUpdateActivity::Error updating Discord activity, error code (%d)", result);
    }
}

EXTERN_C BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
    if (dwReason == DLL_PROCESS_ATTACH)
    {
        hInst = hInstance;
        Log(&k_discord, "DllMain::Attaching DiscordPresence module...");

        switch (GetPESInfo()->GameVersion)
        {
        case gvPES6PC:
            break;
        default:
            Log(&k_discord, "DllMain::Your game version is currently not supported!");
            return false;
        }

        RegisterKModule(&k_discord);

        g_discordInitResult = InitDiscord();

        if (!g_discordInitResult)
        {

            Log(&k_discord, "DllMain::Error with discord initialization!");
            return false;
        }

        HookFunction(hk_D3D_Create, (DWORD)InitModule);
        HookFunction(hk_D3D_Present, (DWORD)DiscordRCPPresent);
    }
    else if (dwReason == DLL_PROCESS_DETACH)
    {
        Log(&k_discord, "DllMain::Detaching DiscordPresence module...");

        if (g_discordInitResult)
        {
            UnhookFunction(hk_D3D_Present, (DWORD)DiscordRCPPresent);
        }

        if (g_core)
        {
            g_core->destroy(g_core);
            g_core = nullptr;
            g_activity = nullptr;
            Log(&k_discord, "DllMain::Discord Core destroyed.");
        }

        Log(&k_discord, "DllMain::Detached.");
    }
    return TRUE;
}

bool InitDiscord()
{
    struct DiscordCreateParams params;
    DiscordCreateParamsSetDefault(&params);
    params.client_id = 1193291233565618326;
    params.flags = DiscordCreateFlags_NoRequireDiscord;
    params.event_data = nullptr;

    EDiscordResult res = DiscordCreate(DISCORD_VERSION, &params, &g_core);
    if (res != DiscordResult_Ok)
    {
        LogWithNumber(&k_discord, "DiscordCreate::Error trying to create Discord Core, code (%d)", res);
        return false;
    }

    g_activity = g_core->get_activity_manager(g_core);
    g_application = g_core->get_application_manager(g_core);

    if (!g_activity || !g_application)
    {
        Log(&k_discord, "DiscordCreate::Error trying to get activity o application manager");
        g_core->destroy(g_core);
        g_core = nullptr;
        return false;
    }

    DiscordLocale locale;

    g_application->get_current_locale(g_application, &locale);
    LogWithString(&k_discord, "DiscordCreate::Discord client locale: %s", locale);

    if (!locale)
    {
        strncpy(locale, "en-US", sizeof(locale) - 1);
        locale[sizeof(locale) - 1] = '\0';
        LogWithString(&k_discord, "DiscordCreate::Error trying to get locale from client, using default (%s)", locale);
    }

    return true;
}

void InitModule()
{
    UnhookFunction(hk_D3D_Create, (DWORD)InitModule);
    Log(&k_discord, "InitModule::Initializing Discord Rich Presence...");

    struct DiscordActivity activity;

    memset(&activity, 0, sizeof(activity));
    strncpy(activity.details, "Playing prueba discord rcp", sizeof(activity.details) - 1);
    strncpy(activity.state, "hk_D3D_Create", sizeof(activity.state) - 1);

    g_activity->update_activity(g_activity, &activity, nullptr, OnUpdateActivity);
}

void UpdateGameState() {
    HANDLE pHandle = GetCurrentProcess();
    
    // Read match state
    __int8 prevMatchStarted = matchStarted;
    ReadMemory(pHandle, MATCH_STARTED_ADDR, &matchStarted, sizeof(matchStarted));
    
    // Set timestamp only when match first starts
    if (matchStarted == 1 && prevMatchStarted == 0) {
        auto now = std::chrono::system_clock::now();
        matchStartTimestamp = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();
        matchStartTime = GetCurrentDateTime();
    }
    // Reset timestamp when match ends
    else if (matchStarted == 0 && prevMatchStarted == 1) {
        matchStartTimestamp = 0;
    }

    // Read scores and match info
    ReadMemory(pHandle, TEAM_HOME_SCORE_ADDR, &teamHomeScore, sizeof(teamHomeScore));
    ReadMemory(pHandle, TEAM_GUEST_SCORE_ADDR, &teamGuestScore, sizeof(teamGuestScore));
    
    // Read match minute as a byte first
    __int8 matchMinuteBuf;
    ReadMemory(pHandle, MATCH_MINUTE_ADDR, &matchMinuteBuf, sizeof(matchMinuteBuf));
    
    // Read match stady
    ReadMemory(pHandle, MATCH_STADY_ADDR, &matchStady, sizeof(matchStady));

    // Convert match minute based on match stady
    if (matchStady <= 1) {
        matchMinute = (int)matchMinuteBuf;
    } else if (matchStady < 4) {
        // Extra time
        matchMinute = (int)matchMinuteBuf + 90;
    } else if (matchStady == 4) {
        // Penalties
        matchMinute = 120;
    }
    
    // Read team names
    ReadMemory(pHandle, TEAM_NAME_HOME_ADDR, teamNameHome, sizeof(teamNameHome));
    ReadMemory(pHandle, TEAM_NAME_GUEST_ADDR, teamNameGuest, sizeof(teamNameGuest));
}

void UpdateDiscordActivity() {
    if (!g_activity) return;

    struct DiscordActivity activity;
    memset(&activity, 0, sizeof(activity));

    std::basic_stringstream<TCHAR> details;
    std::basic_stringstream<TCHAR> state;
    std::string detailsStr;
    std::string stateStr;

    if (matchStarted) {
        details << teamNameHome << _T(" ") << teamHomeScore << _T(" - ") << teamGuestScore << _T(" ") << teamNameGuest;
        
        if (matchStady == 0) { // Regular time
            state << _T("Match Time: ") << matchMinute << _T("'");
        } else if (matchStady == 1) { // Extra time
            state << _T("Extra Time: ") << matchMinute << _T("'");
        } else if (matchStady == 2) { // Penalties
            state << _T("Penalties: ") << teamHomePenaltiesScore << _T(" - ") << teamGuestPenaltiesScore;
        }
    } else {
        details << _T("In Menus");
        state << _T("Setting up match");
    }

    // Convert wide strings to narrow strings for Discord API
    #ifdef UNICODE
        std::wstring wDetails = details.str();
        std::wstring wState = state.str();
        detailsStr = std::string(wDetails.begin(), wDetails.end());
        stateStr = std::string(wState.begin(), wState.end());
    #else
        detailsStr = details.str();
        stateStr = state.str();
    #endif

    strncpy(activity.details, detailsStr.c_str(), sizeof(activity.details) - 1);
    strncpy(activity.state, stateStr.c_str(), sizeof(activity.state) - 1);
    
    // Set timestamps only if we have a valid match timestamp
    if (matchStartTimestamp > 0) {
        activity.timestamps.start = matchStartTimestamp;
    }

    // Set assets with appropriate keys and hover text
    if (matchStarted) {
        strncpy(activity.assets.large_image, "large_image", sizeof(activity.assets.large_image) - 1);
        strncpy(activity.assets.large_text, "Pro Evolution Soccer 6", sizeof(activity.assets.large_text) - 1);
        strncpy(activity.assets.small_image, "match_icon", sizeof(activity.assets.small_image) - 1);
        strncpy(activity.assets.small_text, "In Match", sizeof(activity.assets.small_text) - 1);
    } else {
        strncpy(activity.assets.large_image, "large_image", sizeof(activity.assets.large_image) - 1);
        strncpy(activity.assets.large_text, "Pro Evolution Soccer 6", sizeof(activity.assets.large_text) - 1);
        strncpy(activity.assets.small_image, "menu_icon", sizeof(activity.assets.small_image) - 1);
        strncpy(activity.assets.small_text, "In Menus", sizeof(activity.assets.small_text) - 1);
    }

    g_activity->update_activity(g_activity, &activity, nullptr, OnUpdateActivity);
}

void DiscordRCPPresent(IDirect3DDevice8 *self, CONST RECT *src, CONST RECT *dest, HWND hWnd, LPVOID unused)
{
    if (!g_core || !g_activity) return;

    // Get current time
    auto now = std::chrono::system_clock::now();
    int64_t currentTime = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();

    // Only update if enough time has passed
    if (currentTime - lastUpdateTime >= UPDATE_INTERVAL) {
        UpdateGameState();
        UpdateDiscordActivity();
        lastUpdateTime = currentTime;
    }

    // Always run callbacks as they're lightweight
    g_core->run_callbacks(g_core);
}
