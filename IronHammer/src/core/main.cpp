#include "main.h"
#include <SFML/Graphics.hpp>
#include <imgui.h>
#include <imgui-SFML.h>

int main()
{
    sf::RenderWindow window(sf::VideoMode({ 1280, 720 }), "IronHammer");
    window.setFramerateLimit(60);

    ImGui::SFML::Init(window);

    sf::Clock deltaClock;
    while (window.isOpen())
    {
        while (const auto event = window.pollEvent())
        {
            ImGui::SFML::ProcessEvent(window, *event);
            if (event->is<sf::Event::Closed>())
                window.close();
        }

        ImGui::SFML::Update(window, deltaClock.restart());

        ImGui::Begin("Debug");
        ImGui::Text("ImGui + SFML 3 works");
        ImGui::End();

        window.clear();
        ImGui::SFML::Render(window);
        window.display();
    }

    ImGui::SFML::Shutdown();
}

