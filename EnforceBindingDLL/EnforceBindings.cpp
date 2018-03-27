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
#include "EnforceBindings.h"
#include <vector>
#include "../common/common_hookfuns.h"

//stuff for finding the function as well as the injected code
//should probably be moved somewhere else
DWORD address = NULL;
DWORD JMPBack;
int32_t currentBindings;
const int32_t desktopBindingsID = 413080; //desktop_config appid
const int32_t bigPictureBindingsID = 413090; //big_picture_config appid
const int32_t steamChordBindingsID = 443510; //steam_chord_config appid
int32_t enforceBindingsID = 413080;

std::string fun_prolog = "\x55\x8B\xEC\x83\xEC\x10";

//////////////////////////////////  CODE  ///////////////////////////////////////////
__declspec(naked) void generalized_hookFn()
{
	//excute original function prolog
	__asm
	{
		push ebp
		mov ebp, esp
		sub esp, 0x10
	}


	//our hook code...
	__asm
	{										
		mov eax, dword ptr ss : [ebp + 0xC]						//move second function argument (bindings to set) in eax
		mov currentBindings, eax								//move bindings to set in variable
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
			mov dword ptr ss : [ebp + 0xC], eax				//patch function argument
		}
	}


	//jump back
	__asm
	{
		jmp[JMPBack]											//jump back and continiue with original steam function 
	}

}


void EnforceBindings::patchBytes()
{
	address = FindHookFunctionAdress();
	
	if (address != NULL)
	{
		JMPBack = address + fun_prolog.length();
		hook_commons::PlaceJMP((BYTE*)address, (DWORD)generalized_hookFn, fun_prolog.length());
	}
}

void EnforceBindings::Unpatch()
{
	if (address != NULL)
	{
		hook_commons::RestoreBytes((BYTE*)address, (BYTE*)fun_prolog.c_str(), fun_prolog.length());
	}
}

/*
 * Search for address of functrion we want to hook
 * 
 * In the function we want to hook the appID of the Chord bindings get referenced,
 * it takes 3 arguments (we can get that from the function prolog)
 * and it is the only one that does both. (in steamclient.dll)
 * 
 * We search for the constant appID of SteamChard bindings
 * move a bit upward and search for the correct function prolog (which will get patched in our hook function)
 * 
 */
DWORD EnforceBindings::FindHookFunctionAdress()
{
	MODULEINFO mInfo = hook_commons::GetModInfo("steamclient.dll");
	DWORD baseAddr = (DWORD)mInfo.lpBaseOfDll;
	DWORD size = mInfo.SizeOfImage;

	char pattern[4];
	pattern[3] = (steamChordBindingsID >> 24) & 0xFF;
	pattern[2] = (steamChordBindingsID >> 16) & 0xFF;
	pattern[1] = (steamChordBindingsID >> 8) & 0xFF;
	pattern[0] = steamChordBindingsID & 0xFF;

	for (DWORD i = 0; i < size - 4; i++)
	{
		bool found = true;
		for (DWORD j = 0; j < 4; j++)
			found &= pattern[j] == *(char*)(baseAddr + j + i);

		if (found)
		{
			DWORD addr = baseAddr + i;
			DWORD funStartSearchOffset = addr - 256;

			for (DWORD k = 0; k < 256; k++)
			{
				bool found2 = true;
				for (DWORD l = 0; l < fun_prolog.length(); l++)
					found2 &= fun_prolog[l] == *(char*)(funStartSearchOffset + l + k);

				if (found2)
					return funStartSearchOffset + k;

			}
		}
	}
	return NULL;
}
