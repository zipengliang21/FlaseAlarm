// internal
#include "physics_system.hpp"
#include "world_init.hpp"

using namespace std;

// Returns the local bounding coordinates scaled by the current size of the entity
vec2 get_bounding_box(const Motion& motion)
{
	// abs is to avoid negative scale due to the facing direction.
	return { abs(motion.scale.x), abs(motion.scale.y) };
}

// This is a SUPER APPROXIMATE check that puts a circle around the bounding boxes and sees
// if the center point of either object is inside the other's bounding-box-circle. You can
// surely implement a more accurate detection
bool collides(const Motion& motion1, const Motion& motion2)
{
	vec2 dp = motion1.position - motion2.position;
	float dist_squared = dot(dp,dp);
	const vec2 other_bonding_box = get_bounding_box(motion1) / 2.f;
	const float other_r_squared = dot(other_bonding_box, other_bonding_box);
	const vec2 my_bonding_box = get_bounding_box(motion2) / 2.f;
	const float my_r_squared = dot(my_bonding_box, my_bonding_box);
	const float r_squared = glm::max(other_r_squared, my_r_squared);
	if (dist_squared < r_squared)
		return true;
	return false;
}

// TODO: Define new collision algorithm for walls


void PhysicsSystem::step(float elapsed_ms)
{
	// Move bug based on how much time has passed, this is to (partially) avoid
	// having entities move at different speed based on the machine.
	auto& motion_registry = registry.motions;
	for(uint i = 0; i< motion_registry.size(); i++)
	{
		// !!! TODO A1: update motion.position based on step_seconds and motion.velocity
		
		Motion& motion = motion_registry.components[i];
		Entity entity = motion_registry.entities[i];

		if (registry.lights.has(entity)) 
		{
			// light
			float step_seconds = elapsed_ms / 1000.f;
			motion.angle += motion.velocity.x * step_seconds;
		} 
		else if (!registry.stopeds.has(entity) && !registry.wins.has(entity))  // not stopeds and wins
		{
			float step_seconds = elapsed_ms / 1000.f;
			motion.position.x += motion.velocity.x * step_seconds;
			motion.position.y += motion.velocity.y * step_seconds;
		}

	}
	
	// calc all the explodeds 's life and erase dead instance
	for (auto &e: registry.explodeds.entities)
	{
		Exploded &inst = registry.explodeds.get(e);
		inst.life -= elapsed_ms;
		if (inst.life <= 0)
		{
			registry.remove_all_components_of(e);
			continue;
		}

		// rotate it
		auto &motion = registry.motions.get(e);
		motion.angle += 0.1 * elapsed_ms;

		float lifeCoef = inst.life / inst.initLife; // [0,1], 0 for dead; 1 for born
		motion.scale = inst.initSize * lifeCoef;
	}

	// Check for collisions between all moving entities
    ComponentContainer<Motion> &motion_container = registry.motions;

	for(uint i = 0; i<motion_container.components.size(); i++)
	{
		Motion motion_i = motion_container.components[i];
		Motion motion_ic = motion_i;
		Entity entity_i = motion_container.entities[i];
		
		// note starting j at i+1 to compare all (i,j) pairs only once (and to not compare with itself)
		for(uint j = i+1; j<motion_container.components.size(); j++)
		{
			Motion motion_j = motion_container.components[j];
			Motion motion_jc = motion_j;
			Entity entity_j = motion_container.entities[j];
			for (Entity entity : registry.lights.entities) {
				if (entity == entity_i) {
					motion_i.scale.x += 90.f;
					motion_i.scale.y -= 30.f;
					break;
				} else if (entity == entity_j) {
					motion_j.scale.x += 90.f;
					motion_j.scale.y -= 30.f;
					break;
				}
				motion_j = motion_jc;
			}
			if (collides(motion_i, motion_j))
			{
				// Create a collisions event
				// We are abusing the ECS system a bit in that we potentially insert muliple collisions for the same entity
				registry.collisions.emplace_with_duplicates(entity_i, entity_j);
				registry.collisions.emplace_with_duplicates(entity_j, entity_i);
			}
			motion_i = motion_ic;
		}
		// TODO: iterate through all entities with Wall component, check collision with special wall collision algorithm
	}

	//check if player collide with wall
	if (registry.players.entities.size() > 0) {
		int palyer_collide = 0;
		Entity player = registry.players.entities[0];
		Motion& motion_player = registry.motions.get(player);
		for (int i = 0; i < registry.walls.entities.size(); i++) {
			Entity wall_i = registry.walls.entities[i];
			Motion& motion_i = registry.motions.get(wall_i);
			if (collides(motion_i, motion_player)) {
				palyer_collide = 1;
			}
		}
		if (palyer_collide == 0 && registry.stopeds.has(player)) {
			registry.stopeds.remove(player);
		}
	}


	// debugging of bounding boxes
	if (debugging.in_debug_mode)
	{
		uint size_before_adding_new = (uint)motion_container.components.size();
		for (uint i = 0; i < size_before_adding_new; i++)
		{
			Motion& motion_i = motion_container.components[i];
			Entity entity_i = motion_container.entities[i];

			// don't draw debugging visuals around debug lines
			if (registry.debugComponents.has(entity_i))
				continue;

			// visualize the radius with two axis-aligned lines
			const vec2 bonding_box = get_bounding_box(motion_i);
			float radius = sqrt(dot(bonding_box/2.f, bonding_box/2.f));
			vec2 line_scale1 = { motion_i.scale.x / 10, 2*radius };
			vec2 line_scale2 = { 2*radius, motion_i.scale.x / 10};
			vec2 position = motion_i.position;
			Entity line1 = createLine(motion_i.position, line_scale1);
			Entity line2 = createLine(motion_i.position, line_scale2);

			// !!! TODO A2: implement debug bounding boxes instead of crosses
		}
	}
}