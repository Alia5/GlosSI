#pragma once
#include <SFML/Window/Keyboard.hpp>
#include <unordered_map>

#define QQ(x) #x
#define QUOTE(x) QQ(x)

#define KEYCONVSF(KEY) \
    { QUOTE(KEY), sf::Keyboard::Key::KEY }

namespace keymap {
std::unordered_map<std::string, sf::Keyboard::Key> sfkey = {
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
    KEYCONVSF(Z)
};

#ifdef _WIN32
#define NOMINMAX
#include <Windows.h>
//yep.. there are smarter ways to tho this...
std::unordered_map<std::string, uint8_t> winkey = {
    {"Shift", VK_SHIFT},
    {"Alt", VK_MENU},
    {"Ctrl", VK_CONTROL},
    {"Del", VK_DELETE},
    {"Ins",VK_INSERT},
    {"Home",VK_HOME},
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
    {"KEY_Z", 0x5A}

};
#endif
}
