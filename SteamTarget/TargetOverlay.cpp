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

#include "TargetOverlay.h"

#include <Windows.h>
#include <dwmapi.h>
#include <SFML/Window/Event.hpp>
#include <iostream>
#include "SteamTarget.h"

bool TargetOverlay::init(bool hidden)
{
	const sf::VideoMode mode = sf::VideoMode::getDesktopMode();
	window_.create(sf::VideoMode(mode.width - 16, mode.height - 32), "GloSC_OverlayWindow");
	//Window is too large ; always 16 and 32 pixels?  - sf::Style::None breaks transparency!

	window_.setFramerateLimit(30);
	window_.setPosition({ 0, 0 });
	makeSfWindowTransparent();
	hidden_ = hidden;
	if (window_.setActive(false))
	{
		overlay_thread_ = std::thread(&TargetOverlay::overlayLoop, this);
		return true;
	}
	return false;
}

void TargetOverlay::stop()
{
	run_ = false;
	overlay_thread_.join();
}

void TargetOverlay::overlayLoop()
{
	if (window_.setActive(true))
	{

		if (hidden_)
		{
			ShowWindow(window_.getSystemHandle(), SW_HIDE);
			window_.setFramerateLimit(1); //Window is not shown anyway,
		}
		else
		{
			SetWindowPos(window_.getSystemHandle(), HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_ASYNCWINDOWPOS);
		}


		while (window_.isOpen() && run_)
		{
			mtx_.lock();
			sf::Event event{};
			while (window_.pollEvent(event))
			{
				if (event.type == sf::Event::Closed)
				{
					window_.close();
					SteamTarget::quit();
				}
			}
			window_.clear(sf::Color::Transparent);
			window_.display();
			mtx_.unlock();
		}
	}
}

void TargetOverlay::onOverlayOpened()
{
	//mtx_.lock();
	//TODO: impl

	std::cout << "Overlay opened!\n";

	//mtx_.unlock();
}

void TargetOverlay::onOverlayClosed()
{
	//mtx_.lock();
	//TODO: impl

	std::cout << "Overlay closed!\n";

	//mtx_.unlock();
}

void TargetOverlay::makeSfWindowTransparent()
{
	HWND hwnd = window_.getSystemHandle();
	SetWindowLong(hwnd, GWL_STYLE, WS_VISIBLE | WS_POPUP &~ WS_CAPTION);
	SetWindowLong(hwnd, GWL_EXSTYLE, WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOPMOST);

	MARGINS margins;
	margins.cxLeftWidth = -1;

	DwmExtendFrameIntoClientArea(hwnd, &margins);
	SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE);

	window_.clear(sf::Color::Transparent);
	window_.display();
}

void TargetOverlay::moveMouseIntoOverlay() const
{
	RECT rect = { 0,0,0,0 };
	if (GetWindowRect(window_.getSystemHandle(), &rect))
	{
		POINT cursor_pos = { 0,0 };
		GetCursorPos(&cursor_pos);
		if (PtInRect(&rect, cursor_pos))
		{
			SetCursorPos(cursor_pos.x + 1, cursor_pos.y);
		}
		else
		{
			SetCursorPos(rect.left + 16, rect.top + 16);
		}
	}
}
