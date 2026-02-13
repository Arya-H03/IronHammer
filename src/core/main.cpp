#include "main.h"
#include <SFML/Graphics.hpp>
#include <iostream>
#include "Components.hpp"
#include "ECSCommon.h"
#include "EntityManager.hpp"
#include "GUISystem.h"

int main()
{
    EntityManager entityManager;
    GUISystem guiSystem(entityManager);
    // ArchetypeChunk<CPosition, CRotation, CVelocity> chunk;
    sf::RenderWindow window(sf::VideoMode({1280, 720}), "IronHammer");
    window.setFramerateLimit(60);

    const bool isWindowInitialized = ImGui::SFML::Init(window);
    if (!isWindowInitialized) return 0;

    guiSystem.AppleGUITheme();

    // for(size_t i = 0; i < 2000; ++i)
    // {
    //     entityManager.CreateEntity(CPosition{1, 1, 1}, CVelocity(0, 100, 12));
    // }
    // for(size_t i = 0; i < 1000; ++i)
    // {
    //     entityManager.CreateEntity(CVelocity(0, 100, 12));
    // }

    // for(size_t i = 0; i < 10000; ++i)
    // {
    //     entityManager.CreateEntity(CPosition{1, 1, 1}, CVelocity(0, 100, 12), CRotation(0, 0, 0));
    // }

    Entity e0 = entityManager.CreateEntity(CPosition{1, 1, 1}, CVelocity(0, 100, 12));
    Entity e1 = entityManager.CreateEntity(CPosition{1, 1, 1}, CVelocity(0, 100, 12));
    Entity e2 = entityManager.CreateEntity(CPosition{1, 1, 1}, CVelocity(0, 100, 12));
    Entity e3 = entityManager.CreateEntity(CPosition{1, 1, 1}, CVelocity(0, 100, 12), CRotation(2, 2, 1));
    Entity e4 = entityManager.CreateEntity(CPosition{1, 1, 1}, CVelocity(0, 100, 12), CRotation(2, 2, 2));


    sf::Clock deltaClock;
    while (window.isOpen())
    {
        while (const auto event = window.pollEvent())
        {
            ImGui::SFML::ProcessEvent(window, *event);
            if (event->is<sf::Event::Closed>()) window.close();
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D))
            {
                entityManager.AddToEntity(e2, CRotation{5,4,3});
            }

        }

        ImGui::SFML::Update(window, deltaClock.restart());
        guiSystem.HandleGUISystem();
        window.clear();
        ImGui::SFML::Render(window);
        window.display();
    }
}
