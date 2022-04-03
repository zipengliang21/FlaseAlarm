#include "components.hpp"
#include "render_system.hpp" // for gl_has_errors
#include "world_init.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "../ext/stb_image/stb_image.h"

// stlib
#include <iostream>
#include <sstream>
#include <random>

using namespace std;

Debug debugging;
float death_timer_counter_ms = 3000;

// Very, VERY simple OBJ loader from https://github.com/opengl-tutorials/ogl tutorial 7
// (modified to also read vertex color and omit uv and normals)
bool Mesh::loadFromOBJFile(std::string obj_path, std::vector<ColoredVertex> &out_vertices, std::vector<uint16_t> &out_vertex_indices, vec2 &out_size)
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

	FILE *file = fopen(obj_path.c_str(), "r");
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
	for (ColoredVertex &pos : out_vertices)
	{
		max_position = glm::max(max_position, pos.position);
		min_position = glm::min(min_position, pos.position);
	}
	if (abs(max_position.z - min_position.z) < 0.001)
		max_position.z = min_position.z + 1; // don't scale z direction when everythin is on one plane

	vec3 size3d = max_position - min_position;
	out_size = size3d;

	// Normalize mesh to range -0.5 ... 0.5
	for (ColoredVertex &pos : out_vertices)
		pos.position = ((pos.position - min_position) / size3d) - vec3(0.5f, 0.5f, 0.5f);

	return true;
}

TEXTURE_ASSET_ID Player::GetTexId(double nowTime)
{
	//printf("%f + %f --- %f\n", lastSwitchTime, switchFrame, nowTime);

	// if now is not the time to refresh, return directly.
	if (lastSwitchTime + switchFrame > nowTime)
	{
		//cout << nowTime << " curTexId: " << (int)curTexId << endl;
		return curTexId;
	}

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
	//cout << nowTime << " curTexId: " << (int)curTexId << endl;
	return curTexId;
}

