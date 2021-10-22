#pragma once
#include <functional>
#include <string>
#include <SFML/Graphics.hpp>

class Overlay {
  public:
    Overlay(sf::RenderWindow& window, std::function<void()> on_close);

    void setEnabled(bool enabled);
    bool toggle();
    void update();
    static void ProcessEvent(sf::Event evnt);
    static void Shutdown();
    static void ShowNotification(std::string noti_text);
  private:
    sf::RenderWindow& window_;
    sf::Clock update_clock_;
    bool enabled_ = true;
    std::function<void()> on_close_;

    bool closeButton() const;
};
