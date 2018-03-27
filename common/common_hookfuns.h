/*
Copyright 2018 Peter Repukat - FlatspotSoftware

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/
#pragma once

#include <Windows.h>
#include <Psapi.h>

namespace hook_commons
{

//places a jmp instruction to a __declspec(naked) function on a given adress
//nops the rest of bytes to don't break any instructions
//part of patched code may has to be executed in the hook function
void PlaceJMP(BYTE * Address, DWORD jumpTo, DWORD lenght)
{
	DWORD dwOldProtect, dwBkup, dwReloadAddr;
	VirtualProtect(Address, lenght, PAGE_EXECUTE_READWRITE, &dwOldProtect);
	dwReloadAddr = (jumpTo - (DWORD)Address) - 5;								//5 == lenght of jump instruction (1byte + 4byte address)
	*Address = 0xE9; //jmp instrcution
	*((DWORD*)(Address + 0x1)) = dwReloadAddr;

	for (DWORD x = 5; x < lenght; x++)
		*(Address + x) = 0x90;							//nop the rest

	VirtualProtect(Address, lenght, dwOldProtect, &dwBkup);
}

void RestoreBytes(BYTE * Address, BYTE * original, DWORD lenght)
{
	DWORD dwOldProtect, dwBkup, dwReloadAddr;
	VirtualProtect(Address, lenght, PAGE_EXECUTE_READWRITE, &dwOldProtect);


	for (DWORD x = 0; x < lenght; x++)
	{
		*(Address + x) = *(original + x);
	}

	VirtualProtect(Address, lenght, dwOldProtect, &dwBkup);
}

MODULEINFO GetModInfo(const char * szModule)
{
	MODULEINFO ret = { NULL };
	HMODULE mod = GetModuleHandleA(szModule);

	if (mod != 0)
		GetModuleInformation(GetCurrentProcess(), mod, &ret, sizeof(MODULEINFO));
	return ret;
}

//returns memory address of given pattern ind given module
DWORD FindPattern(const char * module, const char * pattern, const char * mask)
{
	MODULEINFO mInfo = GetModInfo(module);
	DWORD baseAddr = (DWORD)mInfo.lpBaseOfDll;
	if (baseAddr == 0)
		return NULL;
	DWORD size = mInfo.SizeOfImage;

	DWORD patLenght = strlen(mask);

	for (DWORD i = 0; i < size - patLenght; i++)	//bad for loop btw...
	{
		bool found = true;
		for (DWORD j = 0; j < patLenght; j++)
			found &= mask[j] == '?' || pattern[j] == *(char*)(baseAddr + j + i);

		if (found)
			return baseAddr + i;

	}

	return NULL;
}
}