void Player::SwitchDirection(Direction dir, double nowTime)
{
	if (dir == curDir)
	{
		return;
	}

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

Movie::Movie(std::vector<TEXTURE_ASSET_ID> textures, double frameInterval) :
	curTexIndex(0), textures(textures), frameInterval(frameInterval), lastSwitchTime(0)
{
}

void Movie::SetTextures(std::vector<TEXTURE_ASSET_ID> textures)
{
	assert(textures.size() > 0);
	this->textures = textures;
	curTexIndex = 0;
}

TEXTURE_ASSET_ID Movie::GetTexId(double nowTime)
{
	if (nowTime > lastSwitchTime + frameInterval) // should switch texture
	{
		// do switch 

		curTexIndex++;
		if (curTexIndex == textures.size())
			curTexIndex = 0;
		lastSwitchTime = nowTime;
	}

	return textures[curTexIndex];
}

Tool::Tool(ToolType type) :type(type), movie({})
{
	switch (type)
	{
	case ToolType::SANDGLASS:
		movie.SetTextures({ TEXTURE_ASSET_ID::SANDGLASS }); // if you want the tool's apperance to be animated, just add more texture to this
		break;
	case ToolType::REMOTE_CONTROL:
		movie.SetTextures({ TEXTURE_ASSET_ID::REMOTE_CONTROL });
		break;
	case ToolType::HAMMER:
		movie.SetTextures({ TEXTURE_ASSET_ID::HAMMER });
		break;
	case ToolType::BEE:
		movie.SetTextures({ TEXTURE_ASSET_ID::BEE });
		break;
	default:
		assert(0);
	}
}

TEXTURE_ASSET_ID Tool::GetTexId(double nowTime)
{
	return movie.GetTexId(nowTime);
}

vec2 Tool::GetUIPosition() const
{
	vec2 pos;
	switch (type)
	{
	case ToolType::SANDGLASS:
		pos = { window_width_px * TOOL1_UI_X_POS_COEF,window_height_px * 0.1 };
		break;
	case ToolType::REMOTE_CONTROL:
		pos = { window_width_px * TOOL2_UI_X_POS_COEF,window_height_px * 0.1 };
		break;
	case ToolType::HAMMER:
		pos = { window_width_px * TOOL3_UI_X_POS_COEF,window_height_px * 0.1 };
		break;
	case ToolType::BEE:
		pos = { window_width_px * TOOL4_UI_X_POS_COEF,window_height_px * 0.1 };
		break;
	default:
		assert(0);
	}
	return pos;
}

vec2 Tool::GetUISize() const
{
	return { TOOL_UI_SIZE,TOOL_UI_SIZE };
}

char Tool::GetCommandChar() const
{
	char c;
	switch (type)
	{
	case ToolType::SANDGLASS:
		c = SANDGLASS_CHAR;
		break;
	case ToolType::REMOTE_CONTROL:
		c = REMOTE_CONTROL_CHAR;
		break;
	case ToolType::HAMMER:
		c = HAMMER_CHAR;
		break;
	case ToolType::BEE:
		c = BEE_CHAR;
		break;
	default:
		assert(0);
	}
	return c;
}

bool TurnTimer::UpdateAndCheckIsTimeout(float elapsed_ms)
{
	counter_ms -= elapsed_ms;
	if (counter_ms < 0)
	{
		counter_ms = turnTime;
		return true;
	}
	return false;
}

WindParticle::WindParticle(const Wind &parentWind, const Entity &windEntity, float t0) :windEntity(windEntity)
{
	//   vx in [length/5.0, length/1.0]
//   A  in [0.1*width, 0.5*width]
//   T  in [0.1*length, 2.0*length]
	this->t0 = t0;
	pos0 = parentWind.pos;

	uniform_real_distribution<float> uniVx(parentWind.length / 5.0f, parentWind.length / 1.0f);
	v = uniVx(eng);

	uniform_real_distribution<float> uniA(parentWind.width * 0.1f, parentWind.width * 1.0f);
	A = uniA(eng);

	uniform_real_distribution<float> uniT(parentWind.length * 0.01f, parentWind.length * 1.0f);
	T = uniT(eng);

	parentLength = parentWind.length;
	dir = parentWind.dir;
}

bool WindParticle::IsAlive(float t) const
{
	return v * (t - t0) < parentLength;
}

vec2 WindParticle::GetPos(float t) const
{
	vec2 diffPos;
	float sign = -1;
	if (dir == Direction::RIGHT || dir == Direction::DOWN)
		sign = 1;
	diffPos.x = sign * v * (t - t0);
	diffPos.y = A * sin(2.0 * M_PI / T * (t - t0));

	if (dir == Direction::UP || dir == Direction::DOWN)
		swap(diffPos.x, diffPos.y);
	return pos0 + diffPos;
}

bool Wind::InRange(vec2 objPos) const
{
	vec2 xRangeDiff(-length, 0);
	if (dir == Direction::RIGHT || dir == Direction::DOWN)
		xRangeDiff = vec2(0, length);

	vec2 yRangeDiff = { -0.5f * width,+0.5f * width };

	if (dir == Direction::UP || dir == Direction::DOWN)
	{
		swap(xRangeDiff, yRangeDiff);
	}

	vec2 xRange = vec2(pos.x) + xRangeDiff;
	vec2 yRange = vec2(pos.y) + yRangeDiff;

	bool inRange = xRange[0] <= objPos.x && objPos.x <= xRange[1] && yRange[0] <= objPos.y && objPos.y <= yRange[1];
	return inRange;
}

void Wind::Influence(Motion &motion) const
{
	if (InRange(motion.position) == false)
		return;

	float sign = -1;
	if (dir == Direction::RIGHT || dir == Direction::DOWN)
		sign = 1;

	vec2 vGoal = { sign * 100.0f,0 };
	if (dir == Direction::UP || dir == Direction::DOWN)
	{
		swap(vGoal.x, vGoal.y);
	}

	motion.velocityGoal = vGoal;
	cout << "flowing" << endl;
}

Direction Wind::GetWindDirByChar(char c)
{
	Direction dir;
	switch (c)
	{
	case '<':dir = Direction::LEFT; break;
	case '>':dir = Direction::RIGHT; break;
	case 'v':dir = Direction::DOWN; break;
	case '^':dir = Direction::UP; break;
	default: assert(0);
	}
	return dir;
}

char Wind::GetWindDirChar(Direction dir)
{
	char c;
	switch (dir)
	{
	case Direction::LEFT:c = '<'; break;
	case Direction::RIGHT:c = '>'; break;
	case Direction::DOWN:c = 'v'; break;
	case Direction::UP:c = '^'; break;
	default: assert(0);
	}
	return c;
}

Bee::Bee(const Entity &targetEntity, vec2 diffPosWithTarget, vec2 bornPos) :usedTime(0), targetEntity(targetEntity),
diffPosWithTarget(diffPosWithTarget),
bornPos(bornPos),
goingTime(BEE_GOING_TIME),
stayTime(BEE_STAY_TIME),
leaveTime(BEE_LEAVE_TIME)
{

	uniform_real_distribution<float> uniA(WALL_SIZE * 0.1f, WALL_SIZE * 2.0f);
	A = uniA(eng);

	uniform_real_distribution<float> uniT(0.1f, 1.0f);
	T = uniT(eng);
}

Bee::State Bee::GetState() const
{
	// current: go for chasing target
	if (usedTime < goingTime)
	{
		return State::GOING;
	}

	// current: floating on the target
	if (usedTime < goingTime + stayTime)
	{
		return State::STAY;
	}

	// current: leave the target
	if (usedTime < goingTime + stayTime + leaveTime)
	{
		return State::LEAVE;
	}
	assert(0);
	return State();
}

void Bee::ModifyMotion(float dt, Motion &beeMotion, const Motion &targetMotion)
{
	assert(IsAlive());

	// current: go for chasing target
	if (usedTime < goingTime)
	{
		float remainTime = goingTime - usedTime; // the remaining time for the bees to chase the target
		vec2 remainJourney = (targetMotion.position+diffPosWithTarget) - beeMotion.position;
		beeMotion.position += remainJourney * (dt / remainTime);

		usedTime += dt;
		return;
	}

	// current: floating on the target
	if (usedTime < goingTime + stayTime)
	{
		float remainTime = goingTime +stayTime - usedTime; // the remaining time for the bees to stay
		beeMotion.position = (targetMotion.position + diffPosWithTarget);

		// y diff
		float yDiff = A * sin(2.0f*M_PI/T * usedTime);
		beeMotion.position.y += yDiff;

		usedTime += dt;
		return;
	}

	// current: leave the target
	if (usedTime < goingTime + stayTime + leaveTime)
	{
		float remainTime = goingTime + stayTime+leaveTime - usedTime; // the remaining time for the bees to leave
		vec2 remainJourney = bornPos - beeMotion.position;
		beeMotion.position += remainJourney * (dt / remainTime);

		usedTime += dt;
		return;
	}
}

bool Bee::IsAlive() const
{
	return usedTime <= (goingTime + stayTime + leaveTime);
}
