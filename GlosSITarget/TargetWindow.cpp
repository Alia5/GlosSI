#include "TargetWindow.h"

#include <utility>

#include <SFML/Window/Event.hpp>

#ifdef _WIN32
#include <Windows.h>
#include <dwmapi.h>
#endif


TargetWindow::TargetWindow(std::function<void()> on_close) : on_close_(std::move(on_close))
{
	window_.create(sf::VideoMode::getDesktopMode(), "GlosSITarget", sf::Style::None);
    window_.setActive(true);
#ifdef _WIN32
	HWND hwnd = window_.getSystemHandle();
	MARGINS margins;
	margins.cxLeftWidth = -1;
	DwmExtendFrameIntoClientArea(hwnd, &margins);

	// window clickthough.
	SetWindowLong(hwnd, GWL_EXSTYLE, WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOPMOST);

	// always on top
	SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);


#endif
}

void TargetWindow::setFpsLimit(unsigned int fps_limit)
{
	window_.setFramerateLimit(fps_limit);
}

void TargetWindow::setClickThrough(bool click_through)
{
    
}


void TargetWindow::update()
{
	sf::Event event{};
	while (window_.pollEvent(event))
	{
		if (event.type == sf::Event::Closed)
		{
			window_.close();
			on_close_();
		}
	}
	window_.clear(sf::Color::Transparent);
	//window_.clear(sf::Color(255,0,0,1));
	window_.display();
}

void TargetWindow::close()
{
	window_.close();
	on_close_();
}
