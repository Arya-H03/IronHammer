#pragma once
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <imgui-SFML.h>
#include <SFML/Graphics/RenderWindow.hpp>
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
    std::unordered_map<sf::Keyboard::Key, KeyState> m_keyStateMap;

    void HandleEvents(const sf::Event& event)
    {
        if (auto* pressed = event.getIf<sf::Event::KeyPressed>())
        {
            KeyState& keyState = m_keyStateMap[pressed->code];
            if (!keyState.held) keyState.pressed = true;
            keyState.held = true;
        }
        if (auto* released = event.getIf<sf::Event::KeyReleased>())
        {
            KeyState& keyState = m_keyStateMap[released->code];
            keyState.released = true;
            keyState.held = false;
        }
    }

  public:

    InputSystem(sf::RenderWindow& window) : m_window(window)
    {
        //Initialize for map for all keys
        for (int i = 0; i < sf::Keyboard::KeyCount; ++i)
        {
            m_keyStateMap[(sf::Keyboard::Key) i] = KeyState {};
        }
    }

    bool IsPressed(sf::Keyboard::Key key) const { return m_keyStateMap.at(key).pressed; }
    bool IsReleased(sf::Keyboard::Key key) const { return m_keyStateMap.at(key).released; }
    bool IsHeld(sf::Keyboard::Key key) const { return m_keyStateMap.at(key).held; }

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
        for (auto& [key, state] : m_keyStateMap)
        {
            state.pressed = false;
            state.released = false;
        }
    }
};
