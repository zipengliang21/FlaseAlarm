// internal
#include "render_system.hpp"
#include <SDL.h>
#include <glm/gtx/matrix_transform_2d.hpp>

#include "world_init.hpp"
#include "tiny_ecs_registry.hpp"

void RenderSystem::drawTexturedMesh(Entity entity,
	const mat3 &projection)
{
	Motion &motion = registry.motions.get(entity);
	// Transformation code, see Rendering and Transformation in the template
	// specification for more info Incrementally updates transformation matrix,
	// thus ORDER IS IMPORTANT
	Transform transform;
	transform.translate(motion.position);
	transform.scale(motion.scale);
	// !!! TODO A1: add rotation to the chain of transformations, mind the order
	// of transformations
	transform.rotate(motion.angle);

	assert(registry.renderRequests.has(entity));
	const RenderRequest &render_request = registry.renderRequests.get(entity);

	const GLuint used_effect_enum = (GLuint)render_request.used_effect;
	assert(used_effect_enum != (GLuint)EFFECT_ASSET_ID::EFFECT_COUNT);
	const GLuint program = (GLuint)effects[used_effect_enum];

	// Setting shaders
	glUseProgram(program);
	gl_has_errors();

	assert(render_request.used_geometry != GEOMETRY_BUFFER_ID::GEOMETRY_COUNT);
	const GLuint vbo = vertex_buffers[(GLuint)render_request.used_geometry];
	const GLuint ibo = index_buffers[(GLuint)render_request.used_geometry];

	// Setting vertex and index buffers
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	gl_has_errors();

	// Input data location as in the vertex buffer
	if (render_request.used_effect == EFFECT_ASSET_ID::TEXTURED ||
		render_request.used_effect == EFFECT_ASSET_ID::UI)
	{
		GLint in_position_loc = glGetAttribLocation(program, "in_position");
		GLint in_texcoord_loc = glGetAttribLocation(program, "in_texcoord");
		gl_has_errors();
		assert(in_texcoord_loc >= 0);

		glEnableVertexAttribArray(in_position_loc);
		glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE,
			sizeof(TexturedVertex), (void *)0);
		gl_has_errors();

		glEnableVertexAttribArray(in_texcoord_loc);
		glVertexAttribPointer(
			in_texcoord_loc, 2, GL_FLOAT, GL_FALSE, sizeof(TexturedVertex),
			(void *)sizeof(
				vec3)); // note the stride to skip the preceeding vertex position

		// Enabling and binding texture to slot 0
		glActiveTexture(GL_TEXTURE0);
		gl_has_errors();

		assert(registry.renderRequests.has(entity));
		GLuint texture_id =
			texture_gl_handles[(GLuint)registry.renderRequests.get(entity).used_texture];

		assert(texture_id != (int)TEXTURE_ASSET_ID::BUG);
		glBindTexture(GL_TEXTURE_2D, texture_id);
		gl_has_errors();

		if (render_request.used_effect == EFFECT_ASSET_ID::TEXTURED)
		{
			GLint translation_loc = glGetUniformLocation(program, "view");
			assert(translation_loc >= 0);
			glUniformMatrix3fv(translation_loc, 1, GL_FALSE, (float *)&viewMatrix);
			gl_has_errors();

				GLint loc;
				loc = glGetUniformLocation(program, "useMask");
				assert(loc >= 0);
				glUniform1i(loc, useMask);
				gl_has_errors();

			// use mask
			if (useMask)
			{
				// player pos
				loc = glGetUniformLocation(program, "playerPos");
				assert(loc >= 0);
				glUniform2f(loc, playerPos.x, playerPos.y);
				gl_has_errors();


				// rBright and rDark
				loc = glGetUniformLocation(program, "rBright");
				assert(loc >= 0);
				glUniform1f(loc, window_height_px/2.0f * 0.9f);
				gl_has_errors();
				loc = glGetUniformLocation(program, "rDark");
				assert(loc >= 0);
				glUniform1f(loc, window_height_px / 2.0f * 1.2f);
				gl_has_errors();
			}
		}

	}
	else if (render_request.used_effect == EFFECT_ASSET_ID::EGG)
	{
		GLint in_position_loc = glGetAttribLocation(program, "in_position");
		GLint in_color_loc = glGetAttribLocation(program, "in_color");
		gl_has_errors();

		glEnableVertexAttribArray(in_position_loc);
		glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE,
			sizeof(ColoredVertex), (void *)0);
		gl_has_errors();

		glEnableVertexAttribArray(in_color_loc);
		glVertexAttribPointer(in_color_loc, 3, GL_FLOAT, GL_FALSE,
			sizeof(ColoredVertex), (void *)sizeof(vec3));
		gl_has_errors();

		//GLint currProgram;
		//glGetIntegerv(GL_CURRENT_PROGRAM, &currProgram);
		//// Setting uniform values to the currently bound program 
		//GLuint transform_loc = glGetUniformLocation(currProgram, "transform");
		//glUniformMatrix3fv(transform_loc, 1, GL_FALSE, (float*)&transform.mat);
		//GLuint projection_loc = glGetUniformLocation(currProgram, "projection");
		//glUniformMatrix3fv(projection_loc, 1, GL_FALSE, (float*)&projection);
		//gl_has_errors();
	}
	else
	{
		assert(false && "Type of render request not supported");
	}

	// Getting uniform locations for glUniform* calls
	GLint color_uloc = glGetUniformLocation(program, "fcolor");
	const vec3 color = registry.colors.has(entity) ? registry.colors.get(entity) : vec3(1);
	glUniform3fv(color_uloc, 1, (float *)&color);
	gl_has_errors();

	// Get number of indices from index buffer, which has elements uint16_t
	GLint size = 0;
	glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);
	gl_has_errors();

	GLsizei num_indices = size / sizeof(uint16_t);
	// GLsizei num_triangles = num_indices / 3;

	GLint currProgram;
	glGetIntegerv(GL_CURRENT_PROGRAM, &currProgram);
	assert(currProgram >= 0);

	// Setting uniform values to the currently bound program
	GLint transform_loc = glGetUniformLocation(currProgram, "transform");
	assert(transform_loc >= 0);
	glUniformMatrix3fv(transform_loc, 1, GL_FALSE, (float *)&transform.mat);

	GLint projection_loc = glGetUniformLocation(currProgram, "projection");
	assert(projection_loc >= 0);
	glUniformMatrix3fv(projection_loc, 1, GL_FALSE, (float *)&projection);

	gl_has_errors();
	// Drawing of num_indices/3 triangles specified in the index buffer
	glDrawElements(GL_TRIANGLES, num_indices, GL_UNSIGNED_SHORT, nullptr);
	gl_has_errors();
}

