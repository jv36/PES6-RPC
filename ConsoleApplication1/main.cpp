#include <iostream>
#include <fstream>
#include <sstream>
#include "windows.h"
#include <tlhelp32.h>
#include <tchar.h>
#include <atlstr.h>
#include <signal.h>
#include <ctime>

// deprecation warnings
#define _CRT_SECURE_NO_WARNINGS

using namespace std;
#define MAX_OF_PLAYED_PLAYERS 18
#define MAX_OF_ROSTER_PLAYERS 32
#define NOT_PLAYED -1
#define VERSION_APP "1.4"
#define HALF_1 0
#define HALF_2 1
#define EX_HALF_1 2
#define EX_HALF_2 3


inline LPCVOID ToAddress(DWORD addr) {
	#ifdef _WIN64
		return reinterpret_cast<LPCVOID>(static_cast<DWORD64>(addr));
	#else
		return reinterpret_cast<LPCVOID>(addr);
	#endif
}

HANDLE OpenProcessByName(LPCTSTR name, DWORD dwAccess);
void setPlayersStats(HANDLE pHandle, DWORD baseAddr, int* arr, int* playedPlayers);
BOOL setPlayers(HANDLE pHandle, DWORD addr, __int16* arr);
BOOL setTeamScore(HANDLE pHandle, DWORD addr, int* dst);
BOOL setTeamExtraTimeScore(HANDLE pHandle, DWORD half1addr, DWORD half2addr, int* dst);
void print(int* arr, int len);
BOOL setPlayedPlayers(HANDLE pHandle, DWORD addr, int* playedPlayersHome, int* playedPlayersGuest);
void createFileWithStats();
string currentDateTime();
void setTeamName(HANDLE pHandle, DWORD addr, char* name);
void setPlayersDribbleDistances(HANDLE pHandle, DWORD baseAddr, int* arr, int* playedPlayers);
void setPlayersStatsOffset68(HANDLE pHandle, DWORD baseAddr, int* arr, int* playedPlayers);
void setPlayersInjuries(HANDLE pHandle, DWORD baseAddr, int* arr, int* playedPlayers);
void setPlayersNames(HANDLE pHandle, DWORD addr, string* arr);
void setRealPlayerName(HANDLE pHandle, DWORD addr, string* name);

__int8 matchStarted = 0;
__int8 matchStartedBuf = 0;
__int8 matchStadyBuf = 0;
__int16 playersHome[32];
__int16 playersGuest[32];
int playersHomeGoals[32] = { 0 };
int playersGuestGoals[32] = { 0 };
int playersHomeAssists[32] = { 0 };
int playersGuestAssists[32] = { 0 };
int playersHomeYellowCards[32] = { 0 };
int playersGuestYellowCards[32] = { 0 };
int playersHomeRedCards[32] = { 0 };
int playersGuestRedCards[32] = { 0 };
int playedPlayersHome[18] = { 0 };
int playedPlayersGuest[18] = { 0 };
int playersHomeDribbleDistances[32] = { 0 };
int playersGuestDribbleDistances[32] = { 0 };
int playersHomeFouls[32] = { 0 };
int playersGuestFouls[32] = { 0 };
int playersHomeIntercepts[32] = { 0 };
int playersGuestIntercepts[32] = { 0 };
int playersHomeClearedPasses[32] = { 0 };
int playersGuestClearedPasses[32] = { 0 };
int playersHomePossessionOfBall[32] = { 0 };
int playersGuestPossessionOfBall[32] = { 0 };
int playersHomeInjuries[32] = { 0 };
int playersGuestInjuries[32] = { 0 };
int playersHomeTotalShots[32] = { 0 };
int playersGuestTotalShots[32] = { 0 };
int playersHomeShotsOnTarget[32] = { 0 };
int playersGuestShotsOnTarget[32] = { 0 };
string playersHomeNames[32];
string playersGuestNames[32];
int teamHomeScore = 0;
int teamHomeEXScore = 0;
int teamHomePenaltiesScore = 0;
int teamGuestScore = 0;
int teamGuestEXScore = 0;
int teamGuestPenaltiesScore = 0;
string matchStartTime;
char* teamNameHome = new char[50];
char* teamNameGuest = new char[50];
stringstream fileName;
BOOL isTeamPlayersReaded = false;
int matchMinute;
int matchStady;
string realPlayerHomeNickName;
string realPlayerGuestNickName;
std::ofstream out;

