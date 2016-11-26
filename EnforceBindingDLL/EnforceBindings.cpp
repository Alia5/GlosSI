/*
Copyright 2016 Peter Repukat - FlatspotSoftware

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
#include "EnforceBindings.h"

//stuff for finding the function as well as the injected code
//should probably be moved somewhere else
DWORD address = 0x381FA91B;
DWORD JMPBack;
int32_t currentBindings;
const int32_t desktopBindingsID = 413080; //desktop_config appid
const int32_t bigPictureBindingsID = 413090; //desktop_config appid
const int32_t steamChordBindingsID = 443510; //desktop_config appid
int32_t enforceBindingsID = 413080; //0x80000009;
char originalBytes[] = "\x8B\x45\x0c\x57\x8B\x7D\x08";

__declspec(naked) void enforceBindingsHookFn()
{
	__asm
	{
		mov eax, dword ptr ss : [ebp + 0xc]
		mov currentBindings, eax
	}

	if (currentBindings != desktopBindingsID 
		&& currentBindings != bigPictureBindingsID 
		&& currentBindings != steamChordBindingsID)
	{
		enforceBindingsID = currentBindings;
	}

	if (currentBindings == desktopBindingsID)
	{
		__asm
		{
			mov eax, enforceBindingsID
		}
	}

	__asm
	{
		push edi
		mov edi, dword ptr ss : [ebp + 0x8]
		jmp[JMPBack]
	}
}
//\\\


void EnforceBindings::patchBytes()
{
	address = FindPattern("steamclient.dll", originalBytes, "xxxxxxx");
	JMPBack = address + 0x7;			//7 size of pattern/mask == patched instructions
	PlaceJMP((BYTE*)address, (DWORD)enforceBindingsHookFn, 7);
}

void EnforceBindings::Unpatch()
{
	RestoreBytes((BYTE*)address, (BYTE*)originalBytes, 7);
}








void EnforceBindings::PlaceJMP(BYTE * Address, DWORD jumpTo, DWORD lenght)
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

void EnforceBindings::RestoreBytes(BYTE * Address, BYTE * original, DWORD lenght)
{
	DWORD dwOldProtect, dwBkup, dwReloadAddr;
	VirtualProtect(Address, lenght, PAGE_EXECUTE_READWRITE, &dwOldProtect);		


	for (DWORD x = 0; x < lenght; x++)
	{
		*(Address + x) = *(original + x);
	}

	VirtualProtect(Address, lenght, dwOldProtect, &dwBkup);
}

MODULEINFO EnforceBindings::GetModInfo(char * szModule)
{
	MODULEINFO ret = { NULL };
	HMODULE mod = GetModuleHandleA(szModule);

	if (mod != 0)
		GetModuleInformation(GetCurrentProcess(), mod, &ret, sizeof(MODULEINFO));
	return ret;
}

DWORD EnforceBindings::FindPattern(char * module, char * pattern, char * mask)
{
	MODULEINFO mInfo = GetModInfo(module);
	DWORD baseAddr = (DWORD)mInfo.lpBaseOfDll;
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
