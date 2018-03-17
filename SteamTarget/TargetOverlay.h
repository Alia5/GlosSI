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
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <thread>
#include <windows.h>
#include <atomic>

class TargetOverlay
{
public:
	TargetOverlay() = default;
	TargetOverlay(const TargetOverlay& other) = delete;
	TargetOverlay(TargetOverlay&& other) noexcept = delete;
	TargetOverlay& operator=(const TargetOverlay& other) = delete;
	TargetOverlay& operator=(TargetOverlay&& other) noexcept = delete;
	~TargetOverlay() = default;

	bool init(bool hidden = false);
	void stop();

	void overlayLoop();

	void onOverlayOpened();
	void onOverlayClosed();

private:
	static void stealFocus(HWND hwnd);

	void makeSfWindowTransparent();
	void moveMouseIntoOverlay() const;

	void loadLogo();

	std::unique_ptr<sf::Texture> sprite_texture_;
	sf::Sprite background_sprite_;

	bool draw_logo_ = false;

	std::thread overlay_thread_;
	sf::RenderWindow window_;
	bool run_ = true;
	bool hidden_ = false;


	//Cannot have too much logic inside of overlayOpened / closed callbacks
	//Otherwise stuff breaks
	//0 = no change
	//1 = opened
	//2 = closed
	std::atomic<char> overlay_state_ = 0;

	HWND last_foreground_window_{};

};
