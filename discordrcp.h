// discordrcp.h
#include <windows.h>
#include <stdio.h>
#include "kload_exp.h"
#include "soft\discord-game-sdk\c\discord_game_sdk.h"

#define MODID 110
#define NAMELONG "Discord RCP 5.0.0"
#define NAMESHORT "DISCORD"

#define DEFAULT_DEBUG 1

#define BUFLEN 4096

#define DATALEN 1
#define CODELEN 1

enum 
{
	DUMMY,
};

DWORD dtaArray[][DATALEN] =
{
	// PES5 DEMO 2
	{
		0,
	},
	// PES5
	{
		0,
	},
	// WE9
	{
		0,
	},
	// WE9:LE
	{
		0,
	},
};

DWORD codeArray[][CODELEN] =
{
	{
		0,
	},
	{
		0,
	},
	{
		0,
	},
	{
		0,
	},
};

DWORD dta[DATALEN];
DWORD code[CODELEN];
