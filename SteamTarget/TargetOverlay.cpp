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

bool TargetOverlay::init(bool hidden, bool overlay_only_config)
{
	const sf::VideoMode mode = sf::VideoMode::getDesktopMode();
	window_.create(sf::VideoMode(mode.width - 16, mode.height - 32), "GloSC_OverlayWindow");
	//Window is too large ; always 16 and 32 pixels?  - sf::Style::None breaks transparency!

	window_.setFramerateLimit(30);
	window_.setPosition({ 0, 0 });
	last_foreground_window_ = window_.getSystemHandle();
	makeSfWindowTransparent();
	hidden_ = hidden;
	hidden_only_config_ = overlay_only_config;
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

		loadLogo();
		if (hidden_ || hidden_only_config_)
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
			sf::Event event{};
			while (window_.pollEvent(event))
			{
				if (event.type == sf::Event::Closed)
				{
					window_.close();
					SteamTarget::quit();
				}
			}

			if (overlay_state_ == 1)
			{

				if (hidden_only_config_)
				{
					ShowWindow(window_.getSystemHandle(), SW_SHOW);
					window_.setFramerateLimit(30);
				}


				last_foreground_window_ = GetForegroundWindow();

				std::cout << "Saving current ForegorundWindow HWND: " << last_foreground_window_ << std::endl;
				std::cout << "Activating OverlayWindow" << std::endl;

				SetWindowLong(window_.getSystemHandle(), GWL_EXSTYLE, WS_EX_LAYERED); //make overlay window clickable

																					  //Actually activate the overlaywindow
				stealFocus(window_.getSystemHandle());

				//Move the mouse cursor inside the overlaywindow
				//this is neccessary because steam doesn't want to switch to big picture bindings if mouse isn't inside
				moveMouseIntoOverlay();
				overlay_state_ = 0;
				draw_logo_ = true;

			} else if (overlay_state_ == 2)
			{
				//make overlaywindow clickthrough - WS_EX_TRANSPARENT - again
				SetWindowLong(window_.getSystemHandle(), GWL_EXSTYLE, WS_EX_LAYERED | WS_EX_TRANSPARENT);

				std::cout << "Switching to previously focused window" << std::endl;

				//switch back the the previosly focused window
				stealFocus(last_foreground_window_);
				overlay_state_ = 0;
				draw_logo_ = false;

				if (hidden_only_config_)
				{
					ShowWindow(window_.getSystemHandle(), SW_HIDE);
					window_.setFramerateLimit(1); //Window is not shown anyway	
				}

			} 
			window_.clear(sf::Color::Transparent);
			if (draw_logo_)
				window_.draw(background_sprite_);
			window_.display();
		}
	}
}

void TargetOverlay::onOverlayOpened()
{
	overlay_state_ = 1;
}

void TargetOverlay::onOverlayClosed()
{
	overlay_state_ = 2;
}

void TargetOverlay::stealFocus(HWND hwnd)
{
	const DWORD dwCurrentThread = GetCurrentThreadId();
	const DWORD dwFGThread = GetWindowThreadProcessId(GetForegroundWindow(), nullptr);

	AttachThreadInput(dwCurrentThread, dwFGThread, TRUE);

	// Possible actions you may wan to bring the window into focus.
	SetForegroundWindow(hwnd);
	SetCapture(hwnd);
	SetFocus(hwnd);
	SetActiveWindow(hwnd);
	EnableWindow(hwnd, TRUE);

	AttachThreadInput(dwCurrentThread, dwFGThread, FALSE);


	sf::Clock clock;
	while (!SetForegroundWindow(hwnd) && clock.getElapsedTime().asMilliseconds() < 1000) //try to forcefully set foreground window 
	{
		Sleep(1);
	}

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

void TargetOverlay::loadLogo()
{
	HRSRC rsrcData = FindResource(NULL, L"ICOPNG", RT_RCDATA);
	DWORD rsrcDataSize = SizeofResource(NULL, rsrcData);
	HGLOBAL grsrcData = LoadResource(NULL, rsrcData);
	LPVOID firstByte = LockResource(grsrcData);
	sprite_texture_ = std::make_unique<sf::Texture>();
	sprite_texture_->loadFromMemory(firstByte, rsrcDataSize);
	background_sprite_.setTexture(*sprite_texture_);
	background_sprite_.setOrigin(sf::Vector2f(sprite_texture_->getSize().x / 2.f, sprite_texture_->getSize().y / 2));
	sf::VideoMode winSize = sf::VideoMode::getDesktopMode();
	background_sprite_.setPosition(sf::Vector2f(winSize.width / 2.f, winSize.height / 2.f));
}

