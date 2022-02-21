#include "components.hpp"
#include "render_system.hpp" // for gl_has_errors

#define STB_IMAGE_IMPLEMENTATION
#include "../ext/stb_image/stb_image.h"

// stlib
#include <iostream>
#include <sstream>

Debug debugging;
float death_timer_counter_ms = 3000;

// Very, VERY simple OBJ loader from https://github.com/opengl-tutorials/ogl tutorial 7
// (modified to also read vertex color and omit uv and normals)
bool Mesh::loadFromOBJFile(std::string obj_path, std::vector<ColoredVertex>& out_vertices, std::vector<uint16_t>& out_vertex_indices, vec2& out_size)
{
	// disable warnings about fscanf and fopen on Windows
#ifdef _MSC_VER
#pragma warning(disable:4996)
#endif

	printf("Loading OBJ file %s...\n", obj_path.c_str());
	// Note, normal and UV indices are not loaded/used, but code is commented to do so
	std::vector<uint16_t> out_uv_indices, out_normal_indices;
	std::vector<glm::vec2> out_uvs;
	std::vector<glm::vec3> out_normals;

	FILE* file = fopen(obj_path.c_str(), "r");
	if (file == NULL) {
		printf("Impossible to open the file ! Are you in the right path ? See Tutorial 1 for details\n");
		getchar();
		return false;
	}

	while (1) {
		char lineHeader[128];
		// read the first word of the line
		int res = fscanf(file, "%s", lineHeader);
		if (res == EOF)
			break; // EOF = End Of File. Quit the loop.

		if (strcmp(lineHeader, "v") == 0) {
			ColoredVertex vertex;
			int matches = fscanf(file, "%f %f %f %f %f %f\n", &vertex.position.x, &vertex.position.y, &vertex.position.z,
				&vertex.color.x, &vertex.color.y, &vertex.color.z);
			if (matches == 3)
				vertex.color = { 1,1,1 };
			out_vertices.push_back(vertex);
		}
		else if (strcmp(lineHeader, "vt") == 0) {
			glm::vec2 uv;
			fscanf(file, "%f %f\n", &uv.x, &uv.y);
			uv.y = -uv.y; // Invert V coordinate since we will only use DDS texture, which are inverted. Remove if you want to use TGA or BMP loaders.
			out_uvs.push_back(uv);
		}
		else if (strcmp(lineHeader, "vn") == 0) {
			glm::vec3 normal;
			fscanf(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z);
			out_normals.push_back(normal);
		}
		else if (strcmp(lineHeader, "f") == 0) {
			std::string vertex1, vertex2, vertex3;
			unsigned int vertexIndex[3], normalIndex[3], uvIndex[3];

			int matches = fscanf(file, "%d %d %d\n", &vertexIndex[0], &vertexIndex[1], &vertexIndex[2]);
			if (matches == 1) // try again
			{
				// Note first vertex index is already consumed by the first fscanf call (match ==1) since it aborts on the first error
				matches = fscanf(file, "//%d %d//%d %d//%d\n", &normalIndex[0], &vertexIndex[1], &normalIndex[1], &vertexIndex[2], &normalIndex[2]);
				if (matches != 5) // try again
				{
					matches = fscanf(file, "%d/%d %d/%d/%d %d/%d/%d\n", &uvIndex[0], &normalIndex[0], &vertexIndex[1], &uvIndex[1], &normalIndex[1], &vertexIndex[2], &uvIndex[2], &normalIndex[2]);
					if (matches != 8)
					{
						printf("File can't be read by our simple parser :-( Try exporting with other options\n");
						fclose(file);
						return false;
					}
				}
			}

			// -1 since .obj starts counting at 1 and OpenGL starts at 0
			out_vertex_indices.push_back((uint16_t)vertexIndex[0] - 1);
			out_vertex_indices.push_back((uint16_t)vertexIndex[1] - 1);
			out_vertex_indices.push_back((uint16_t)vertexIndex[2] - 1);
			//out_uv_indices.push_back(uvIndex[0] - 1);
			//out_uv_indices.push_back(uvIndex[1] - 1);
			//out_uv_indices.push_back(uvIndex[2] - 1);
			out_normal_indices.push_back((uint16_t)normalIndex[0] - 1);
			out_normal_indices.push_back((uint16_t)normalIndex[1] - 1);
			out_normal_indices.push_back((uint16_t)normalIndex[2] - 1);
		}
		else {
			// Probably a comment, eat up the rest of the line
			char stupidBuffer[1000];
			fgets(stupidBuffer, 1000, file);
		}
	}
	fclose(file);

	// Compute bounds of the mesh
	vec3 max_position = { -99999,-99999,-99999 };
	vec3 min_position = { 99999,99999,99999 };
	for (ColoredVertex& pos : out_vertices)
	{
		max_position = glm::max(max_position, pos.position);
		min_position = glm::min(min_position, pos.position);
	}
	if(abs(max_position.z - min_position.z)<0.001)
		max_position.z = min_position.z+1; // don't scale z direction when everythin is on one plane

	vec3 size3d = max_position - min_position;
	out_size = size3d;

	// Normalize mesh to range -0.5 ... 0.5
	for (ColoredVertex& pos : out_vertices)
		pos.position = ((pos.position - min_position) / size3d) - vec3(0.5f, 0.5f, 0.5f);

	return true;
}