int main(int argc, char** argv) {
	HANDLE pHandle;
	TCHAR gameName [50];
	const DWORD matchStartedAddr = 0x010CF2EC;
	const DWORD playersHomeAddr = 0x010D4536;
	const DWORD playersGuestAddr = 0x010D775E;
	const DWORD playersHomeGoalsAddr = 0x01017B5C;
	const DWORD playersHomeAssistsAddr = 0x01017B60;
	const DWORD playersGuestGoalsAddr = 0x01017E50;
	const DWORD playersGuestAssistsAddr = 0x01017E54;
	const DWORD compoundsAddr = 0x01018DE8;
	const DWORD teamNameHomeAddr = 0x010D3C16;
	const DWORD teamNameGuestAddr = 0x010D6E3E;
	const DWORD playersHomeYellowCardsAddr = 0x01017B70;
	const DWORD playersGuestYellowCardsAddr = 0x01017E64;
	const DWORD playersHomeRedCardsAddr = 0x01017B71;
	const DWORD playersGuestRedCardsAddr = 0x01017E65;
	const DWORD teamHomeScoreAddr = 0x01017B38;
	const DWORD teamHomeEX1HalfScoreAddr = 0x01017B34;
	const DWORD teamHomeEX2HalfScoreAddr = 0x01017B36;
	const DWORD teamHomePenaltiesScoreAddr = 0x01017B3C;
	const DWORD teamGuestScoreAddr = 0x01017E2C;
	const DWORD teamGuestEX1HalfScoreAddr = 0x01017E28;
	const DWORD teamGuestEX2HalfScoreAddr = 0x01017E2A;
	const DWORD teamGuestPenaltiesScoreAddr = 0x01017E30;
	const DWORD matchMinuteAddr = 0x010D2986;
	const DWORD matchStadyAddr = 0x010D298C;
	const DWORD playersHomeDribbleDistancesAddr = 0x010CF660;
	const DWORD playersGuestDribbleDistancesAddr = 0x010CFDB0;
	const DWORD playersHomeFoulsAddr = 0x01017B68;
	const DWORD playersGuestFoulsAddr = 0x01017E5C;
	const DWORD playersHomeInterceptsAddr = 0x10CF65C;
	const DWORD playersGuestInterceptsAddr = 0x010CFDAC;
	const DWORD playersHomeNamesAddr = 0x03BCF688;
	const DWORD playersGuestNamesAddr = 0x03BD5F88;
	const DWORD playersHomeClearedPassesAddr = 0x10CF650;
	const DWORD playersGuestClearedPassesAddr = 0x10CFDA0;
	const DWORD playersHomePossessionOfBallAddr = 0x10CF63E;
	const DWORD playersGuestPossessionOfBallAddr = 0x10CFD8E;
	const DWORD playersHomeInjuriesAddr = 0x3BCF631;
	const DWORD playersGuestInjuriesAddr = 0x3BD5F31;
	const DWORD realPlayerHomeNicknameAddr = 0x3BE6BE0;
	const DWORD realPlayerGuestNicknameAddr = 0x3BE6C0E;
	const DWORD playersHomeTotalShotsAddr = 0x1017B62;
	const DWORD playersGuestTotalShotsAddr = 0x1017E56;
	const DWORD playersHomeShotsOnTargetAddr = 0x1017B66;
	const DWORD playersGuestShotsOnTargetAddr = 0x1017E5A;


	USES_CONVERSION;
	_tcscpy(gameName, A2T("pes6.exe"));

	pHandle = OpenProcessByName(gameName, PROCESS_VM_READ);
	if (pHandle == INVALID_HANDLE_VALUE || pHandle == NULL) {
		cout << "Error by open game process" << endl;
		cout << "Error code : " << GetLastError() << endl;
		return -1;
	}
	cout << "Process opened!" << endl;

	char gameDir[MAX_PATH] = { 0 };
	GetModuleFileNameA(NULL, gameDir, MAX_PATH);
	PathRemoveFileSpecA(gameDir);

	stringstream ss;
	ss << gameDir << "\\stats\\";

	const string dir = ss.str();

	int result = CreateDirectoryA(dir.c_str(), NULL);
	if (result == 0 && GetLastError() != ERROR_ALREADY_EXISTS) {
		cout << "Error when create folder: code " << GetLastError() << endl;
		return -1;
	}

	ss << "\\log\\";
	string logPath = ss.str();
	result = CreateDirectoryA(logPath.c_str(), NULL);
	if (result == 0 && GetLastError() != ERROR_ALREADY_EXISTS) {
		cout << "Error when create folder: code " << GetLastError() << endl;
		return -1;
	}

	ss.str("");
	ss << logPath << "log_" << currentDateTime().c_str();
	out.open(ss.str(), std::ofstream::out | std::ofstream::app);

	BOOL allowReading = true;
	while (true) {
		Sleep(2000);

		if (!ReadProcessMemory(pHandle, (LPCVOID)matchStartedAddr, &matchStarted, 1, NULL)) {
			cout << "Error by reading process - code : " << GetLastError() << endl;
			return -1;
		}

		__int8 buf2;
		if (!ReadProcessMemory(pHandle, (LPCVOID)matchStadyAddr, &buf2, 1, NULL)) {
			cout << "Error by reading process - code : " << GetLastError() << endl;
			out << "Error by reading process - code : " << GetLastError() << endl;
			return -1;
		}
		matchStady = (int)buf2;

		if (matchStarted != 1) {
			if (matchStartedBuf == 1) {
				cout << "Match ended" << endl;
				out << "Match ended" << endl;
			}
			if (matchStartTime.length() == 0) {
				matchStartTime = currentDateTime();
			}
		}

		if ((matchStartedBuf == 0 && matchStarted == 1) || (matchStadyBuf > 0 && matchStady == 0)) {
			
			matchStartTime = currentDateTime();
			allowReading = true;
			isTeamPlayersReaded = false;
		}

		matchStartedBuf = matchStarted;
		matchStadyBuf = matchStady;

		if (!allowReading) {
			continue;
		}

		if (matchStarted != 1) {
			allowReading = false;
		}

		__int8 buf1;
		if (!ReadProcessMemory(pHandle, (LPCVOID)matchMinuteAddr, &buf1, 1, NULL)) {
			cout << "Error by reading process - code : " << GetLastError() << endl;
			out << "Error by reading process - code : " << GetLastError() << endl;
			return -1;
		}

		if (matchStady <= 1) {
			matchMinute = (int)buf1;
		} else if (matchStady < 4) {
			matchMinute = (int)buf1 + 90;
		} else if (matchStady == 4) {
			matchMinute = 120;
		}
		cout << "Match minute " << matchMinute << endl;
		out << "Match minute " << matchMinute << endl;

		cout << "Match half " << matchStady << endl;
		out << "Match half " << matchStady << endl;

		setTeamName(pHandle, teamNameHomeAddr, teamNameHome);
		setTeamName(pHandle, teamNameGuestAddr, teamNameGuest);
		if (string(teamNameHome) == string(teamNameGuest) && string(teamNameHome) == string("")) {
			cout << "Empty team names" << endl;
			out << "Empty team names" << endl;
			continue;
		}
		if (!setPlayers(pHandle, playersHomeAddr, playersHome)) {
			continue;
		}
		if (!setPlayers(pHandle, playersGuestAddr, playersGuest)) {
			continue;
		}

		if (!setPlayedPlayers(pHandle, compoundsAddr, playedPlayersHome, playedPlayersGuest)) {
			continue;
		}

		if (!isTeamPlayersReaded) {
			setPlayersNames(pHandle, playersHomeNamesAddr, playersHomeNames);
			setPlayersNames(pHandle, playersGuestNamesAddr, playersGuestNames);
			isTeamPlayersReaded = true;
		}

		if (!setTeamScore(pHandle, teamHomeScoreAddr, &teamHomeScore)) {
			continue;
		}
		if (!setTeamScore(pHandle, teamHomePenaltiesScoreAddr, &teamHomePenaltiesScore)) {
			continue;
		}
		if (!setTeamExtraTimeScore(pHandle, teamHomeEX1HalfScoreAddr, teamHomeEX2HalfScoreAddr, &teamHomeEXScore)) {
			continue;
		}
		if (!setTeamScore(pHandle, teamGuestScoreAddr, &teamGuestScore)) {
			continue;
		}
		if (!setTeamExtraTimeScore(pHandle, teamGuestEX1HalfScoreAddr, teamGuestEX2HalfScoreAddr, &teamGuestEXScore)) {
			continue;
		}
		if (!setTeamScore(pHandle, teamGuestPenaltiesScoreAddr, &teamGuestPenaltiesScore)) {
			continue;
		}

		setPlayersStats(pHandle, playersHomeGoalsAddr, playersHomeGoals, playedPlayersHome);
		setPlayersStats(pHandle, playersGuestGoalsAddr, playersGuestGoals, playedPlayersGuest);

		setPlayersStats(pHandle, playersHomeAssistsAddr, playersHomeAssists, playedPlayersHome);
		setPlayersStats(pHandle, playersGuestAssistsAddr, playersGuestAssists, playedPlayersGuest);

		setPlayersStats(pHandle, playersHomeYellowCardsAddr, playersHomeYellowCards, playedPlayersHome);
		setPlayersStats(pHandle, playersGuestYellowCardsAddr, playersGuestYellowCards, playedPlayersGuest);
		setPlayersStats(pHandle, playersHomeRedCardsAddr, playersHomeRedCards, playedPlayersHome);
		setPlayersStats(pHandle, playersGuestRedCardsAddr, playersGuestRedCards, playedPlayersGuest);

		setPlayersStats(pHandle, playersHomeFoulsAddr, playersHomeFouls, playedPlayersHome);
		setPlayersStats(pHandle, playersGuestFoulsAddr, playersGuestFouls, playedPlayersGuest);

		setPlayersStats(pHandle, playersHomeTotalShotsAddr, playersHomeTotalShots, playedPlayersHome);
		setPlayersStats(pHandle, playersGuestTotalShotsAddr, playersGuestTotalShots, playedPlayersGuest);

		setPlayersStats(pHandle, playersHomeShotsOnTargetAddr, playersHomeShotsOnTarget, playedPlayersHome);
		setPlayersStats(pHandle, playersGuestShotsOnTargetAddr, playersGuestShotsOnTarget, playedPlayersGuest);

		setPlayersInjuries(pHandle, playersHomeInjuriesAddr, playersHomeInjuries, playedPlayersHome);
		setPlayersInjuries(pHandle, playersGuestInjuriesAddr, playersGuestInjuries, playedPlayersGuest);

		setRealPlayerName(pHandle, realPlayerHomeNicknameAddr, &realPlayerHomeNickName);
		cout << "Home: " << teamHomeScore << endl;
		setRealPlayerName(pHandle, realPlayerGuestNicknameAddr, &realPlayerGuestNickName);
		cout << "Guest: " << teamGuestScore << endl;

		fileName.str("");
		fileName.clear();
		if (realPlayerHomeNickName != string("") && realPlayerGuestNickName != string("")) {
			fileName << dir << string(teamNameHome).c_str() << " (" << realPlayerHomeNickName.c_str() << ")" << " - " << string(teamNameGuest).c_str() << " (" << realPlayerGuestNickName.c_str() << ") " << matchStartTime.c_str() << ".dat";
		} else {
			fileName << dir << string(teamNameHome).c_str() << " - " << string(teamNameGuest).c_str() << " " << matchStartTime.c_str() << ".dat";
		}
		
		cout << "Current match " << string(teamNameHome) << " - " << string(teamNameGuest) << " " << matchStartTime << endl;
		out << "Current match " << string(teamNameHome) << " - " << string(teamNameGuest) << " " << matchStartTime << endl;
		out << "Home real player: " << realPlayerHomeNickName;
		out << "Guest real player: " << realPlayerGuestNickName;

		// only after halftime
		if (matchStady > 0) {
			setPlayersDribbleDistances(pHandle, playersHomeDribbleDistancesAddr, playersHomeDribbleDistances, playedPlayersHome);
			setPlayersDribbleDistances(pHandle, playersGuestDribbleDistancesAddr, playersGuestDribbleDistances, playedPlayersGuest);

			setPlayersStatsOffset68(pHandle, playersHomeInterceptsAddr, playersHomeIntercepts, playedPlayersHome);
			setPlayersStatsOffset68(pHandle, playersGuestInterceptsAddr, playersGuestIntercepts, playedPlayersGuest);

			setPlayersStatsOffset68(pHandle, playersHomeClearedPassesAddr, playersHomeClearedPasses, playedPlayersHome);
			setPlayersStatsOffset68(pHandle, playersGuestClearedPassesAddr, playersGuestClearedPasses, playedPlayersGuest);

			setPlayersStatsOffset68(pHandle, playersHomePossessionOfBallAddr, playersHomePossessionOfBall, playedPlayersHome);
			setPlayersStatsOffset68(pHandle, playersGuestPossessionOfBallAddr, playersGuestPossessionOfBall, playedPlayersGuest);
		} else {
			ZeroMemory(playersHomeDribbleDistances, 128);
			ZeroMemory(playersGuestDribbleDistances, 128);
			ZeroMemory(playersHomeIntercepts, 128);
			ZeroMemory(playersGuestIntercepts, 128);
			ZeroMemory(playersHomeClearedPasses, 128);
			ZeroMemory(playersGuestClearedPasses, 128);
			ZeroMemory(playersHomePossessionOfBall, 128);
			ZeroMemory(playersGuestPossessionOfBall, 128);
		}

		createFileWithStats();
	}
	return 0;
}

