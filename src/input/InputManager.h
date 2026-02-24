#pragma once

#include "input/InputSystem.h"
#include <SFML/Window/Keyboard.hpp>
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

    struct InputAction
    {
        sf::Keyboard::Key key;
        std::unordered_map<InputTrigger, std::vector<InputActionCallBack>> callbacks;
    };

    InputSystem& m_inputSystem;
    std::unordered_map<std::string, InputAction> m_inputActionMap;

    void RegisterAction(const std::string& actionName, sf::Keyboard::Key actionKey) { m_inputActionMap[actionName] = InputAction { actionKey }; }

    void Invoke(InputAction& inputAction, InputTrigger inputTrigger)
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

    InputManager(InputSystem& inputSystem) : m_inputSystem(inputSystem) { }

    void BindToAction(const std::string& actionName, InputTrigger inputTrigger, InputActionCallBack callBack)
    {
        auto it = m_inputActionMap.find(actionName);
        if (it == m_inputActionMap.end()) return;
        it->second.callbacks[inputTrigger].push_back(callBack);
    }

    void CreateInputAction(const std::string& name, sf::Keyboard::Key key, InputTrigger inputTrigger, InputActionCallBack callback)
    {
        RegisterAction(name, key);
        BindToAction(name, inputTrigger, callback);
    }

    void Update()
    {
        for (auto& [name, inputAction] : m_inputActionMap)
        {
            if (m_inputSystem.IsPressed(inputAction.key))
            {
                Invoke(inputAction, InputTrigger::Pressed);
            }
            if (m_inputSystem.IsReleased(inputAction.key))
            {
                Invoke(inputAction, InputTrigger::Released);
            }
            if (m_inputSystem.IsHeld(inputAction.key))
            {
                Invoke(inputAction, InputTrigger::Held);
            }
        }
    };
};
