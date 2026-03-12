#pragma once

#include "input/InputSystem.h"
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Mouse.hpp>
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

enum class InputTrigger
{
    Pressed,
    Released,
    Held
};

class InputManager
{
  private:

    using InputActionCallBack = std::function<void()>;

    struct KeyboardInputAction
    {
        sf::Keyboard::Key key;
        std::unordered_map<InputTrigger, std::vector<InputActionCallBack>> callbacks;
    };

    struct MouseInputAction
    {
        sf::Mouse::Button button;
        std::unordered_map<InputTrigger, std::vector<InputActionCallBack>> callbacks;
    };

    std::unordered_map<std::string, KeyboardInputAction> m_keyboardInputActionMap;
    std::unordered_map<std::string, MouseInputAction> m_mouseInputActionMap;

    void RegisterAction(const std::string& actionName, sf::Keyboard::Key actionKey)
    {
        m_keyboardInputActionMap[actionName] = KeyboardInputAction { actionKey };
    }

    void RegisterAction(const std::string& actionName, sf::Mouse::Button actionButton)
    {
        m_mouseInputActionMap[actionName] = MouseInputAction { actionButton };
    }

    void Invoke(MouseInputAction& inputAction, InputTrigger inputTrigger)
    {
        auto it = inputAction.callbacks.find(inputTrigger);
        if (it != inputAction.callbacks.end())
        {
            for (auto& callback : it->second)
            {
                callback();
            }
        }
    }

    void Invoke(KeyboardInputAction& inputAction, InputTrigger inputTrigger)
    {
        auto it = inputAction.callbacks.find(inputTrigger);
        if (it != inputAction.callbacks.end())
        {
            for (auto& callback : it->second)
            {
                callback();
            }
        }
    }

  public:

    void BindToMouseInputAction(const std::string& actionName, InputTrigger inputTrigger, InputActionCallBack callBack)
    {
        auto it = m_mouseInputActionMap.find(actionName);
        if (it == m_mouseInputActionMap.end()) return;
        it->second.callbacks[inputTrigger].push_back(callBack);
    }

    void BindToKeyboardInputAction(const std::string& actionName, InputTrigger inputTrigger, InputActionCallBack callBack)
    {
        auto it = m_keyboardInputActionMap.find(actionName);
        if (it == m_keyboardInputActionMap.end()) return;
        it->second.callbacks[inputTrigger].push_back(callBack);
    }

    void CreateInputAction(const std::string& name, sf::Mouse::Button button, InputTrigger inputTrigger, InputActionCallBack callback)
    {
        RegisterAction(name, button);
        BindToMouseInputAction(name, inputTrigger, callback);
    }

    void CreateInputAction(const std::string& name, sf::Keyboard::Key key, InputTrigger inputTrigger, InputActionCallBack callback)
    {
        RegisterAction(name, key);
        BindToKeyboardInputAction(name, inputTrigger, callback);
    }

    void Update(InputSystem& inputSytem)
    {
        for (auto& [name, inputAction] : m_mouseInputActionMap)
        {
            if (inputSytem.IsPressed(inputAction.button))
            {
                Invoke(inputAction, InputTrigger::Pressed);
            }
            if (inputSytem.IsReleased(inputAction.button))
            {
                Invoke(inputAction, InputTrigger::Released);
            }
            if (inputSytem.IsHeld(inputAction.button))
            {
                Invoke(inputAction, InputTrigger::Held);
            }
        }

        for (auto& [name, inputAction] : m_keyboardInputActionMap)
        {
            if (inputSytem.IsPressed(inputAction.key))
            {
                Invoke(inputAction, InputTrigger::Pressed);
            }
            if (inputSytem.IsReleased(inputAction.key))
            {
                Invoke(inputAction, InputTrigger::Released);
            }
            if (inputSytem.IsHeld(inputAction.key))
            {
                Invoke(inputAction, InputTrigger::Held);
            }
        }
    };
};