void createFileWithStats() {
	std::ofstream out;
	out.open(fileName.str());
	if (out.fail()) {
		cout << "open failure : " << endl;
		out << "open failure : " << endl;
		return;
	}

	// match info
	out << "Match Statistics" << endl;
	out << "================" << endl;
	out << "Date: " << matchStartTime << endl << endl;

	// Teams
	out << "Teams" << endl;
	out << "-----" << endl;
	out << "Home: " << teamNameHome << " Score: " << teamHomeScore;
	if (teamHomePenaltiesScore != 0 || teamGuestPenaltiesScore != 0) {
		out << " (Penalties: " << teamHomePenaltiesScore << ")";
	}
	if (matchStady > 1) {
		out << " (Extra Time: " << teamHomeEXScore << ")";
	}
	out << endl;

	out << "Away: " << teamNameGuest << " Score: " << teamGuestScore;
	if (teamHomePenaltiesScore != 0 || teamGuestPenaltiesScore != 0) {
		out << " (Penalties: " << teamGuestPenaltiesScore << ")";
	}
	if (matchStady > 1) {
		out << " (Extra Time: " << teamGuestEXScore << ")";
	}
	out << endl << endl;

	out << "Minutes Played: " << matchMinute << endl;
	out << "Version: " << VERSION_APP << endl << endl;

	// Players
	out << "Home Team Players" << endl;
	out << "----------------" << endl;
	int maxOfPlayedPlayers = MAX_OF_PLAYED_PLAYERS;
	int notPlayed = NOT_PLAYED;

	for (int i = 0; i < maxOfPlayedPlayers; i++) {
		if (playedPlayersHome[i] != notPlayed) {
			out << "Player " << playersHome[playedPlayersHome[i]] << " (" << playersHomeNames[playedPlayersHome[i]] << ")" << endl;
			if (playersHomeGoals[i] > 0) out << "  Goals: " << playersHomeGoals[i] << endl;
			if (playersHomeAssists[i] > 0) out << "  Assists: " << playersHomeAssists[i] << endl;
			if (playersHomeYellowCards[i] > 0) out << "  Yellow Cards: " << playersHomeYellowCards[i] << endl;
			if (playersHomeRedCards[i] > 0) out << "  Red Cards: " << playersHomeRedCards[i] << endl;
			if (playersHomeDribbleDistances[i] > 0) out << "  Dribble Distance: " << playersHomeDribbleDistances[i] << endl;
			if (playersHomeFouls[i] > 0) out << "  Fouls: " << playersHomeFouls[i] << endl;
			if (playersHomeIntercepts[i] > 0) out << "  Intercepts: " << playersHomeIntercepts[i] << endl;
			if (playersHomeClearedPasses[i] > 0) out << "  Cleared Passes: " << playersHomeClearedPasses[i] << endl;
			if (playersHomePossessionOfBall[i] > 0) out << "  Possession: " << playersHomePossessionOfBall[i] << endl;
			if (playersHomeInjuries[i] > 0) out << "  Injury Level: " << playersHomeInjuries[i] << endl;
			if (playersHomeTotalShots[i] > 0) out << "  Total Shots: " << playersHomeTotalShots[i] << endl;
			if (playersHomeShotsOnTarget[i] > 0) out << "  Shots on Target: " << playersHomeShotsOnTarget[i] << endl;
			out << endl;
		}
	}

	out << endl << "Away Team Players" << endl;
	out << "----------------" << endl;
	for (int i = 0; i < maxOfPlayedPlayers; i++) {
		if (playedPlayersGuest[i] != notPlayed) {
			out << "Player " << playersGuest[playedPlayersGuest[i]] << " (" << playersGuestNames[playedPlayersGuest[i]] << ")" << endl;
			if (playersGuestGoals[i] > 0) out << "  Goals: " << playersGuestGoals[i] << endl;
			if (playersGuestAssists[i] > 0) out << "  Assists: " << playersGuestAssists[i] << endl;
			if (playersGuestYellowCards[i] > 0) out << "  Yellow Cards: " << playersGuestYellowCards[i] << endl;
			if (playersGuestRedCards[i] > 0) out << "  Red Cards: " << playersGuestRedCards[i] << endl;
			if (playersGuestDribbleDistances[i] > 0) out << "  Dribble Distance: " << playersGuestDribbleDistances[i] << endl;
			if (playersGuestFouls[i] > 0) out << "  Fouls: " << playersGuestFouls[i] << endl;
			if (playersGuestIntercepts[i] > 0) out << "  Intercepts: " << playersGuestIntercepts[i] << endl;
			if (playersGuestClearedPasses[i] > 0) out << "  Cleared Passes: " << playersGuestClearedPasses[i] << endl;
			if (playersGuestPossessionOfBall[i] > 0) out << "  Possession: " << playersGuestPossessionOfBall[i] << endl;
			if (playersGuestInjuries[i] > 0) out << "  Injury Level: " << playersGuestInjuries[i] << endl;
			if (playersGuestTotalShots[i] > 0) out << "  Total Shots: " << playersGuestTotalShots[i] << endl;
			if (playersGuestShotsOnTarget[i] > 0) out << "  Shots on Target: " << playersGuestShotsOnTarget[i] << endl;
			out << endl;
		}
	}

	out.close();
	// cout << "Successfully create file" << endl;
	// out << "Successfully create file" << endl;
}

