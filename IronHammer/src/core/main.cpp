#include <SFML/Graphics.hpp>

#include "main.h"
#include "EntityManager.h"

struct CPosition
{
	float x, y, z;
};

struct CVelocity
{
	float x, y, z;
};

struct CRotation
{
	float x, y, z;
};

template<typename ...Components>
struct ArchetypeChunk //Total Size = 72Bytes + |Components| * 32
{
	static constexpr size_t maxChunkSize = 64;
	size_t currentChunckSize = 0;
	std::tuple<std::vector<Components>...> components;

	std::vector<size_t> denseIDs;
	std::vector<size_t> sparse;

	ArchetypeChunk()
	{
		denseIDs.reserve(maxChunkSize);
		sparse.reserve(maxChunkSize);

		std::apply([&](auto&... componentVector)
			{

				(..., componentVector.resize(maxChunkSize));

			}, components);
	}
};

template<typename ...Components>
struct Archetype
{
	std::vector<ArchetypeChunk<Components...>> chunks;
};

int main()
{
	//ArchetypeChunk<CPosition, CRotation, CVelocity> chunk;

	sf::RenderWindow window(sf::VideoMode({ 1280, 720 }), "IronHammer");
	window.setFramerateLimit(60);

	ImGui::SFML::Init(window);

	EntityManager entityManager;
	Entity* e1 = entityManager.CreateEntity();
	Entity* e2 = entityManager.CreateEntity();
	Entity* e3 = entityManager.CreateEntity();
	Entity* e4 = entityManager.CreateEntity();

	std::cout <<  "Pre Delete:" << "\n";
	std::cout << e1->id << " / "<< e1 << "\n";
	std::cout << e2->id << " / " << e2 << "\n";
	std::cout << e3->id << " / " << e3 << "\n";
	std::cout << e4->id << " / " << e4 << "\n";

	entityManager.DeleteEntity(e1);
	entityManager.DeleteEntity(e4);

	Entity* e5 = entityManager.CreateEntity();
	Entity* e6 = entityManager.CreateEntity();

	std::cout << "Post Delete:" << "\n";

	std::cout << e1->id << " / " << e1 << "\n";
	std::cout << e2->id << " / " << e2 << "\n";
	std::cout << e3->id << " / " << e3 << "\n";
	std::cout << e4->id << " / " << e4 << "\n";
	std::cout << e5->id << " / " << e5 << "\n";
	std::cout << e6->id << " / " << e6 << "\n";

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