// draw the intermediate texture to the screen, with some distortion to simulate
// wind
void RenderSystem::drawToScreen()
{
	// Setting shaders
	// get the wind texture, sprite mesh, and program
	glUseProgram(effects[(GLuint)EFFECT_ASSET_ID::WIND]);
	gl_has_errors();
	// Clearing backbuffer
	int w, h;
	glfwGetFramebufferSize(window, &w, &h); // Note, this will be 2x the resolution given to glfwCreateWindow on retina displays
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, w, h);
	glDepthRange(0, 10);
	glClearColor(1.f, 0, 0, 1.0);
	glClearDepth(1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	gl_has_errors();
	// Enabling alpha channel for textures
	glDisable(GL_BLEND);
	// glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_DEPTH_TEST);

	// Draw the screen texture on the quad geometry
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffers[(GLuint)GEOMETRY_BUFFER_ID::SCREEN_TRIANGLE]);
	glBindBuffer(
		GL_ELEMENT_ARRAY_BUFFER,
		index_buffers[(GLuint)GEOMETRY_BUFFER_ID::SCREEN_TRIANGLE]); // Note, GL_ELEMENT_ARRAY_BUFFER associates
																	 // indices to the bound GL_ARRAY_BUFFER
	gl_has_errors();
	const GLuint wind_program = effects[(GLuint)EFFECT_ASSET_ID::WIND];
	// Set clock
	GLuint time_uloc = glGetUniformLocation(wind_program, "time");
	GLuint dead_timer_uloc = glGetUniformLocation(wind_program, "darken_screen_factor");
	glUniform1f(time_uloc, (float)(glfwGetTime() * 10.0f));
	ScreenState &screen = registry.screenStates.get(screen_state_entity);
	glUniform1f(dead_timer_uloc, screen.darken_screen_factor);
	gl_has_errors();
	// Set the vertex position and vertex texture coordinates (both stored in the
	// same VBO)
	GLint in_position_loc = glGetAttribLocation(wind_program, "in_position");
	glEnableVertexAttribArray(in_position_loc);
	glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), (void *)0);
	gl_has_errors();

	// Bind our texture in Texture Unit 0
	glActiveTexture(GL_TEXTURE0);

	glBindTexture(GL_TEXTURE_2D, off_screen_render_buffer_color);
	gl_has_errors();
	// Draw
	glDrawElements(
		GL_TRIANGLES, 3, GL_UNSIGNED_SHORT,
		nullptr); // one triangle = 3 vertices; nullptr indicates that there is
				  // no offset from the bound index buffer
	gl_has_errors();
}