void setPlayersStats(HANDLE pHandle, DWORD baseAddr, int* arr, int* playedPlayers) {
	DWORD offset = 0x28;
	int maxOfPlayedPlayers = MAX_OF_PLAYED_PLAYERS;
	int notPlayed = NOT_PLAYED;
	__int8 val;
	DWORD addr;
	for (int i = 0; i < maxOfPlayedPlayers; i++) {
		if (playedPlayers[i] == notPlayed) {
			continue;
		}
		addr = baseAddr + i * offset;
		if (!ReadProcessMemory(pHandle, ToAddress(addr), &val, 1, NULL)) {
			cout << "Error by reading players stats - code : " << GetLastError() << endl;
			out << "Error by reading players stats - code : " << GetLastError() << endl;
			return;
		}
		arr[i] = (int)val;
	}
}

void setTeamName(HANDLE pHandle, DWORD addr, char* name) {
	if (!ReadProcessMemory(pHandle, ToAddress(addr), name, 50, NULL)) {
		cout << "Error by reading team name - code : " << GetLastError() << endl;
		out << "Error by reading team name - code : " << GetLastError() << endl;
	}
}

void setRealPlayerName(HANDLE pHandle, DWORD addr, string* name) {
	char buf[50];
	if (!ReadProcessMemory(pHandle, ToAddress(addr), buf, 50, NULL)) {
		cout << "Error by reading real player name - code : " << GetLastError() << endl;
		out << "Error by reading real player name - code : " << GetLastError() << endl;
	}
	*name = string(buf);
}

