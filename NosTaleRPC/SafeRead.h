#pragma once
#include <Windows.h>
#include <Psapi.h>
#include <iostream>
class SafeRead
{
private:
	static DWORD start;
	static DWORD stop;
public:
	static void Init();
	static bool IsInModule(void* ptr);
	~SafeRead();
};

