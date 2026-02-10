#include "main.h"
#include <SFML/Graphics.hpp>
#include "Components.hpp"
#include "ECSCommon.h"
#include "EntityManager.hpp"
#include "GUISystem.h"

int main()
{
    // ArchetypeChunk<CPosition, CRotation, CVelocity> chunk;
    sf::RenderWindow window(sf::VideoMode({1280, 720}), "IronHammer");
    window.setFramerateLimit(60);

    const bool isWindowInitialized = ImGui::SFML::Init(window);
    if (!isWindowInitialized) return 0;


    EntityManager entityManager;
    GUISystem guiSystem(entityManager);

    Entity e0 = entityManager.CreateEntity(CPosition{1, 1, 1}, CVelocity(0, 100, 12));
    Entity e2 = entityManager.CreateEntity(CPosition{1, 1, 1}, CVelocity(0, 100, 12));
    Entity e4 = entityManager.CreateEntity(CPosition{1, 1, 1}, CVelocity(0, 100, 12));
    Entity e5 = entityManager.CreateEntity(CPosition{1, 1, 1}, CVelocity(0, 100, 12));
    Entity e1 = entityManager.CreateEntity(CPosition{1, 1, 1}, CVelocity(0, 100, 12), CRotation(0, 0, 0));
    Entity e6 = entityManager.CreateEntity(CPosition{1, 1, 1}, CVelocity(0, 100, 12), CRotation(0, 0, 0));
    Entity e7 = entityManager.CreateEntity(CPosition{1, 1, 1}, CVelocity(0, 100, 12), CRotation(0, 0, 0));
    Entity e3 = entityManager.CreateEntity(CPosition{1, 1, 1}, CVelocity(0, 100, 12), CRotation(0, 0, 0));
    Entity e9 = entityManager.CreateEntity(CPosition{1, 1, 1}, CVelocity(0, 100, 12), CRotation(0, 0, 0));

    sf::Clock deltaClock;
    while (window.isOpen())
    {
        while (const auto event = window.pollEvent())
        {
            ImGui::SFML::ProcessEvent(window, *event);
            if (event->is<sf::Event::Closed>()) window.close();
        }

        ImGui::SFML::Update(window, deltaClock.restart());
        guiSystem.HandleGUISystem();
        window.clear();
        ImGui::SFML::Render(window);
        window.display();
    }
}
