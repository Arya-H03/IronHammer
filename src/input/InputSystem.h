#pragma once
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Mouse.hpp>
#include <imgui-SFML.h>
#include <unordered_map>

class InputSystem
{
  private:
    struct KeyState
    {
        bool pressed = false;
        bool released = false;
        bool held = false;
    };

    sf::RenderWindow& m_window;
    std::unordered_map<sf::Keyboard::Key, KeyState> m_keyboardKeyStateMap;
    std::unordered_map<sf::Mouse::Button, KeyState> m_mouseButtonStateMap;

    void HandleEvents(const sf::Event& event)
    {
        // Mouse
        if (auto* pressed = event.getIf<sf::Event::MouseButtonPressed>())
        {
            KeyState& keyState = m_mouseButtonStateMap[pressed->button];
            if (!keyState.held) keyState.pressed = true;
            keyState.held = true;
        }
        if (auto* released = event.getIf<sf::Event::MouseButtonReleased>())
        {
            KeyState& keyState = m_mouseButtonStateMap[released->button];
            keyState.released = true;
            keyState.held = false;
        }

        // Keyboard
        if (auto* pressed = event.getIf<sf::Event::KeyPressed>())
        {
            KeyState& keyState = m_keyboardKeyStateMap[pressed->code];
            if (!keyState.held) keyState.pressed = true;
            keyState.held = true;
        }
        if (auto* released = event.getIf<sf::Event::KeyReleased>())
        {
            KeyState& keyState = m_keyboardKeyStateMap[released->code];
            keyState.released = true;
            keyState.held = false;
        }
    }

  public:
    InputSystem(sf::RenderWindow& window) : m_window(window)
    {
        // Initialize for map for all keys
        for (int i = 0; i < sf::Keyboard::KeyCount; ++i)
        {
            m_keyboardKeyStateMap[(sf::Keyboard::Key)i] = KeyState{};
        }

        m_mouseButtonStateMap[sf::Mouse::Button::Left] = KeyState{};
        m_mouseButtonStateMap[sf::Mouse::Button::Right] = KeyState{};
        m_mouseButtonStateMap[sf::Mouse::Button::Middle] = KeyState{};
    }

    bool IsPressed(sf::Mouse::Button button) const { return m_mouseButtonStateMap.at(button).pressed; }
    bool IsReleased(sf::Mouse::Button button) const { return m_mouseButtonStateMap.at(button).released; }
    bool IsHeld(sf::Mouse::Button button) const { return m_mouseButtonStateMap.at(button).held; }

    bool IsPressed(sf::Keyboard::Key key) const { return m_keyboardKeyStateMap.at(key).pressed; }
    bool IsReleased(sf::Keyboard::Key key) const { return m_keyboardKeyStateMap.at(key).released; }
    bool IsHeld(sf::Keyboard::Key key) const { return m_keyboardKeyStateMap.at(key).held; }

    void PollEvents()
    {
        while (const auto event = m_window.pollEvent())
        {
            ImGui::SFML::ProcessEvent(m_window, *event);
            if (event->is<sf::Event::Closed>()) m_window.close();
            HandleEvents(*event);
        }
    }

    void ClearEvents()
    {
        for (auto& [key, state] : m_keyboardKeyStateMap)
        {
            state.pressed = false;
            state.released = false;
        }

        for (auto& [key, state] : m_mouseButtonStateMap)
        {
            state.pressed = false;
            state.released = false;
        }
    }
};