TEXTURE_ASSET_ID Player::GetTexId(double nowTime)
{
	//printf("%f + %f --- %f\n", lastSwitchTime, switchFrame, nowTime);

	// if now is not the time to refresh, return directly.
	if (lastSwitchTime + switchFrame > nowTime)
		return curTexId;

	// do refresh
	switch (curDir)
	{
	case Direction::UP:
		switch (curTexId)
		{
		case TEXTURE_ASSET_ID::PLAYER_UP_0: curTexId = TEXTURE_ASSET_ID::PLAYER_UP_1; break;
		case TEXTURE_ASSET_ID::PLAYER_UP_1: curTexId = TEXTURE_ASSET_ID::PLAYER_UP_2; break;
		case TEXTURE_ASSET_ID::PLAYER_UP_2: curTexId = TEXTURE_ASSET_ID::PLAYER_UP_3; break;
		case TEXTURE_ASSET_ID::PLAYER_UP_3: curTexId = TEXTURE_ASSET_ID::PLAYER_UP_0; break;
		default:curTexId = TEXTURE_ASSET_ID::PLAYER_UP_0; break;
		}
		break;
	case Direction::DOWN:
		switch (curTexId)
		{
		case TEXTURE_ASSET_ID::PLAYER_DOWN_0: curTexId = TEXTURE_ASSET_ID::PLAYER_DOWN_1; break;
		case TEXTURE_ASSET_ID::PLAYER_DOWN_1: curTexId = TEXTURE_ASSET_ID::PLAYER_DOWN_2; break;
		case TEXTURE_ASSET_ID::PLAYER_DOWN_2: curTexId = TEXTURE_ASSET_ID::PLAYER_DOWN_3; break;
		case TEXTURE_ASSET_ID::PLAYER_DOWN_3: curTexId = TEXTURE_ASSET_ID::PLAYER_DOWN_0; break;
		default:curTexId = TEXTURE_ASSET_ID::PLAYER_DOWN_0; break;
		}
		break;
	case Direction::LEFT:
		switch (curTexId)
		{
		case TEXTURE_ASSET_ID::PLAYER_LEFT_0: curTexId = TEXTURE_ASSET_ID::PLAYER_LEFT_1; break;
		case TEXTURE_ASSET_ID::PLAYER_LEFT_1: curTexId = TEXTURE_ASSET_ID::PLAYER_LEFT_2; break;
		case TEXTURE_ASSET_ID::PLAYER_LEFT_2: curTexId = TEXTURE_ASSET_ID::PLAYER_LEFT_3; break;
		case TEXTURE_ASSET_ID::PLAYER_LEFT_3: curTexId = TEXTURE_ASSET_ID::PLAYER_LEFT_0; break;
		default:curTexId = TEXTURE_ASSET_ID::PLAYER_LEFT_0; break;
		}
		break;
	case Direction::RIGHT:
		switch (curTexId)
		{
		case TEXTURE_ASSET_ID::PLAYER_RIGHT_0: curTexId = TEXTURE_ASSET_ID::PLAYER_RIGHT_1; break;
		case TEXTURE_ASSET_ID::PLAYER_RIGHT_1: curTexId = TEXTURE_ASSET_ID::PLAYER_RIGHT_2; break;
		case TEXTURE_ASSET_ID::PLAYER_RIGHT_2: curTexId = TEXTURE_ASSET_ID::PLAYER_RIGHT_3; break;
		case TEXTURE_ASSET_ID::PLAYER_RIGHT_3: curTexId = TEXTURE_ASSET_ID::PLAYER_RIGHT_0; break;
		default:curTexId = TEXTURE_ASSET_ID::PLAYER_RIGHT_0; break;
		}
		break;
	}
	lastSwitchTime = nowTime; // update frame time
	return curTexId;
}

