**Implemented Feature:**

**Basic character movement (w/a/s/d):** world_system.cpp, on_key(), line 465-502

**Basic collision detection (player with walls/floors):** world_system.cpp, handle_collisions method, line 407-428, when the collision is detected between player and the wall, it will set the player velocity to 0, and set the position backward by 30 units with respect to the velocity direction when this collision happens.

**Sprite with key element (player, security guards/camera, door, fire alarm):**

player: world_system.cpp, restart_game, line 256-260, line 351 create student(player) with a given location.

fire alarm: world_system.cpp, restart_game, line 265-266, create a fire alarm in the upper right corner of the map. 

door/wall: world_system.cpp, restart_game, line 268-349, a few loops are being used to create a wall conditionally. In this loop, if the space has not been used, then this space is left for doors. 

security guard: world_system.cpp, restart_game, line 354, create guard with a given location 

Basic level map with static walls:world_system.cpp, restart_game, line 264-349: we used square wall and for loops to create walls for rooms at all corners of the map

**Camera Control (Up/Down/Left/Right):** render_system.hpp, line 79, define translation matrix. 

render_system.hpp, line 124-126, use translation matrix to create view
		       
textured.vs.glsl, line 19-20, use translation matrix in texture to update view position.
    
World_system.cpp, onkey(), line 506-525, use up/down/left/right to control the camera.
	
**Audio Feedback:** audio file is added in data/audio, and there are 2 sound effects. fire_alarm.wav will play once the user reaches the fire alarm destination, wall_collision.wav will play once the user collides with the wall. Declaration of image file is in line 67, 68 of world_system.hpp, audio is loaded at line 113, 114 of world_system.hpp, checking for null pointer at line 116 to 122 of world_system.hpp, file data is freed in line 39-42 of world_system.hpp.

**Set simple path for security guards:** world_system.cpp, step, line 222-230: we set a walkTimer for the guard to change the direction of his motion for a fixed time period.

**Graceful Termination:** Once the user reaches the fire alarm, a text box will show up and says "You Win" to indicate termination, textbox is created at line 435 of world_system.cpp, create textbox is defined in line 127-151 of world_system.cpp. Also once the user reach destination, user won't be able to move anymore and keyboard control will be disabled, this is done by line 432-434 in world_system.cpp, line 43 in physics_system.cpp and line 460-462 of world_system.cpp. The textbox displayed come from the image in data/textures, which is loaded in line 37 of render_system.hpp

**Milestone 2**

**sprite sheet animation:**
used sprite sheet to implement player and guard animation. component.cpp line 123-298: used time and direction to update sprite's current texture. component.hpp line 9-64: define character direction, and switch direction function. component.hpp line 251-302: define character's texture. Render_system.hpp line 37-89, define texture path for sprites. world_system.hpp line 263-281,639,651,663,675: switch sprite walking direction. 

**Playability:**
data/levels: Create a txt file for each level. W: wall, S: student, G: guard, T: trap, C:camera, L:light, E: Exit.
world_system.cpp, showLevelContent, line 411-458, based on the current level, it will read in the corresponding txt file line by line and push the characters into a vector. It will then render each entity based on the position in the txt file.

**Simple_path:**
ai_system.cpp, calcChaseVectorV1, line 66-129, calculate the chase vector by BFS. Until the guard reaches the player position, the guard chases the player around by following the path with shortest manhattan distance, and avoid walls if there is a wall in his neighboring position. 
ai_system.cpp, PosIsValid, line 15-18, return validity of an index position
ai_system.cpp, Distance, line 21-24, calculate manhattan distance between points a and b
ai_system.cpp, hasWallAtNeighbor, line 50-63, return true if there is a wall at the neighboring range of pos 

**Movement Interpolation:**
world_system.cpp, approach, line 889-901, for given current velocity of the entity and the goal velocity, interpolate the in-between velocities bases on time dt. Return goal velocity if the entity reaches it.
world_system.cpp, on_key, line 632-680, set the goal velocity of the player instead of directly set its velocity.
world_system.cpp, step, line 165-169, update the current velocity of the player and the guard.

**Game logic response to user input:**
Added image files in data folder for buttons of each level, different image represent a locked level and unlocked level. In http://cpsc-427-dev/template/data/textures/
Textbox have a new component called Clickable, which has a it's location and the correspounding button action. In cpsc-427-dev/template/src/components.hpp line 190-204, also in cpsc-427-dev/template/src/tiny_ecs_registry.hpp 30-70, In cpsc-427-dev/template/src/world_init.cpp line 190-219, world_system.cpp line 355-361
Create new class GameState and GameLevel, GameState records a state, it's either "LEVEL_SELECTION" represent we should display the menu, "LEVEL_SELECTED" represent we shuold display the selected level, "TUTORIAL_PAGE" represent we should display the tutorial page. Each GameState stores the current state and stores an reference to the GameLevel. Game_state.hpp and game_state.cpp.
New listener redirect for mouse clicks, when user clicks a the mouse, we check if it's clicking on a specific button. In world_system.cpp line 500-573, handle_collision function
And the handler for the event will check the button's action and change the program state. In world_system.cpp, line 758-888.

**Basic user tutorial/help:**
- Shiyu:
Create button for tutorial on the menu. In http://cpsc-427-dev/template/data/textures/
Create button handler for tutorail, create key handler to get back to main menu from tutorial page, world_system.cpp line 355-361, line 461-463, line 591-600
- Jingyang:
Drawed tutorial image

**Reloadability:**
New file added in data folder, /data/game_text_file/unlockedLevel.txt, which stores an integer represent the current number of level unlocked by the user
Reload and Save of game state is called i game_state.cpp line1 to the end of file

**Game Logic:**
New NPC character that will have conversation with the user, In cpsc-427-dev/template/src/world_init.cpp line 276 - 306, createNPC for character 'N' on the map in world_system.cpp line 441
New conversation component in components.hpp line 208-216
Conversation component has its own state, as a decision tree in conversation_state.hpp, the entire file.
New image for NPC and conversation added in template/data/textures/
User talk to the NPC on collision: handleder in world_system.cpp line 535-548
User interact with NPC with key event in world_system.cpp on_key line 586-603

**Asset:**
- Trap: 
World_init.cpp line 221-247: Implement createTrap function to create a new trap with the given position
World_system.cpp line 428, based on the design in level.txt file to create traps
World_system.cpp line 546-547, handle the collision between the player and the trap. It will show the sound, remove that trap, emplace the player into trappable component so that it will result in the guard chases the player, which trigger the current step function code in AISystem.cpp

- Security Camera: additional enemy. Player will die if player get scan by camera light.
World_init.cpp line 133-190: Initialize camera and light entities
World_system.cpp line 445-450: Add camera and light to the background
Physics_system.cpp line 43-45: Change the direction of the light once after a while