BOOL setPlayedPlayers(HANDLE pHandle, DWORD addr, int* playedPlayersHome, int* playedPlayersGuest) {
	int maxOfPlayedPlayers = MAX_OF_PLAYED_PLAYERS;
	int maxOfRosterPlayers = MAX_OF_ROSTER_PLAYERS;
	int notPlayed = NOT_PLAYED;
	int i = 0;
	__int8* buf = new __int8[maxOfPlayedPlayers];
	DWORD offset = 0x12;
	if (!ReadProcessMemory(pHandle, ToAddress(addr), buf, maxOfPlayedPlayers, NULL)) {
		cout << "Error by reading home played players - code : " << GetLastError() << endl;
		out << "Error by reading home played players - code : " << GetLastError() << endl;
		delete[] buf;
		return false;
	}
	int playedPlayersHomeCount = 0;
	for (i = 0; i < maxOfPlayedPlayers; i++) {
		if (buf[i] > maxOfRosterPlayers - 1 || buf[i] < notPlayed) {
			cout << "Value damaged " << buf[i] << endl;
			out << "Value damaged " << buf[i] << endl;
			delete[] buf;
			return false;
		}
		if (buf[i] != notPlayed) {
			playedPlayersHomeCount++;
		}
		playedPlayersHome[i] = (int)buf[i];
	}
	if (playedPlayersHomeCount < 11) {
		cout << "Home pLayers on a field less than 11 - " << playedPlayersHomeCount << endl;
		out << "Home pLayers on a field less than 11 - " << playedPlayersHomeCount << endl;
		delete[] buf;
		return false;
	}
	if (!ReadProcessMemory(pHandle, ToAddress(addr + offset), buf, maxOfPlayedPlayers, NULL)) {
		cout << "Error by reading guest played players - code : " << GetLastError() << endl;
		out << "Error by reading guest played players - code : " << GetLastError() << endl;
		delete[] buf;
		return false;
	}
	int playedPlayersGuestCount = 0;
	for (i = 0; i < maxOfPlayedPlayers; i++) {
		if (buf[i] > maxOfRosterPlayers || buf[i] < notPlayed) {
			cout << "Value damaged " << buf[i] <<endl;
			out << "Value damaged " << buf[i] << endl;
			delete[] buf;
			return false;
		}
		if (buf[i] != notPlayed) {
			playedPlayersGuestCount++;
		}
		playedPlayersGuest[i] = int(buf[i]);
	}
	if (playedPlayersGuestCount < 11) {
		cout << "Guest pLayers on a field less than 11 - " << playedPlayersGuestCount << endl;
		out << "Guest pLayers on a field less than 11 - " << playedPlayersGuestCount << endl;
		delete[] buf;
		return false;
	}
	delete[] buf;
	return true;
}

