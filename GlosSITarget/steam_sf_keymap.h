/*
Copyright 2021-2022 Peter Repukat - FlatspotSoftware

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
#include <SFML/Window/Keyboard.hpp>
#include <unordered_map>

#define QQ(x) #x
#define QUOTE(x) QQ(x)

#define KEYCONVSF(KEY)                     \
    {                                      \
        QUOTE(KEY), sf::Keyboard::Key::KEY \
    }

namespace keymap {
static std::unordered_map<std::string, sf::Keyboard::Key> sfkey = {
    {"Shift", sf::Keyboard::Key::LShift},
    {"Alt", sf::Keyboard::Key::LAlt},
    {"Ctrl", sf::Keyboard::Key::LControl},
    {"Del", sf::Keyboard::Key::Delete},
    {"Ins", sf::Keyboard::Key::Insert},
    {"Home", sf::Keyboard::Key::Home},
    {"Space", sf::Keyboard::Key::Space},
    {"Backspace", sf::Keyboard::Key::Backspace},
    {"Enter", sf::Keyboard::Key::Enter},
    {"KEY_TAB", sf::Keyboard::Key::Tab},
    // TODO: more special keys with keylayout mapping... nope..
    {"KEY_0", sf::Keyboard::Key::Num0},
    {"KEY_1", sf::Keyboard::Key::Num1},
    {"KEY_2", sf::Keyboard::Key::Num2},
    {"KEY_3", sf::Keyboard::Key::Num3},
    {"KEY_4", sf::Keyboard::Key::Num4},
    {"KEY_5", sf::Keyboard::Key::Num5},
    {"KEY_6", sf::Keyboard::Key::Num6},
    {"KEY_7", sf::Keyboard::Key::Num7},
    {"KEY_8", sf::Keyboard::Key::Num8},
    {"KEY_9", sf::Keyboard::Key::Num9},
    KEYCONVSF(A),
    KEYCONVSF(B),
    KEYCONVSF(C),
    KEYCONVSF(D),
    KEYCONVSF(E),
    KEYCONVSF(F),
    KEYCONVSF(G),
    KEYCONVSF(H),
    KEYCONVSF(I),
    KEYCONVSF(J),
    KEYCONVSF(K),
    KEYCONVSF(L),
    KEYCONVSF(M),
    KEYCONVSF(N),
    KEYCONVSF(O),
    KEYCONVSF(P),
    KEYCONVSF(Q),
    KEYCONVSF(R),
    KEYCONVSF(S),
    KEYCONVSF(T),
    KEYCONVSF(U),
    KEYCONVSF(V),
    KEYCONVSF(W),
    KEYCONVSF(X),
    KEYCONVSF(Y),
    KEYCONVSF(Z),
    {"KEY_F1", sf::Keyboard::Key::F1},
    {"KEY_F2", sf::Keyboard::Key::F2},
    {"KEY_F3", sf::Keyboard::Key::F3},
    {"KEY_F4", sf::Keyboard::Key::F4},
    {"KEY_F5", sf::Keyboard::Key::F5},
    {"KEY_F6", sf::Keyboard::Key::F6},
    {"KEY_F7", sf::Keyboard::Key::F7},
    {"KEY_F8", sf::Keyboard::Key::F8},
    {"KEY_F9", sf::Keyboard::Key::F9},
    {"KEY_F10", sf::Keyboard::Key::F10},
    {"KEY_F11", sf::Keyboard::Key::F11},
    {"KEY_F12", sf::Keyboard::Key::F12}};

#ifdef _WIN32
#define NOMINMAX
#include <Windows.h>
//yep.. there are smarter ways to tho this...
static std::unordered_map<std::string, uint8_t> winkey = {
    {"Shift", VK_SHIFT},
    {"Alt", VK_MENU},
    {"Ctrl", VK_CONTROL},
    {"Del", VK_DELETE},
    {"Ins", VK_INSERT},
    {"Home", VK_HOME},
    {"Space", VK_SPACE},
    {"Backspace", VK_BACK},
    {"Enter", VK_RETURN},
    {"KEY_TAB", VK_TAB},
    // TODO: more special keys with keylayout mapping... nope..
    {"KEY_0", 0x30},
    {"KEY_1", 0x31},
    {"KEY_2", 0x32},
    {"KEY_3", 0x33},
    {"KEY_4", 0x34},
    {"KEY_5", 0x35},
    {"KEY_6", 0x36},
    {"KEY_7", 0x37},
    {"KEY_8", 0x38},
    {"KEY_9", 0x39},
    {"KEY_A", 0x41},
    {"KEY_B", 0x42},
    {"KEY_C", 0x43},
    {"KEY_D", 0x44},
    {"KEY_E", 0x45},
    {"KEY_F", 0x46},
    {"KEY_G", 0x47},
    {"KEY_H", 0x48},
    {"KEY_I", 0x49},
    {"KEY_J", 0x4A},
    {"KEY_K", 0x4B},
    {"KEY_L", 0x4C},
    {"KEY_M", 0x4D},
    {"KEY_N", 0x4E},
    {"KEY_O", 0x5F},
    {"KEY_P", 0x50},
    {"KEY_Q", 0x51},
    {"KEY_R", 0x52},
    {"KEY_S", 0x53},
    {"KEY_T", 0x54},
    {"KEY_U", 0x55},
    {"KEY_V", 0x56},
    {"KEY_W", 0x57},
    {"KEY_X", 0x58},
    {"KEY_Y", 0x59},
    {"KEY_Z", 0x5A},
    {"KEY_F1", VK_F1},
    {"KEY_F2", VK_F2},
    {"KEY_F3", VK_F3},
    {"KEY_F4", VK_F4},
    {"KEY_F5", VK_F5},
    {"KEY_F6", VK_F6},
    {"KEY_F7", VK_F7},
    {"KEY_F8", VK_F8},
    {"KEY_F9", VK_F9},
    {"KEY_F10", VK_F10},
    {"KEY_F11", VK_F11},
    {"KEY_F12", VK_F12}};
#endif
} // namespace keymap
