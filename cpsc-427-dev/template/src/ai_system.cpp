// internal
#include "ai_system.hpp"

float GUARD_VELOCITY = 100.f;
int refresh_frequency = 5;

void AISystem::step(float elapsed_ms)
{
	// trigger the trap effect, guards will start to chase the player with the shortest path
	// TODO: currently, the shortest path is guard move toward the player directly that will ignore wall, which need to be changed
	if (registry.trappables.size() > 0) {
		int refresh_timer = refresh_frequency;

		// refresh 
		if (refresh_timer >= refresh_frequency) {
			refresh_timer = 0;

			// get entity
			Entity player = registry.players.entities[0];
			Entity guard = registry.guards.entities[0];

			// get their motion
			Motion& player_motion = registry.motions.get(player);
			Motion& guard_motion = registry.motions.get(guard);

			// Get the vector from guard to player.
			// and then calculate the unit vector
			vec2 positionDifference = player_motion.position - guard_motion.position;
			vec2 vector_chase = positionDifference / sqrt(dot(positionDifference, positionDifference));

			// update the velocity and angle of guard
			guard_motion.angle = atan2(vector_chase.y, vector_chase.x);
			guard_motion.velocity = vector_chase * GUARD_VELOCITY;

		}
		refresh_timer++;
	}
}