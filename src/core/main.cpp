#include "main.h"
#include <SFML/Graphics.hpp>
#include "EntityManager.hpp"

int main()
{
	//ArchetypeChunk<CPosition, CRotation, CVelocity> chunk;
	sf::RenderWindow window(sf::VideoMode({ 1280, 720 }), "IronHammer");
	window.setFramerateLimit(60);

	ImGui::SFML::Init(window);

	EntityManager entityManager;
	ArchetypeRegistry ar;

	//auto test = ar.FindOrCreateArchetype<CPosition, CRotation>();
	//std::cout << test;
	/*Entity e1 = entityManager.CreateEntity();
	Entity e2 = entityManager.CreateEntity();
	Entity e3 = entityManager.CreateEntity();
	Entity e4 = entityManager.CreateEntity();


	std::cout <<  "Pre Delete:" << "\n";
	std::cout << e1.id << " / " << &e1 << " / " << e1.generation<<  "\n";
	std::cout << e2.id << " / " << &e2 << " / " << e2.generation << "\n";
	std::cout << e3.id << " / " << &e3 << " / " << e3.generation << "\n";
	std::cout << e4.id << " / " << &e4 << " / " << e4.generation << "\n";

	entityManager.DeleteEntity(e1);
	entityManager.DeleteEntity(e4);

	Entity e5 = entityManager.CreateEntity();
	Entity e6 = entityManager.CreateEntity();

	std::cout << "Post Delete:" << "\n";

	std::cout << e1.id << " / " << &e1 << " / " << e1.generation << "\n";
	std::cout << e2.id << " / " << &e2 << " / " << e2.generation << "\n";
	std::cout << e3.id << " / " << &e3 << " / " << e3.generation << "\n";
	std::cout << e4.id << " / " << &e4 << " / " << e4.generation << "\n";
	std::cout << e5.id << " / " << &e5 << " / " << e5.generation << "\n";
	std::cout << e6.id << " / " << &e6 << " / " << e6.generation << "\n"; */

		//Archetype<64, CPosition, CRotation, CVelocity> arc;

		/*std::cout << GetComponentID<CVelocity>()<<"\n";
		std::cout << GetComponentID<CPosition>()<<"\n";
		std::cout << GetComponentID<CRotation>()<<"\n";
		std::cout << GetComponentID<CRotation>()<<"\n";
		std::cout << GetComponentID<CVelocity>() << "\n";
		std::cout << GetComponentID<CPosition>() << "\n";*/

	//std::cout << std::get<0>(arc.CreateArchetypeChunk().ptrToComponentArrayTuple);



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
