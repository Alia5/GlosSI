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
#include <thread>
#include <mutex>

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

	void makeSfWindowTransparent();
	void moveMouseIntoOverlay() const;

	std::thread overlay_thread_;
	sf::RenderWindow window_;
	std::mutex mtx_;
	bool run_ = true;
	bool hidden_ = false;
};
