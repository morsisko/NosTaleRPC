#define _CRT_SECURE_NO_WARNINGS
#include <Windows.h>
#include <iostream>
#include "discord_rpc.h"
#include "DataHolder.h"
#include "Structures.h"
#include "detours.h"
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

static const char* APPLICATION_ID = "445187290210369547";

BOOL(WINAPI *oFreeLibrary)(HMODULE hModule);
FARPROC oShowNostaleSplash = NULL;
FARPROC oFreeNostaleSplash = NULL;

extern "C" __declspec(dllexport) void __declspec(naked) ShowNostaleSplash()
{
	__asm JMP oShowNostaleSplash;
}

extern "C" __declspec(dllexport) void __declspec(naked) FreeNostaleSplash()
{
	__asm JMP oFreeNostaleSplash;
}

BOOL WINAPI FreeLibrary_HOOK(HMODULE hLibModule)
{
	char aLibFileName[MAX_PATH];
	GetModuleFileNameA(hLibModule, aLibFileName, sizeof(aLibFileName));

	std::cout << "Hello from FreeLibraryHook! The hLibModuleFileNameA is equal to... " << aLibFileName << std::endl;

	if (strstr(aLibFileName, "EWSF.EWS")) {
		hLibModule = GetModuleHandleA("EWSF.dll");
	}

	return oFreeLibrary(hLibModule);
}

void HookDLL()
{
	HMODULE hLibModule = LoadLibraryA("EWSF.dll");
	oShowNostaleSplash = GetProcAddress(hLibModule, "ShowNostaleSplash");
	oFreeNostaleSplash = GetProcAddress(hLibModule, "FreeNostaleSplash");

	oFreeLibrary = FreeLibrary;

	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach(&(PVOID&)oFreeLibrary, FreeLibrary_HOOK);
	DetourTransactionCommit();
}

void ConvertToUTF8(Label* in, char* out, int bufferSize = 256)
{
	memset(out, 0, bufferSize);
	int len = in->GetLen() / 2;
	if (len < 2 || len + 1 >= bufferSize)
		strcpy(out, "??");
	else
		WideCharToMultiByte(CP_UTF8, NULL, in->GetText(), len + 1, out, bufferSize, NULL, NULL);
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
		WaveTimer* waveTimer = *(WaveTimer**)(waveTimerAddr);
		DataHolder::GetInstance().SetTimeToEnd(waveTimer->GetTimeToEnd() / 10);
	}
	else if (DataHolder::GetInstance().WasMapChanged(mapId))
	{
		DataHolder::GetInstance().ResetTimestamps();
	}
}

void Update()
{
	CharacterInfo* charInfo = *(CharacterInfo**)(charInfoAddr);
	TimespaceInformation* tsInfo = *(TimespaceInformation**)(timespaceInformationAddr);
	MiniMap* miniMap = *(MiniMap**)(mapAddr);
	Player* player = *(Player**)(playerAddr);
	bool isInGame = *(bool*)(inGameAddr);

	char buffer[256];

	ConvertToUTF8(charInfo->GetNickname(), buffer);
	DataHolder::GetInstance().SetNickname(buffer);

	ConvertToUTF8(charInfo->GetLvl(), buffer);
	DataHolder::GetInstance().SetLevel(buffer);

	if (isInGame)
	{
		//SET CURRENT ICON
		DataHolder::GetInstance().SetIconId(charInfo->GetIcon()->GetInformation()->GetId());


		//SET CURRENT MAP
		ConvertToUTF8(miniMap->GetName(), buffer);
		DataHolder::GetInstance().SetMapName(buffer);

		//SET IN TIMESPACE
		if (tsInfo->IsInTimespace())
			DataHolder::GetInstance().SetState(GameState::TIME_SPACE);
		else if (player->IsSitting())
			DataHolder::GetInstance().SetState(GameState::IDLE);
		else
			DataHolder::GetInstance().SetState(GameState::REGULAR_GAMEPLAY);
	}
	else
		DataHolder::GetInstance().SetState(GameState::LOGIN_SCREEN);

	DiscordRichPresence discordPresence = DataHolder::GetInstance().Craft();
	Discord_UpdatePresence(&discordPresence);
}

DWORD WINAPI DLLStart(LPVOID param)
{
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
			//HookDLL();
			DisableThreadLibraryCalls(hModule);
			CreateThread(0, 0, DLLStart, 0, 0, 0);
			break;
		}
		case DLL_THREAD_ATTACH:
			break;
		case DLL_THREAD_DETACH:
		{
			std::cout << "That case\n";
			break;
		}
		case DLL_PROCESS_DETACH:
		{
			std::cout << "Shut down123\n";
			Discord_Shutdown();
			std::cout << "Shut down\n";
			Sleep(5000);
			break;
		}
	}
	return TRUE;
}

