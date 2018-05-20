#include "SafeRead.h"

DWORD SafeRead::start;
DWORD SafeRead::stop;

void SafeRead::Init()
{
	//Passing NULL is equal to the name of parent module (.exe/.dat)
	HMODULE module = GetModuleHandle(NULL);

	std::cout << "Got module of parent exe... " << module << std::endl;

	MODULEINFO info;
	GetModuleInformation(GetCurrentProcess(), module, &info, sizeof(info));

	start = (DWORD)info.lpBaseOfDll;
	stop = (DWORD)info.lpBaseOfDll + info.SizeOfImage - 1;

	std::cout << "Got module information...\n"
		<< "Base: " << info.lpBaseOfDll << std::endl
		<< "Size: " << (void*)info.SizeOfImage << std::endl
		<< "Entry point: " << info.EntryPoint << std::endl;
}

bool SafeRead::IsInModule(void * ptr)
{
	DWORD value = (DWORD)ptr;

	return value >= start && value <= stop;
}

SafeRead::~SafeRead()
{
}
