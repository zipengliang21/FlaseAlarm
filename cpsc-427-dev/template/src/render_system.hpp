#pragma once

#include <array>
#include <utility>

#include "common.hpp"
#include "components.hpp"
#include "tiny_ecs.hpp"

// System responsible for setting up OpenGL and for rendering all the
// visual entities in the game
class RenderSystem {
	/**
	 * The following arrays store the assets the game will use. They are loaded
	 * at initialization and are assumed to not be modified by the render loop.
	 *
	 * Whenever possible, add to these lists instead of creating dynamic state
	 * it is easier to debug and faster to execute for the computer.
	 */
	std::array<GLuint, texture_count> texture_gl_handles;
	std::array<ivec2, texture_count> texture_dimensions;

	// Make sure these paths remain in sync with the associated enumerators.
	// Associated id with .obj path
	const std::vector < std::pair<GEOMETRY_BUFFER_ID, std::string>> mesh_paths =
	{
		  std::pair<GEOMETRY_BUFFER_ID, std::string>(GEOMETRY_BUFFER_ID::CHICKEN, mesh_path("chicken.obj"))
		  // specify meshes of other assets here
	};

	// Make sure these paths remain in sync with the associated enumerators.
	const std::array<std::string, texture_count> texture_paths = {
			textures_path("student.png"),
			textures_path("guard.png"),
			textures_path("wall.png"),
			textures_path("exit.png"),
			textures_path("win.png"),
			textures_path("camera.png"),
			textures_path("light.png"),
			textures_path("player/player_01.png"),
			textures_path("player/player_02.png"),
			textures_path("player/player_03.png"),
			textures_path("player/player_04.png"),
			textures_path("player/player_05.png"),
			textures_path("player/player_06.png"),
			textures_path("player/player_07.png"),
			textures_path("player/player_08.png"),
			textures_path("player/player_09.png"),
			textures_path("player/player_10.png"),
			textures_path("player/player_11.png"),
			textures_path("player/player_12.png"),
			textures_path("player/player_13.png"),
			textures_path("player/player_14.png"),
			textures_path("player/player_15.png"),
			textures_path("player/player_16.png"),
			textures_path("guard/guard_01.png"),
			textures_path("guard/guard_02.png"),
			textures_path("guard/guard_03.png"),
			textures_path("guard/guard_04.png"),
			textures_path("guard/guard_05.png"),
			textures_path("guard/guard_06.png"),
			textures_path("guard/guard_07.png"),
			textures_path("guard/guard_08.png"),
			textures_path("guard/guard_09.png"),
			textures_path("guard/guard_10.png"),
			textures_path("guard/guard_11.png"),
			textures_path("guard/guard_12.png"),
			textures_path("guard/guard_13.png"),
			textures_path("guard/guard_14.png"),
			textures_path("guard/guard_15.png"),
			textures_path("guard/guard_16.png"),
			textures_path("guard/guard_17.png"),
			textures_path("guard/guard_18.png"),
			textures_path("guard/guard_19.png"),
			textures_path("guard/guard_20.png"),
			textures_path("guard/guard_21.png"),
			textures_path("guard/guard_22.png"),
			textures_path("guard/guard_23.png"),
			textures_path("guard/guard_24.png"),
			textures_path("guard/guard_25.png"),
			textures_path("guard/guard_26.png"),
			textures_path("guard/guard_27.png"),
			textures_path("guard/guard_28.png"),
			textures_path("guard/guard_29.png"),
			textures_path("guard/guard_30.png"),
			textures_path("guard/guard_31.png"),
			textures_path("guard/guard_32.png"),
			textures_path("guard/guard_33.png"),
			textures_path("guard/guard_34.png"),
			textures_path("guard/guard_35.png"),
			textures_path("guard/guard_36.png"),
			textures_path("paper_trap.png")
	};

	std::array<GLuint, effect_count> effects;
	// Make sure these paths remain in sync with the associated enumerators.
	const std::array<std::string, effect_count> effect_paths = {
		shader_path("coloured"),
		shader_path("egg"),
		shader_path("chicken"),
		shader_path("textured"),
		shader_path("wind") };

	std::array<GLuint, geometry_count> vertex_buffers;
	std::array<GLuint, geometry_count> index_buffers;
	std::array<Mesh, geometry_count> meshes;

public:
	// Initialize the window
	bool init(GLFWwindow* window);

	template <class T>
	void bindVBOandIBO(GEOMETRY_BUFFER_ID gid, std::vector<T> vertices, std::vector<uint16_t> indices);

	void initializeGlTextures();

	void initializeGlEffects();

	void initializeGlMeshes();
	Mesh& getMesh(GEOMETRY_BUFFER_ID id) { return meshes[(int)id]; };

	void initializeGlGeometryBuffers();
	// Initialize the screen texture used as intermediate render target
	// The draw loop first renders to this texture, then it is used for the wind
	// shader
	bool initScreenTexture();

	// Destroy resources associated to one or all entities created by the system
	~RenderSystem();

	// Draw all entities
	void draw();

	mat3 translationMatrix = { {-0.5f, 0.f, 0.f}, {0.f, 1.0f, 0.f}, {0.f, 0.f, 0.f} };

	mat3 createProjectionMatrix();

private:
	// Internal drawing functions for each entity type
	void drawTexturedMesh(Entity entity, const mat3& projection);
	void drawToScreen();

	// Window handle
	GLFWwindow* window;

	// Screen texture handles
	GLuint frame_buffer;
	GLuint off_screen_render_buffer_color;
	GLuint off_screen_render_buffer_depth;

	Entity screen_state_entity;
};

bool loadEffectFromFile(
	const std::string& vs_path, const std::string& fs_path, GLuint& out_program);