// Render our game world
// http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-14-render-to-texture/
void RenderSystem::draw()
{
	// Getting size of window
	int w, h;
	glfwGetFramebufferSize(window, &w, &h); // Note, this will be 2x the resolution given to glfwCreateWindow on retina displays

	// First render to the custom framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);
	gl_has_errors();
	// Clearing backbuffer
	glViewport(0, 0, w, h);
	glDepthRange(0.00001, 10);
	//glClearColor(0.663, 0.663, 0.663, 1.0);
	glClearColor(0, 0, 0, 1.0); // set default clear color to be black
	glClearDepth(10.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_DEPTH_TEST); // native OpenGL does not work with a depth buffer
							  // and alpha blending, one would have to sort
							  // sprites back to front
	gl_has_errors();
	mat3 projection_2D = createProjectionMatrix();

	std::vector<Entity> entitiesDrawFinal;

	// Draw all textured meshes that have a position and size component
	for (Entity &entity : registry.renderRequests.entities)
	{
		if (!registry.motions.has(entity)) // not motions
			continue;

		// mark the elements that should be drawn on the top layer
		if (registry.uis.has(entity))
		{
			entitiesDrawFinal.push_back(entity);
			continue;
		}

		// Note, its not very efficient to access elements indirectly via the entity
		// albeit iterating through all Sprites in sequence. A good point to optimize
		drawTexturedMesh(entity, projection_2D);
	}

	// draw the elments on the top layer
	for (Entity &entity : entitiesDrawFinal)
	{
		drawTexturedMesh(entity, projection_2D);
	}

	/* --------------- draw minimap start --------------- */
	do
	{
		if (registry.gameStates.size() == 0) // is in any level
			break;

		auto &gameState = registry.gameStates.components[0];
		if (gameState.AtValidLevel() == false)
			break;

		const vec2 mapSize = gameState.GetMapPixelSize();
		const float minimapWidth = window_width_px * MINIMAP_WIDTH_COEF;
		const float scaleCoef = minimapWidth / mapSize.x;
		const vec2 minimapPos = vec2(MINIMAP_POS_X, MINIMAP_POS_Y);

		// set the view
		mat3 originViewMatrix = viewMatrix;
		bool originUseMask = useMask; // minimap not use the mask
		useMask = false;
		viewMatrix = mat3(1.0f);
		viewMatrix = glm::translate(viewMatrix, minimapPos);
		viewMatrix = glm::scale(viewMatrix, vec2(scaleCoef));

		// draw a temperary background and remove it
		{
			vec2 bgCenter = mapSize / 2.0f - vec2(WALL_SIZE);
			vec2 bgSize = mapSize - vec2(WALL_SIZE);
			auto bgEntity = createBackground(this, bgCenter, bgSize, TEXTURE_ASSET_ID::FLOOR_BG);
			drawTexturedMesh(bgEntity, projection_2D);
			registry.remove_all_components_of(bgEntity);
		}

		// Draw all textured meshes that have a position and size component
		for (Entity &entity : registry.renderRequests.entities)
		{
			if (!registry.motions.has(entity)) // not motions
				continue;

			auto &inst = registry.renderRequests.get(entity);
			if (inst.showOnMinimap == false)
			{
				continue;
			}

			// Note, its not very efficient to access elements indirectly via the entity
			// albeit iterating through all Sprites in sequence. A good point to optimize
			drawTexturedMesh(entity, projection_2D);
		}

		// restore
		useMask = originUseMask;
		viewMatrix = originViewMatrix;

	} while (0);
	/* --------------- draw minimap end --------------- */

	// Truely render to the screen
	drawToScreen();

	// flicker-free display with a double buffer
	glfwSwapBuffers(window);
	gl_has_errors();
}

mat3 RenderSystem::createProjectionMatrix()
{
	// Fake projection matrix, scales with respect to window coordinates
	float left = 0.f;
	float top = 0.f;

	gl_has_errors();
	float right = (float)window_width_px;
	float bottom = (float)window_height_px;

	float sx = 2.f / (right - left);
	float sy = 2.f / (top - bottom);
	float tx = -(right + left) / (right - left);
	float ty = -(top + bottom) / (top - bottom);
	return { {sx, 0.f, 0.f}, {0.f, sy, 0.f}, {tx, ty, 1.f} };
}