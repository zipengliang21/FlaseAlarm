#pragma once
#include "GameLevel.h"

#include "tiny_ecs.hpp"
#include "ai_system.hpp"

#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_mixer.h>

#include <memory>

class LevelPlay :
	public GameLevel
{
public:
	Entity player;

	LevelPlay(RenderSystem *renderer, LevelManager *manager, GLFWwindow *window);

	virtual ~LevelPlay();

	// must call before clearing ECS 
	virtual void Restart() override;

	// Steps the game ahead by ms milliseconds
	virtual void step(float elapsed_ms) override;

	// Check for collisions
	virtual void handle_collisions() override;

	virtual void OnKey(int key, int, int action, int mod) override;

	virtual void OnMouseMove(double x, double y) override;

	virtual void OnMouseButton(int button, int action, int mods) override;
private:
	AISystem ai;

	float current_speed;
	float next_bug_spawn;
	int point;
	bool displayed;

	// Entity player;
	Entity guard;
	Entity exit;
	Entity digit;
	std::set<Entity> hoverHammer; // stores the hovering hammer 
	std::map<std::pair<int, int>, Entity> walls; // key={row,col}, value=Entity of wall

	// music references
	Mix_Chunk *chicken_dead_sound;
	Mix_Chunk *chicken_eat_sound;
	Mix_Chunk *death_sound;
	Mix_Chunk *fire_alarm_sound;
	Mix_Chunk *trap_sound;
	Mix_Chunk *explode_sound;
	Mix_Chunk *startLevel_sound;

	void ProcessKeyPress();

	// these series function should be called in handle_collisions(), 
	// while the one collision object is player
	bool if_collisions_player_with_deadly(Entity other);

	// now, only tools can be ate.
	bool if_collisions_player_with_eatable(Entity other);

	bool if_collisions_player_with_stopable(Entity other);

	bool if_collisions_player_with_wins(Entity other);

	bool if_collisions_player_with_traps(Entity other);

	bool if_collisions_player_with_conversations(Entity other);

	using TimeCallBack = std::function<void()>;

	// the count down event, created by a time and an entity
	// in the step(), every countdown event is be checked, if it's timeout, call its callback function
	struct CountdownEvent
	{
		float runTime;
		TimeCallBack cb;
		CountdownEvent(float runTime,  TimeCallBack cb) :
			runTime(runTime),cb(cb){}
	};

	// stored all the countdown events
	std::list<CountdownEvent> countdownEvents;

	void if_clicked_sandglass_button(Entity entity);

	void if_clicked_remote_control_button(Entity entity);

	void if_clicked_bee_button(Entity entity);

	// input a mouse cursor position, returns true, and make the row and col index of the clicked grid
	bool GetClickedRowCol(vec2 cursor, int &row, int &col);

	// add wind particles and remove dead wind particles
	void UpdateWindParticle();

	// 
	void UpdateBee(float dt);
};

// Interpolate
float approach(float goal_v, float cur_v, float dt);