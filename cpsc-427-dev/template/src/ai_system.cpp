// internal
#include "ai_system.hpp"

#include "world_init.hpp"

#include <iostream>
#include <queue>

const float GUARD_VELOCITY = 100.0f;
const float CALC_INTERVAL = 0.5f; // Calculate the shortest-path every 0.5 seconds

using namespace std;

// return validity of an index position
inline bool PosIsValid(ivec2 pos, int rows, int cols)
{
	return 0 <= pos.y && pos.y < rows && 0 <= pos.x && pos.x < cols;
}

// manhattan distance
inline int Distance(ivec2 a, ivec2 b)
{
	return abs(a.x - b.x) + abs(a.y - b.y);
}

// Node for calculating shortest-path
struct Node
{
	ivec2 pos; // current position
	int distToTarget; // the distance to target

	bool isStart; // is the second Node
	ivec2 firstDir; // start direction
	Node() {}
	Node(ivec2 pos, int distToTarget, bool isStart, ivec2 firstDir) :pos(pos), distToTarget(distToTarget), isStart(isStart), firstDir(firstDir) {}
	bool operator<(const Node &other) const
	{
		return distToTarget < other.distToTarget;
	}
	bool operator>(const Node &other) const
	{
		return distToTarget > other.distToTarget;
	}
};

// return true if there is a wall at the neighbor of pos 
// 
//   e.g. [ ][W][ ][P]     return true if range==2; return false if range==1
// 
bool hasWallAtNeighbor(const vector<vector<char>> &levelMap, ivec2 pos, int range)
{
	int rows = levelMap.size();
	int cols = levelMap[0].size();
	for (int i = std::max(0, pos.y - range); i < std::min(rows, pos.y + range); ++i)
	{
		for (int j = std::max(0, pos.x - range); j < std::min(cols, pos.x + range); ++j)
		{
			if (levelMap[i][j] == 'W')
				return true;
		}
	}
	return false;
}

// calculate the chase vector by BFS
vec2 calcChaseVectorV1(const vector<vector<char>> &levelMap, ivec2 guardPos, ivec2 targetPos)
{
	vector<vector<char>> temp = levelMap;
	int rows = levelMap.size();
	int cols = levelMap[0].size();
	guardPos = clamp(guardPos, { 0,0 }, { cols,rows });
	targetPos = clamp(targetPos, { 0,0 }, { cols,rows });

	vector<vector<char>> visited = vector<vector<char>>(rows, vector<char>(cols));
	queue<Node> q;
	q.push(Node(guardPos, Distance(guardPos, targetPos), true, ivec2(0)));
	//visited[playerPos.y][playerPos.x] = 1;

	// 
	const vector<ivec2> directions = { ivec2(1,0),ivec2(-1,0),ivec2(0,1),ivec2(0,-1), ivec2(-1,-1),ivec2(-1,1),ivec2(1,-1),ivec2(1,1) };
	int searchNum = 0;

	Node cur;
	while (!q.empty())
	{
		searchNum++;
		cur = q.front();
		auto curPos = cur.pos;
		if (curPos == targetPos)
			break;

		q.pop();

		// if visited
		if (visited[curPos.y][curPos.x])
			continue;

		visited[curPos.y][curPos.x] = 1;

		// for all the directions
		for (auto dir : directions)
		{
			ivec2 newPos = curPos + dir;
			if (PosIsValid(newPos, rows, cols) && visited[newPos.y][newPos.x] == 0 && levelMap[newPos.y][newPos.x] == ' ')
			{
				if (hasWallAtNeighbor(levelMap, newPos, 3))
					continue;

				ivec2 firstDir;
				if (cur.isStart)
					firstDir = dir; // record the start direction
				else
					firstDir = cur.firstDir; // equals to the start direction
				q.push(Node(newPos, Distance(newPos, targetPos), false, firstDir));
			}
		}
	}
	//cur.firstDir.y = -cur.firstDir.y;
	vec2 chaseVector = normalize(vec2(cur.firstDir));

	if (chaseVector.x != chaseVector.x)
	{
		srand(glfwGetTime());
		chaseVector = normalize(vec2(directions[ rand() % directions.size()]));
	}

	// print
	// cout << "Search Nums=" << searchNum << endl;
	// cout << "Chase Vector=(" << chaseVector.x << "," << chaseVector.y << ")" << endl;
	return chaseVector;
}


void AISystem::step(float elapsed_ms)
{
	if (enable == false)
		return;

	// trigger the trap effect, guards will start to chase the player with the shortest path
	// TODO: currently, the shortest path is guard move toward the player directly that will ignore wall, which need to be changed
	if (registry.trappables.size() > 0) {

		// refresh 
		if (remainWaitTime <= 0) {

			//debugging.in_debug_mode = true;

			cout << "AISystem::step  " << "update chasing" << endl;

			// get entity
			Entity player = registry.players.entities[0];
			Entity guard = registry.guards.entities[0];

			// get their motion
			Motion &player_motion = registry.motions.get(player);
			Motion &guard_motion = registry.motions.get(guard);


			//ivec2 vectorChaseIndex=calcChaseVector()
			Entity gameStateEntity = registry.gameStates.entities[0];
			GameState &gameState = registry.gameStates.get(gameStateEntity);

			// calculate the chase vector
			ivec2 playerPosIndex = player_motion.position / vec2(WALL_SIZE, WALL_SIZE);
			ivec2 guardPosIndex=guard_motion.position / vec2(WALL_SIZE, WALL_SIZE);
			vec2 vector_chase = calcChaseVectorV1(gameState.GetCurrentMap(), guardPosIndex, playerPosIndex);

			Character::Direction dir;
			auto &guardObj = registry.deadlys.get(guard);
			if (abs(vector_chase.x) > abs(vector_chase.y)) {
				if (vector_chase.x > 0) {
					dir = Character::Direction::RIGHT;
				}
				else {
					dir = Character::Direction::LEFT;
				}
			}
			else {
				if (vector_chase.y > 0) {
					dir = Character::Direction::DOWN;
				}
				else {
					dir = Character::Direction::UP;
				}
			}


			guardObj.SwitchDirection(dir, glfwGetTime());

			// update the velocity and angle of guard
			//guard_motion.angle = atan2(vector_chase.y, vector_chase.x);
			guard_motion.velocityGoal = vector_chase * GUARD_VELOCITY;

			remainWaitTime = CALC_INTERVAL;
		}
		else
		{
			remainWaitTime -= elapsed_ms / 1000.0f;
		}
	}
}

void AISystem::SetEnable(bool enable)
{
	this->enable = enable;
}
