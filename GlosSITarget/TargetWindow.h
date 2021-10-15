#pragma once
#include <functional>

#include <SFML/Graphics/RenderWindow.hpp>

class TargetWindow
{
public:
    explicit TargetWindow(std::function<void()> on_close = [](){});

    void setFpsLimit(unsigned int fps_limit);
    void setClickThrough(bool click_through);
    void update();
    void close();

private:
    const std::function<void()> on_close_;
    sf::RenderWindow window_;
};