void Player::SwitchDirection(Direction dir, double nowTime)
{
	if (dir == curDir)
		return;

	lastSwitchTime = nowTime;
	switch (dir)
	{
	case Direction::UP:
		curTexId = TEXTURE_ASSET_ID::PLAYER_UP_0;
		break;
	case Direction::DOWN:
		curTexId = TEXTURE_ASSET_ID::PLAYER_DOWN_0;
		break;
	case Direction::LEFT:
		curTexId = TEXTURE_ASSET_ID::PLAYER_LEFT_0;
		break;
	case Direction::RIGHT:
		curTexId = TEXTURE_ASSET_ID::PLAYER_RIGHT_0;
		break;
	}
	curDir = dir;
}

void Deadly::SwitchDirection(Direction dir, double nowTime)
{
	if (dir == curDir)
		return;

	lastSwitchTime = nowTime;
	switch (dir)
	{
	case Direction::UP:
		curTexId = TEXTURE_ASSET_ID::GUARD_UP_0;
		break;
	case Direction::DOWN:
		curTexId = TEXTURE_ASSET_ID::GUARD_DOWN_0;
		break;
	case Direction::LEFT:
		curTexId = TEXTURE_ASSET_ID::GUARD_LEFT_0;
		break;
	case Direction::RIGHT:
		curTexId = TEXTURE_ASSET_ID::GUARD_RIGHT_0;
		break;
	}
	curDir = dir;
}

