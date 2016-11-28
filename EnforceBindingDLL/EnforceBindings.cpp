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
DWORD address;
DWORD JMPBack;
int32_t currentBindings;
const int32_t desktopBindingsID = 413080; //desktop_config appid
const int32_t bigPictureBindingsID = 413090; //big_picture_config appid
const int32_t steamChordBindingsID = 443510; //steam_chord_config appid
int32_t enforceBindingsID = 413080;
char originalBytes[] = "\x8B\x45\x0c\x57\x8B\x7D\x08\x3D\x76\xC4\x06\x00"; //original assembly code of steamclient.dll that we want to hook
/* == 
mov eax, dword ptr ss : [ebp + 0xc]						//appId of bindings to be switched gets moved into eax register
push edi												//part of original steam code
mov edi, dword ptr ss : [ebp + 0x8]						//part of original steam code
cmp eax, 0x6C476										//part of original steam code - checks if bindings to be set are steamchord bindings
*/
char mask[] = "xxxxxxxxxxxx";											   //mask for searching 

__declspec(naked) void enforceBindingsHookFn()
{
	__asm
	{
		mov eax, dword ptr ss : [ebp + 0xc]						//part of original steam code - appId of bindings to be switched gets moved into eax register
		mov currentBindings, eax								//move into "currentBindings" variable
	}

	if (currentBindings != desktopBindingsID					//if the current bindings aren't desktop, big picture, or steam-chord bindings
		&& currentBindings != bigPictureBindingsID				//they have to be our game bindings
		&& currentBindings != steamChordBindingsID)				//we can grab them here, because bindings switch right after we have injected and the target changes focused window
	{
		enforceBindingsID = currentBindings;
	}

	if (currentBindings == desktopBindingsID)					//if steam wants to set desktop-bindings
	{
		__asm
		{
			mov eax, enforceBindingsID							//move appid of bindings to enforce into eax register
		}
	}

	__asm
	{
		push edi												//part of original steam code
		mov edi, dword ptr ss : [ebp + 0x8]						//part of original steam code
		cmp eax, 0x6C476										//part of original steam code - checks if bindings to be set are steamchord bindings
		jmp[JMPBack]											//jump back and continiue with original steam function 
	}															//note: zero flag doesn't get altered by jmp instruction, previous compare still works fine
}
//\\\


void EnforceBindings::patchBytes()
{
	address = FindPattern("steamclient.dll", originalBytes, mask);
	if (address == NULL)
	{
		return;
	}
	JMPBack = address + 12;			//12 size of pattern/mask == patched instructions
	PlaceJMP((BYTE*)address, (DWORD)enforceBindingsHookFn, 12);
}

void EnforceBindings::Unpatch()
{
	if (address == NULL)
	{
		return;
	}
	RestoreBytes((BYTE*)address, (BYTE*)originalBytes, 12);
}







//places a jmp instruction to a __declspec(naked) function on a given adress
//nops the rest of bytes to don't break any instructions
//part of patched code may has to be executed in the hook function
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

//returns memory address of given pattern ind given module
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
