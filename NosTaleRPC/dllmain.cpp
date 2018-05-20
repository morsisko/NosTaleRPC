#define _CRT_SECURE_NO_WARNINGS
#include <Windows.h>
#include <iostream>
#include "discord_rpc.h"
#include "Structures.h"
#include "SafeRead.h"
#include <ctime>

BSTR bstr;
FILE *dummyStream;
const DWORD charInfoAddr = 0x85155C;
const DWORD timespaceInformationAddr = 0x8515CC;
const DWORD waveTimerAddr = 0x8515D4;
const DWORD mapAddr = 0x851650;
const DWORD mapIdAddr = 0x6C3858;
const DWORD playerAddr = 0x851540;
const DWORD inGameAddr = 0x6C3010; //NostaleClientX.exe+2C2E8C // NostaleClientX.exe+2C2FE8

const int MAX_ITER = 30;
int currentIter = MAX_ITER;
char* MASK_ONLY_LVL = "%s - Lvl: %s";
char* MASK_WITH_AW = "%s - Lvl: %s %s";
int startTimestamp = 0;
int endTimestamp = 0;
int lastMapId = 0;
bool lastSitState = false;

static const char* APPLICATION_ID = "445187290210369547";

void ConvertToUTF8(wchar_t* in, char* out, int bufferSize = 255)
{
	WideCharToMultiByte(CP_UTF8, NULL, in, -1, out, bufferSize, NULL, NULL);
}

static void handleDiscordReady(const DiscordUser* connectedUser)
{
	printf("\nDiscord: connected to user %s#%s - %s\n",
		connectedUser->username,
		connectedUser->discriminator,
		connectedUser->userId);
}

static void handleDiscordDisconnected(int errcode, const char* message)
{
	printf("\nDiscord: disconnected (%d: %s)\n", errcode, message);
}

static void handleDiscordError(int errcode, const char* message)
{
	printf("\nDiscord: error (%d: %s)\n", errcode, message);
}

static void handleDiscordJoin(const char* secret)
{
	printf("\nDiscord: join (%s)\n", secret);
}


void Init()
{
	DiscordEventHandlers handlers;
	memset(&handlers, 0, sizeof(handlers));
	handlers.ready = handleDiscordReady;
	handlers.disconnected = handleDiscordDisconnected;
	handlers.errored = handleDiscordError;
	handlers.joinGame = handleDiscordJoin;
	Discord_Initialize(APPLICATION_ID, &handlers, 0, NULL);
}

void UpdateTimestamps()
{
	TimespaceInformation* tsInfo = *(TimespaceInformation**)(timespaceInformationAddr);
	Player* player = *(Player**)(playerAddr);
	int mapId = *(int*)(mapIdAddr);

	if (tsInfo->IsInTimespace())
	{
		startTimestamp = 0;
		WaveTimer* waveTimer = *(WaveTimer**)(waveTimerAddr);
		endTimestamp = time(0) + waveTimer->GetTimeToEnd() / 10;
	}
	else if (mapId != lastMapId)
	{
		startTimestamp = time(0);
		endTimestamp = 0;
	}

	else if (player->IsSitting() && lastSitState == false)
	{
		startTimestamp = time(0);
		endTimestamp = 0;
	}

	lastMapId = mapId;
	lastSitState = player->IsSitting();
}

void PrepareForLoginScreen(DiscordRichPresence& discordPresence)
{
	discordPresence.largeImageKey = "-1";
	discordPresence.largeImageText = 0;
	discordPresence.details = 0;
	endTimestamp = 0;
	startTimestamp = 0;
	discordPresence.state = "Logging in";
}

void Update()
{
	CharacterInfo* charInfo = *(CharacterInfo**)(charInfoAddr);
	TimespaceInformation* tsInfo = *(TimespaceInformation**)(timespaceInformationAddr);
	MiniMap* miniMap = *(MiniMap**)(mapAddr);
	Player* player = *(Player**)(playerAddr);
	bool isInGame = *(bool*)(inGameAddr);

	char iconBuffer[256];
	char charInfoBuffer[256];
	char mapNameBuffer[256];
	char unicodeBuffer[256];
	char lvlBuffer[4];
	char awBuffer[8];
	char nickNameBuffer[66];

	ConvertToUTF8(charInfo->GetNickname()->GetText(), nickNameBuffer, 65);
	ConvertToUTF8(charInfo->GetLvl()->GetText(), lvlBuffer, 3);


	DiscordRichPresence discordPresence;
	memset(&discordPresence, 0, sizeof(discordPresence));


	//if (isInGame)
	//{
		//SET CURRENT ICON
		sprintf(iconBuffer, "%d", charInfo->GetIcon()->GetInformation()->GetId());
		discordPresence.largeImageKey = iconBuffer;

		//SET ICON DESCRIPTION
		if (charInfo->GetAwLvl()->HasText())
		{
			ConvertToUTF8(charInfo->GetAwLvl()->GetText(), awBuffer, 7);
			sprintf(charInfoBuffer, MASK_WITH_AW, nickNameBuffer, lvlBuffer, awBuffer);
		}
		else
			sprintf(charInfoBuffer, MASK_ONLY_LVL, nickNameBuffer, lvlBuffer);

		discordPresence.largeImageText = charInfoBuffer;


		//SET CURRENT MAP
		ConvertToUTF8(miniMap->GetName()->GetText(), unicodeBuffer);
		discordPresence.details = unicodeBuffer;

		//SET IN TIMESPACE
		if (tsInfo->IsInTimespace())
			discordPresence.state = "In Timespace";
		else if (player->IsSitting())
			discordPresence.state = "Idle";
		else
			discordPresence.state = "Playing";
	//}
	//else
	//	PrepareForLoginScreen(discordPresence);


	//discordPresence.partyId = "party1234";
	//discordPresence.partySize = 1337;
	//discordPresence.partyMax = 2137;

	discordPresence.endTimestamp = endTimestamp;
	discordPresence.startTimestamp = startTimestamp;
	Discord_UpdatePresence(&discordPresence);
}

DWORD WINAPI DLLStart(LPVOID param)
{
	SafeRead::Init();
	Init();
	while (true)
	{
		UpdateTimestamps();
		if (currentIter >= MAX_ITER)
		{
			Update();
			currentIter = 0;
			std::cout << "Sending update\n";
		}
		currentIter++;
		Discord_RunCallbacks();
		Sleep(1000);
	}
	return 0;
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
		{
			AllocConsole();
			freopen_s(&dummyStream, "CONOUT$", "wb", stdout);
			freopen_s(&dummyStream, "CONOUT$", "wb", stderr);
			freopen_s(&dummyStream, "CONIN$", "rb", stdin);
			std::cout << "In DEBUG mode\n";
			CreateThread(0, 0, DLLStart, 0, 0, 0);
			break;
		}
		case DLL_THREAD_ATTACH:
			break;
		case DLL_THREAD_DETACH:
			break;
		case DLL_PROCESS_DETACH:
		{
			Discord_Shutdown();
			std::cout << "Shut down\n";
			Sleep(5000);
			break;
		}
	}
	return TRUE;
}