TEXTURE_ASSET_ID Deadly::GetTexId(double nowTime)
{
	//printf("%f + %f --- %f\n", lastSwitchTime, switchFrame, nowTime);
	if (lastSwitchTime + switchFrame > nowTime)
		return curTexId;

	switch (curDir)
	{
	case Direction::UP:
		switch (curTexId)
		{
		case TEXTURE_ASSET_ID::GUARD_UP_0: curTexId = TEXTURE_ASSET_ID::GUARD_UP_1; break;
		case TEXTURE_ASSET_ID::GUARD_UP_1: curTexId = TEXTURE_ASSET_ID::GUARD_UP_2; break;
		case TEXTURE_ASSET_ID::GUARD_UP_2: curTexId = TEXTURE_ASSET_ID::GUARD_UP_3; break;
		case TEXTURE_ASSET_ID::GUARD_UP_3: curTexId = TEXTURE_ASSET_ID::GUARD_UP_4; break;
		case TEXTURE_ASSET_ID::GUARD_UP_4: curTexId = TEXTURE_ASSET_ID::GUARD_UP_5; break;
		case TEXTURE_ASSET_ID::GUARD_UP_5: curTexId = TEXTURE_ASSET_ID::GUARD_UP_6; break;
		case TEXTURE_ASSET_ID::GUARD_UP_6: curTexId = TEXTURE_ASSET_ID::GUARD_UP_7; break;
		case TEXTURE_ASSET_ID::GUARD_UP_7: curTexId = TEXTURE_ASSET_ID::GUARD_UP_8; break;
		case TEXTURE_ASSET_ID::GUARD_UP_8: curTexId = TEXTURE_ASSET_ID::GUARD_UP_0; break;
		default:curTexId = TEXTURE_ASSET_ID::GUARD_UP_0; break;
		}
		break;
	case Direction::DOWN:
		switch (curTexId)
		{
		case TEXTURE_ASSET_ID::GUARD_DOWN_0: curTexId = TEXTURE_ASSET_ID::GUARD_DOWN_1; break;
		case TEXTURE_ASSET_ID::GUARD_DOWN_1: curTexId = TEXTURE_ASSET_ID::GUARD_DOWN_2; break;
		case TEXTURE_ASSET_ID::GUARD_DOWN_2: curTexId = TEXTURE_ASSET_ID::GUARD_DOWN_3; break;
		case TEXTURE_ASSET_ID::GUARD_DOWN_3: curTexId = TEXTURE_ASSET_ID::GUARD_DOWN_4; break;
		case TEXTURE_ASSET_ID::GUARD_DOWN_4: curTexId = TEXTURE_ASSET_ID::GUARD_DOWN_5; break;
		case TEXTURE_ASSET_ID::GUARD_DOWN_5: curTexId = TEXTURE_ASSET_ID::GUARD_DOWN_6; break;
		case TEXTURE_ASSET_ID::GUARD_DOWN_6: curTexId = TEXTURE_ASSET_ID::GUARD_DOWN_7; break;
		case TEXTURE_ASSET_ID::GUARD_DOWN_7: curTexId = TEXTURE_ASSET_ID::GUARD_DOWN_8; break;
		case TEXTURE_ASSET_ID::GUARD_DOWN_8: curTexId = TEXTURE_ASSET_ID::GUARD_DOWN_0; break;
		default:curTexId = TEXTURE_ASSET_ID::GUARD_DOWN_0; break;
		}
		break;
	case Direction::LEFT:
		switch (curTexId)
		{
		case TEXTURE_ASSET_ID::GUARD_LEFT_0: curTexId = TEXTURE_ASSET_ID::GUARD_LEFT_1; break;
		case TEXTURE_ASSET_ID::GUARD_LEFT_1: curTexId = TEXTURE_ASSET_ID::GUARD_LEFT_2; break;
		case TEXTURE_ASSET_ID::GUARD_LEFT_2: curTexId = TEXTURE_ASSET_ID::GUARD_LEFT_3; break;
		case TEXTURE_ASSET_ID::GUARD_LEFT_3: curTexId = TEXTURE_ASSET_ID::GUARD_LEFT_4; break;
		case TEXTURE_ASSET_ID::GUARD_LEFT_4: curTexId = TEXTURE_ASSET_ID::GUARD_LEFT_5; break;
		case TEXTURE_ASSET_ID::GUARD_LEFT_5: curTexId = TEXTURE_ASSET_ID::GUARD_LEFT_6; break;
		case TEXTURE_ASSET_ID::GUARD_LEFT_6: curTexId = TEXTURE_ASSET_ID::GUARD_LEFT_7; break;
		case TEXTURE_ASSET_ID::GUARD_LEFT_7: curTexId = TEXTURE_ASSET_ID::GUARD_LEFT_8; break;
		case TEXTURE_ASSET_ID::GUARD_LEFT_8: curTexId = TEXTURE_ASSET_ID::GUARD_LEFT_0; break;
		default:curTexId = TEXTURE_ASSET_ID::GUARD_LEFT_0; break;
		}
		break;
	case Direction::RIGHT:
		switch (curTexId)
		{
		case TEXTURE_ASSET_ID::GUARD_RIGHT_0: curTexId = TEXTURE_ASSET_ID::GUARD_RIGHT_1; break;
		case TEXTURE_ASSET_ID::GUARD_RIGHT_1: curTexId = TEXTURE_ASSET_ID::GUARD_RIGHT_2; break;
		case TEXTURE_ASSET_ID::GUARD_RIGHT_2: curTexId = TEXTURE_ASSET_ID::GUARD_RIGHT_3; break;
		case TEXTURE_ASSET_ID::GUARD_RIGHT_3: curTexId = TEXTURE_ASSET_ID::GUARD_RIGHT_4; break;
		case TEXTURE_ASSET_ID::GUARD_RIGHT_4: curTexId = TEXTURE_ASSET_ID::GUARD_RIGHT_5; break;
		case TEXTURE_ASSET_ID::GUARD_RIGHT_5: curTexId = TEXTURE_ASSET_ID::GUARD_RIGHT_6; break;
		case TEXTURE_ASSET_ID::GUARD_RIGHT_6: curTexId = TEXTURE_ASSET_ID::GUARD_RIGHT_7; break;
		case TEXTURE_ASSET_ID::GUARD_RIGHT_7: curTexId = TEXTURE_ASSET_ID::GUARD_RIGHT_8; break;
		case TEXTURE_ASSET_ID::GUARD_RIGHT_8: curTexId = TEXTURE_ASSET_ID::GUARD_RIGHT_0; break;
		default:curTexId = TEXTURE_ASSET_ID::GUARD_RIGHT_0; break;
		}
		break;
	}
	lastSwitchTime = nowTime;
	return curTexId;
}
