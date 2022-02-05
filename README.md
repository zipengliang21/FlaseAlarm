**Implemented Feature:**

**Basic character movement (w/a/s/d):** world_system.cpp, on_key(), line 465-502

**Basic collision detection (player with walls/floors):**

**Sprite with key element (player, security guards/camera, door, fire alarm):**

world_system.cpp, restart_game, line 256-260, line 351 create student(player) with a given location.

world_system.cpp, restart_game, line 265-266, create a fire alarm(exit) in the upper right corner of the map. 

world_system.cpp, restart_game, line 268-349, a few loops are being used to create a wall conditionally. In this loop, if the space has not been used, then this space is left for doors. 

world_system.cpp, restart_game, line 354, create guard with a given location 

Basic level map with static walls:world_system.cpp, restart_game, line 264-349: we used square wall and for loops to create walls for rooms at all corners of the map

**Camera Control (Up/Down/Left/Right):** render_system.hpp, line 79, define translation matrix. 

render_system.hpp, line 124-126, use translation matrix to create view
		       
textured.vs.glsl, line 19-20, use translation matrix in texture to update view position.
    
World_system.cpp, onkey(), line 506-525, use up/down/left/right to control the camera.
	
**Audio Feedback:** audio file is added in data/audio, and there are 2 sound effects. fire_alarm.wav will play once the user reaches the fire alarm destination, wall_collision.wav will play once the user collides with the wall. Declaration of image file is in line 67, 68 of world_system.hpp, audio is loaded at line 113, 114 of world_system.hpp, checking for null pointer at line 116 to 122 of world_system.hpp, file data is freed in line 39-42 of world_system.hpp.

**Set simple path for security guards:** world_system.cpp, step, line 222-230: we set a walkTimer for the guard to change the direction of his motion for a fixed time period.

**Graceful Termination:** Once the user reaches the fire alarm, a text box will show up and says "You Win" to indicate termination, textbox is created at line 435 of world_system.cpp, create textbox is defined in line 127-151 of world_system.cpp. Also once the user reach destination, user won't be able to move anymore and keyboard control will be disabled, this is done by line 432-434 in world_system.cpp, line 43 in physics_system.cpp and line 460-462 of world_system.cpp. The textbox displayed come from the image in data/textures, which is loaded in line 37 of render_system.hpp

