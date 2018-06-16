#include "DataHolder.h"

DataHolder::DataHolder()
{

}


DataHolder::~DataHolder()
{
}

DataHolder & DataHolder::GetInstance()
{
	static DataHolder instance;

	return instance;

}

void DataHolder::SetState(GameState state)
{
	if (currentState != state)
		ResetTimestamps();

	currentState = state;

	if (currentState == LOGIN_SCREEN)
	{
		largeImageKey = "-1";
		largeImageText.clear();
		details.clear();
		this->state = "Logging in";
	}

	else if (currentState == REGULAR_GAMEPLAY)
	{
		this->state = "Playing";
	}

	else if (currentState == IDLE)
	{
		this->state = "Idle";
	}

	else if (currentState == TIME_SPACE)
	{
		this->state = "In Timespace";
	}
}

void DataHolder::SetIconId(int id)
{
	largeImageKey = std::to_string(id);
}

void DataHolder::SetNickname(char * nickname)
{
	this->nickname = nickname;
}

void DataHolder::SetLevel(char * level)
{
	this->level = level;
}

void DataHolder::SetAwLevel(char * aw)
{
	this->awLevel = aw;
}

void DataHolder::SetTimeToEnd(int timeToEnd)
{
	startTimestamp = 0;
	endTimestamp = time(0) + timeToEnd;
}

bool DataHolder::WasMapChanged(int id)
{
	bool state = (id != currentMapId);

	currentMapId = id;

	return state;
}

void DataHolder::ResetTimestamps()
{
	endTimestamp = 0;
	startTimestamp = time(0);
}

void DataHolder::SetMapName(char * name)
{
	this->details = name;
}

DiscordRichPresence DataHolder::Craft()
{
	DiscordRichPresence rpc;
	std::memset(&rpc, 0, sizeof(rpc));

	std::stringstream ss;
	ss << nickname << " - Lvl: " << level << " " << awLevel;

	largeImageText = ss.str();

	rpc.largeImageKey = largeImageKey.data();
	rpc.largeImageText = largeImageText.data();
	rpc.details = details.data();
	rpc.state = state.data();
	rpc.startTimestamp = startTimestamp;
	rpc.endTimestamp = endTimestamp;

	return rpc;
}
