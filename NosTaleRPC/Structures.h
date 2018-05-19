#pragma once
#include <Windows.h>

#define CAN_READ_POINTER(x) if(IsBadReadPtr(this, sizeof(x))) return nullptr;
#define CAN_READ(x) if(IsBadReadPtr(this, sizeof(x))) return -1;
#define CAN_READ_BOOL(x) if(IsBadReadPtr(this, sizeof(x))) return false;

// Created with ReClass.NET by KN4CK3R
// These structures contains only MINIMAL information needed to the RPC

class IconInformation
{
private:
	char pad_0000[12]; //0x0000
	short m_sIconId; //0x000C

public:
	short GetId();
}; //Size: 0x000E

class PlayerIcon
{
private:
	char pad_0000[168]; //0x0000
	IconInformation* m_pIconInformation; //0x00A8

public:
	IconInformation* GetInformation();
}; //Size: 0x00AC

class CharacterInfo
{
private:
	char pad_0000[248]; //0x0000
	PlayerIcon* m_pPlayerIcon; //0x00F8

public:
	PlayerIcon* GetIcon();
}; //Size: 0x00FC

class TimespaceInformation
{
private:
	char pad_0000[24]; //0x0000
	bool m_bIsInTimespace; //0x0018

public:
	bool IsInTimespace();
}; //Size: 0x0019

class WaveTimer
{
private:
	char pad_0000[92]; //0x0000
	int m_iTimeToEnd; //0x005C

public:
	int GetTimeToEnd();
}; //Size: 0x0060