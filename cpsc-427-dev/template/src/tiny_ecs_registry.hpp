#pragma once
#include <vector>

#include "tiny_ecs.hpp"
#include "components.hpp"

class ECSRegistry
{
	// Callbacks to remove a particular or all entities in the system
	std::vector<ContainerInterface*> registry_list;

public:
	// Manually created list of all components this game has
	// TODO: A1 add a LightUp component
	ComponentContainer<DeathTimer> deathTimers;
	ComponentContainer<Motion> motions;
	ComponentContainer<Collision> collisions;
	ComponentContainer<Player> players;
	ComponentContainer<Mesh*> meshPtrs;
	ComponentContainer<RenderRequest> renderRequests;
	ComponentContainer<ScreenState> screenStates;
	ComponentContainer<Eatable> eatables;
	ComponentContainer<Deadly> deadlys;
	ComponentContainer<DebugComponent> debugComponents;
	ComponentContainer<vec3> colors;
	ComponentContainer<Wall> walls;
	ComponentContainer<TurnTimer> turnTimers;
	ComponentContainer<Stopable> stopables;
	ComponentContainer<Exit> exits;
	ComponentContainer<Win> wins;
	ComponentContainer<Stoped> stopeds;
	ComponentContainer<WinTimer> winTimers;
	ComponentContainer<Clickable> clickables;
	ComponentContainer<Camera> cameras;
	ComponentContainer<Light> lights;
	ComponentContainer<Trap> traps;
	ComponentContainer<Trappable> trappables;
	ComponentContainer<Guard> guards;
	ComponentContainer<Conversation> conversations;
	ComponentContainer<GameState> gameStates;
	ComponentContainer<Movie> movies;
	ComponentContainer<Tool> tools;
	ComponentContainer<UI> uis;
	ComponentContainer<Background> background;
	ComponentContainer<Exploded> explodeds;
	ComponentContainer<Wind> winds;
	ComponentContainer<WindParticle> windParticles;
	ComponentContainer<Bee> bees;

	// constructor that adds all containers for looping over them
	// IMPORTANT: Don't forget to add any newly added containers!
	ECSRegistry()
	{
		// TODO: A1 add a LightUp component
		registry_list.push_back(&deathTimers);
		registry_list.push_back(&motions);
		registry_list.push_back(&collisions);
		registry_list.push_back(&players);
		registry_list.push_back(&meshPtrs);
		registry_list.push_back(&renderRequests);
		registry_list.push_back(&screenStates);
		registry_list.push_back(&eatables);
		registry_list.push_back(&deadlys);
		registry_list.push_back(&debugComponents);
		registry_list.push_back(&colors);
		registry_list.push_back(&walls);
		registry_list.push_back(&turnTimers);
		registry_list.push_back(&stopables);;
		registry_list.push_back(&exits);
		registry_list.push_back(&wins);
		registry_list.push_back(&stopeds);
		registry_list.push_back(&winTimers);
		registry_list.push_back(&clickables);
		registry_list.push_back(&cameras);
		registry_list.push_back(&lights);
		registry_list.push_back(&traps);
		registry_list.push_back(&trappables);
		registry_list.push_back(&guards);
		registry_list.push_back(&conversations);
		registry_list.push_back(&gameStates);
		registry_list.push_back(&movies);
		registry_list.push_back(&tools);
		registry_list.push_back(&uis);
		registry_list.push_back(&background);
		registry_list.push_back(&explodeds);
		registry_list.push_back(&winds);
		registry_list.push_back(&windParticles);
		registry_list.push_back(&bees);
	}

	void clear_all_components() {
		for (ContainerInterface* reg : registry_list)
			reg->clear();
	}

	void list_all_components() {
		printf("Debug info on all registry entries:\n");
		for (ContainerInterface* reg : registry_list)
			if (reg->size() > 0)
				printf("%4d components of type %s\n", (int)reg->size(), typeid(*reg).name());
	}

	void list_all_components_of(Entity e) {
		printf("Debug info on components of entity %u:\n", (unsigned int)e);
		for (ContainerInterface* reg : registry_list)
			if (reg->has(e))
				printf("type %s\n", typeid(*reg).name());
	}

	void remove_all_components_of(Entity e) {
		for (ContainerInterface* reg : registry_list)
			reg->remove(e);
	}
};

extern ECSRegistry registry;