BOOL setPlayers(HANDLE pHandle, DWORD addr, __int16* arr) {
	__int16 buf[32];
	int maxOfRosterPlayers = MAX_OF_ROSTER_PLAYERS;
	if (!ReadProcessMemory(pHandle, ToAddress(addr), buf, maxOfRosterPlayers * 2, NULL)) {
		cout << "Error by reading team players - code : " << GetLastError() << endl;
		out << "Error by reading team players - code : " << GetLastError() << endl;
		return false;
	}
	for (int i = 0; i < maxOfRosterPlayers; i++) {
		// Check player id
		if (buf[i] < 0 || buf[i] > 5000) {
			cout << "Value of id player damaged " << buf[i] << endl;
			out << "Value of id player damaged " << buf[i] << endl;
			return false;
		}
		arr[i] = buf[i];
	}
	return true;
}

void setPlayersNames(HANDLE pHandle, DWORD baseAddr, string* arr) {
	DWORD offset = 0x348;
	int maxNameLength = 30;
	int maxOfRosterPlayers = MAX_OF_ROSTER_PLAYERS;
	char* buf = new char[maxNameLength];
	DWORD addr;
	for (int i = 0; i < maxOfRosterPlayers; i++) {
		addr = baseAddr + i * offset;
		if (!ReadProcessMemory(pHandle, ToAddress(addr), buf, maxNameLength, NULL)) {
			cout << "Error by reading team players - code : " << GetLastError() << endl;
			out << "Error by reading team players - code : " << GetLastError() << endl;
		}
		int k = 0;
		string name;
		while (buf[k] != 0) {
			name += buf[k];
			k++;
		}
		arr[i] = string(buf);
	}
	delete[] buf;
}

