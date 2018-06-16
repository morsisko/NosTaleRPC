#pragma once
#include "discord_rpc.h"
#include <iostream>
#include <string>
#include <sstream>
#include <ctime>

enum GameState
{
	LOGIN_SCREEN = 0,
	REGULAR_GAMEPLAY,
	IDLE,
	TIME_SPACE
};

class DataHolder
{
private:
	DataHolder();

	GameState currentState;
	int startTimestamp;
	int endTimestamp;
	std::string largeImageKey;
	std::string largeImageText;
	std::string details;
	std::string state;
	std::string nickname;
	std::string level;
	std::string awLevel;

	int currentMapId = -1;
public:
	DataHolder(DataHolder const&) = delete;
	void operator=(DataHolder const&) = delete;
	~DataHolder();
	static DataHolder& GetInstance();
	void SetState(GameState state);
	void SetIconId(int id);
	void SetNickname(char* nickname);
	void SetLevel(char* level);
	void SetAwLevel(char* aw);
	void SetTimeToEnd(int timeToEnd);
	bool WasMapChanged(int id);
	void ResetTimestamps();
	void SetMapName(char* name);

	DiscordRichPresence Craft();
};

