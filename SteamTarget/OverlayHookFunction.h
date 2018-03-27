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

class SteamTarget;

namespace overlay_hook
{
	DWORD JMPBackOpen;
	DWORD JMPBackClosed;
	
	TargetOverlay* target_overlay;


	__declspec(naked) void overlay_opend_hookFN()
	{


		//excute overrriden instructions 
		__asm {
			push esi
			mov byte ptr ds : [esi + 0x28], 1
		}

		//our hook code...
		target_overlay->onOverlayOpened();



		//std::cout << "Opened!\n";


		//jump back
		__asm
		{
			jmp[JMPBackOpen]											//jump back and continiue with original steam function 
		}
	}


	__declspec(naked) void overlay_closed_hookFN()
	{
		//excute overrriden instructions 
		__asm {
			mov dword ptr ds : [esi + 0x24], 0
			mov byte ptr ds : [esi + 0x28], 0
		}

		//our hook code...
		target_overlay->onOverlayClosed();

		//std::cout << "Closed!\n";

		//jump back
		__asm
		{
			jmp[JMPBackClosed]											//jump back and continiue with original steam function 
		}
	}

}