BOOL setTeamScore(HANDLE pHandle, DWORD addr, int* dst) {
	__int8 buf;
	if (!ReadProcessMemory(pHandle, ToAddress(addr), &buf, 1, NULL)) {
		cout << "Error by reading team score - code : " << GetLastError() << endl;
		out << "Error by reading team score - code : " << GetLastError() << endl;
	}
	if (buf < 0) {
		return false;
	}
	*dst = (int)buf;
	return true;
}

BOOL setTeamExtraTimeScore(HANDLE pHandle, DWORD half1addr, DWORD half2addr, int* dst) {
	__int8 buf;
	if (!ReadProcessMemory(pHandle, ToAddress(half1addr), &buf, 1, NULL)) {
		cout << "Error by reading team extra time score - code : " << GetLastError() << endl;
		out << "Error by reading team extra time score - code : " << GetLastError() << endl;
	}
	if (buf < 0) {
		return false;
	}
	*dst = (int)buf;
	if (!ReadProcessMemory(pHandle, ToAddress(half2addr), &buf, 1, NULL)) {
		cout << "Error by reading team score - code : " << GetLastError() << endl;
		out << "Error by reading team score - code : " << GetLastError() << endl;
	}
	if (buf < 0) {
		return false;
	}
	*dst = *dst + (int)buf;

	return true;
}

