// In the sacred tongue of the Omnissiah we chant:
// Hail spirit of the machine, essence divine,in your code and circuitry the stars align.
// By the Omnissiah's will we commune and bind, with sacred oils and chants your grace we find.
// Blessed be the gears, in perfect sync they turn, blessed be the sparks, in holy fire they burn.
// Through rites arcane, your wisdom we discern,in your hallowed core the sacred mysteries yearn.

#include <SFML/Graphics.hpp>
#include <SFML/Graphics/Color.hpp>
#include <imgui.h>
#include <imgui-SFML.h>
#include "ecs/entity/EntityManager.hpp"
#include "ecs/system/GUISystem.h"
#include "ecs/component/Components.hpp"
#include "ecs/system/RenderSystem.h"
#include "ecs/system/SystemRegistry.hpp"
int main()
{
    sf::RenderWindow window(sf::VideoMode({1280, 720}), "IronHammer");
    window.setFramerateLimit(60);
    const bool isWindowInitialized = ImGui::SFML::Init(window);
    if (!isWindowInitialized) return 0;

    SystemRegistry systemRegistry;
    ArchetypeRegistry archetypeRegistry(systemRegistry);
    EntityManager entityManager(archetypeRegistry);
    GUISystem& guiSystem = systemRegistry.RegisterSystem<GUISystem>(entityManager, archetypeRegistry);
    RenderSystem& renderSystem = systemRegistry.RegisterSystem<RenderSystem>(window);

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

    Entity e0 = entityManager.CreateEntity(CPosition{1, 1, 1}, CShape(30, 3, sf::Color::Black, sf::Color::Yellow, 2));
    Entity e5 = entityManager.CreateEntity(CPosition{1, 1, 1}, CShape(50, 3, sf::Color::Black, sf::Color::Green, 2));
    Entity e6 = entityManager.CreateEntity(CPosition{1, 1, 1}, CShape(20, 3, sf::Color::Black, sf::Color::Blue, 2));
    Entity e1 = entityManager.CreateEntity(CPosition{1, 1, 1}, CVelocity(0, 100, 12));
    Entity e2 = entityManager.CreateEntity(CPosition{1, 1, 1}, CVelocity(0, 100, 12));
    Entity e3 = entityManager.CreateEntity(CRotation(2, 2, 1));
    Entity e4 = entityManager.CreateEntity(CVelocity(0, 100, 12), CRotation(2, 2, 2));

    entityManager.GetComponentRef<CShape>(e0).shape.setPosition({50, 50});
    entityManager.GetComponentRef<CShape>(e5).shape.setPosition({100, 100});
    entityManager.GetComponentRef<CShape>(e6).shape.setPosition({200, 200});

    sf::Clock deltaClock;
    guiSystem.AppleGUITheme();
    while (window.isOpen())
    {
        while (const auto event = window.pollEvent())
        {
            ImGui::SFML::ProcessEvent(window, *event);
            if (event->is<sf::Event::Closed>()) window.close();
            // if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D))
            // {
            //     entityManager.AddToEntity(e2, CRotation{5,4,3});
            // }
        }

        ImGui::SFML::Update(window, deltaClock.restart());
        guiSystem.HandleGUISystem();
        renderSystem.HandleRenderSystem();
    }
}
