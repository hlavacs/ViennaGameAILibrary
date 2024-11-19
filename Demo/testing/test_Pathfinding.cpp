/**
* The Vienna Game AI Library
*
* (c) bei Lavinia-Elena Lehaci, University of Vienna, 2024
*
*/

#include "ViennaGameAILibrary.hpp"
#include "../src/Timer.h"

int main(int argc, char* argv[])
{
	uint32_t screenWidth = 1000;
	uint32_t screenHeight = 1000;
	float gridStrideX = 100.0f;
	float gridStrideY = 100.0f;

	uint32_t navmeshWidth = static_cast<uint32_t>(screenWidth / gridStrideX);
	uint32_t navmeshHeight = static_cast<uint32_t>(screenHeight / gridStrideY);
	VGAIL::NavMesh* navmesh = new VGAIL::NavMesh(navmeshWidth, navmeshHeight, 30.0f, 10.0f, 10.0f);

	VGAIL::Vec2ui startPosition = VGAIL::Vec2ui(1, 1);
	VGAIL::Vec2ui targetPosition = VGAIL::Vec2ui(navmeshWidth - 1, navmeshHeight - 1);

	if (navmesh->getNode(startPosition).state == VGAIL::NodeState::OBSTRUCTABLE)
		navmesh->getNode(startPosition).state = VGAIL::NodeState::WALKABLE;

	if (navmesh->getNode(targetPosition).state == VGAIL::NodeState::OBSTRUCTABLE)
		navmesh->getNode(targetPosition).state = VGAIL::NodeState::WALKABLE;

	std::cout << ">> Number of nodes: " << navmeshWidth * navmeshHeight << std::endl;

	Timer timer_Preprocess(">> Preprocessing time");
	Timer timer_A("A* time");
	Timer timer_GP("GP time");

	timer_Preprocess.start();
	navmesh->preprocess(true, 24);
	timer_Preprocess.end();
	timer_Preprocess.print();

	double total_A = 0.0;
	double total_GP = 0.0;
	double count_A = 0.0;
	double count_GP = 0.0;
	bool testing = true;

	while (testing)
	{
		if(count_A < 100.0)
		{
			timer_A.start();
			navmesh->findPath(startPosition, targetPosition);
			timer_A.end();
			
            total_A += timer_A.getDuration();
			count_A++;
		}

		if(count_GP < 100.0)
		{
			timer_GP.start();
			navmesh->findPreprocessedPath(startPosition, targetPosition);
			timer_GP.end();

			total_GP += timer_GP.getDuration();
			count_GP++;
		}

		if(count_A == 100.0 && count_GP == 100.0)
		{
			std::cout << ">> Average A*: " << total_A / count_A << " microseconds" << std::endl;
			std::cout << ">> Average GP: " << total_GP / count_GP << " microseconds" << std::endl;

			testing = false;
		}
	}

	return 0;
}