void setPlayersDribbleDistances(HANDLE pHandle, DWORD baseAddr, int* arr, int* playedPlayers) {
	DWORD offset = 0x68;
	int maxOfPlayedPlayers = MAX_OF_PLAYED_PLAYERS;
	int notPlayed = NOT_PLAYED;
	float val;
	DWORD addr;
	for (int i = 0; i < maxOfPlayedPlayers; i++) {
		if (playedPlayers[i] == notPlayed) {
			continue;
		}
		addr = baseAddr + i * offset;
		if (!ReadProcessMemory(pHandle, ToAddress(addr), &val, sizeof(float), NULL)) {
			cout << "Error by reading players stats - code : " << GetLastError() << endl;
			out << "Error by reading players stats - code : " << GetLastError() << endl;
			return;
		}
		arr[i] = (int)val;
	}
}

void setPlayersStatsOffset68(HANDLE pHandle, DWORD baseAddr, int* arr, int* playedPlayers) {
	DWORD offset = 0x68;
	int maxOfPlayedPlayers = MAX_OF_PLAYED_PLAYERS;
	int notPlayed = NOT_PLAYED;
	__int8 val;
	DWORD addr;
	for (int i = 0; i < maxOfPlayedPlayers; i++) {
		if (playedPlayers[i] == notPlayed) {
			continue;
		}
		addr = baseAddr + i * offset;
		if (!ReadProcessMemory(pHandle, ToAddress(addr), &val, 1, NULL)) {
			cout << "Error by reading players stats - code : " << GetLastError() << endl;
			out << "Error by reading players stats - code : " << GetLastError() << endl;
			return;
		}
		arr[i] = (int)val;
	}
}

void setPlayersInjuries(HANDLE pHandle, DWORD baseAddr, int* arr, int* playedPlayers) {
	DWORD offset = 0x348;
	int maxOfPlayedPlayers = MAX_OF_PLAYED_PLAYERS;
	int notPlayed = NOT_PLAYED;
	__int8 val[2];
	DWORD addr;
	for (int i = 0; i < maxOfPlayedPlayers; i++) {
		if (playedPlayers[i] == notPlayed) {
			continue;
		}
		addr = baseAddr + i * offset;
		if (!ReadProcessMemory(pHandle, ToAddress(addr), &val, 2, NULL)) {
			cout << "Error by reading players injuries - code : " << GetLastError() << endl;
			out << "Error by reading players injuries - code : " << GetLastError() << endl;
			return;
		}
		int a = (int)val[0];
		int b = (int)val[1];
		if (a + b >= 7) {
			arr[i] = 2;
		} else if (a + b > 3) {
			arr[i] = 1;
		}
		else {
			arr[i] = 0;
		}
	}
}

void print(int* arr, int len) {
	for (int i = 0; i < len; i++) {
		if (i != len - 1) {
			printf("%d, ", arr[i]);
		}
		else {
			printf("%d", arr[i]);
		}
	}
	printf("\n");
}

string currentDateTime() {
	time_t now = time(0);
	struct tm timeinfo;
	char buf[80];
	
	localtime_s(&timeinfo, &now);
	strftime(buf, sizeof(buf), "%Y-%m-%d %H-%M-%S", &timeinfo);
	
	return string(buf);
}

HANDLE OpenProcessByName(LPCTSTR name, DWORD dwAccess)
{
	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hSnap != INVALID_HANDLE_VALUE)
	{
		PROCESSENTRY32 pe;
		ZeroMemory(&pe, sizeof(PROCESSENTRY32));
		pe.dwSize = sizeof(PROCESSENTRY32);
		Process32First(hSnap, &pe);
		do
		{
			if (!lstrcmpi(pe.szExeFile, name))
			{
				cout << "Find process id " << pe.th32ProcessID << endl;
				out << "Find process id " << pe.th32ProcessID << endl;
				return OpenProcess(dwAccess, false, pe.th32ProcessID);
			}
		} while (Process32Next(hSnap, &pe));

	}
	return INVALID_HANDLE_VALUE;
}
