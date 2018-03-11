#pragma once
#include <Windows.h>
#include <functional>

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
