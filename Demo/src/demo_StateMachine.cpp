// Resources
	// https://kenney.nl/assets/medieval-rts
	// https://www.dafont.com/pixelplay.font
	// https://www.dafont.com/sunny-spells-basic.font

#include "ViennaGameAILibrary.hpp"

#include "raylib.h"
#include "raymath.h"
#include "Timer.h"
#include <sstream>
#include <string>

constexpr uint32_t screen_width = 1200;
constexpr uint32_t screen_height = 900;

float tile_size = 50.0f;

class Mine
{
public:
	Mine(uint32_t size, VGAIL::Vec2ui pos)
		: size(size)
		, position(pos)
	{}

	~Mine() {}

	Rectangle get_rectangle()
	{
		return { static_cast<float>(position.x * tile_size),
				 static_cast<float>(position.y * tile_size),
				 tile_size, tile_size };
	}

	uint32_t size;
	VGAIL::Vec2ui position;
};

int main(int argc, char* argv[])
{
	uint32_t navmesh_width = static_cast<uint32_t>(screen_width / tile_size);
	uint32_t navmesh_height = static_cast<uint32_t>((screen_height - 100) / tile_size);
	VGAIL::NavMesh* navmesh = new VGAIL::NavMesh(navmesh_width, navmesh_height, 30);

	VGAIL::Vec2ui worker_position = VGAIL::Vec2ui(10, 10);
	Rectangle worker = { static_cast<float>(worker_position.x * tile_size),
						static_cast<float>(worker_position.y * tile_size),
						tile_size, tile_size };
	Color worker_color = BLUE;

	if (navmesh->get_node(worker_position).state == VGAIL::NodeState::OBSTRUCTABLE)
	{
		navmesh->get_node(worker_position).state = VGAIL::NodeState::WALKABLE;
	}

	VGAIL::Vec2ui home_position = VGAIL::Vec2ui(3, 2);
	Rectangle home = { static_cast<float>(home_position.x * tile_size),
						static_cast<float>(home_position.y * tile_size),
						tile_size, tile_size };

	if (navmesh->get_node(home_position).state == VGAIL::NodeState::OBSTRUCTABLE)
	{
		navmesh->get_node(home_position).state = VGAIL::NodeState::WALKABLE;
	}

	Mine* mine_1 = new Mine(3,  VGAIL::Vec2ui{ 20, 15 });
	Mine* mine_2 = new Mine(10, VGAIL::Vec2ui{ 13,  9 });
	Mine* mine_3 = new Mine(6,  VGAIL::Vec2ui{  3, 11 });
	Mine* mine_4 = new Mine(21, VGAIL::Vec2ui{ 16,  7 });
	Mine* mine_5 = new Mine(18, VGAIL::Vec2ui{  7,  2 });

	std::vector<Mine*> mines;
	mines.push_back(mine_1);
	mines.push_back(mine_2);
	mines.push_back(mine_3);
	mines.push_back(mine_4);
	mines.push_back(mine_5);

	for(Mine* mine : mines)
	{
		if (navmesh->get_node(mine->position).state == VGAIL::NodeState::OBSTRUCTABLE)
		{
			navmesh->get_node(mine->position).state = VGAIL::NodeState::WALKABLE;
		}
	}

	Mine* current_mine = nullptr;

	uint32_t home_load = 0;
	uint32_t current_load = 0;
	uint32_t max_load_capacity = 5;
	float load_time = 0.0f;
	float load_time_cooldown = 1.0f;

	float worker_speed = 100.0f;
	
	std::vector<VGAIL::Vec2ui> path;
	int32_t current_path_index = -1;

	InitWindow(screen_width, screen_height, "Demo");
	SetTargetFPS(60);

	Font large_font = LoadFont("Demo/res/pixelplay.ttf");
	Font tiny_font = LoadFont("Demo/res/SunnySpellsBasic.ttf");

    Texture2D home_texture = LoadTexture("Demo/res/home.png");
	Rectangle home_texture_src = { 0.0f, 0.0f, static_cast<float>(home_texture.width), static_cast<float>(home_texture.height) };
	Rectangle home_texture_dest = { home_position.x * tile_size, home_position.y * tile_size, tile_size, tile_size };

	Texture2D mine_texture = LoadTexture("Demo/res/mine.png");
	Rectangle mine_texture_src = { 0.0f, 0.0f, static_cast<float>(mine_texture.width), static_cast<float>(mine_texture.height) };

	Texture2D worker_texture = LoadTexture("Demo/res/worker.png");
	Rectangle worker_texture_src = { 0.0f, 0.0f, static_cast<float>(worker_texture.width), static_cast<float>(worker_texture.height) };

	Texture2D ground_texture = LoadTexture("Demo/res/ground.png");
	Texture2D obstacle_texture = LoadTexture("Demo/res/obstacle.png");

	VGAIL::StateMachine state_machine;

	VGAIL::State* dropOffState = state_machine.create_state();
	VGAIL::State* locateMineState = state_machine.create_state();
	VGAIL::State* locateHomeState = state_machine.create_state();
	VGAIL::State* idleState = state_machine.create_state();
	VGAIL::State* collectState = state_machine.create_state();

	uint32_t current_state_index = 0;

	dropOffState->add_transition(locateMineState, [&]() {	
		return current_load == 0; 
	});

	locateMineState->add_transition(idleState, [&](){
		return current_mine->size == 0 || path.size() == 0;
	});

	locateMineState->add_transition(collectState, [&](){
		return current_path_index == path.size() - 1;
	});

	collectState->add_transition(locateHomeState, [&](){
		return current_load == max_load_capacity || (current_mine->size == 0 && current_load > 0);
	});

	locateHomeState->add_transition(dropOffState, [&](){
		return current_path_index == path.size() - 1;
	});

	dropOffState->onEnterCallback = [&](){
		current_state_index = 0;		
	};

	dropOffState->onUpdateCallback = [&](float delta) {
		current_state_index = 0;
		if (current_load > 0)
		{
			load_time += delta;
			if (load_time > load_time_cooldown)
			{
				current_load--;
				home_load++;
				load_time = 0.0f;
			}
		}
	};

	locateHomeState->onEnterCallback = [&](){
		current_state_index = 1;

		VGAIL::Vec2ui worker_pos = VGAIL::Vec2ui{ static_cast<uint32_t>(worker.x / tile_size), static_cast<uint32_t>(worker.y / tile_size) };
		path = navmesh->A_star(worker_pos, home_position);

		if (path.size() > 0)
		{				
			current_path_index = 0;
		}
	};

	locateHomeState->onUpdateCallback = [&](float delta){		
		if (current_path_index < path.size() - 1)
		{
			VGAIL::Vec2ui target_node = path[current_path_index];
			VGAIL::Vec2f target = VGAIL::Vec2f{ static_cast<float>(target_node.x) * tile_size, static_cast<float>(target_node.y) * tile_size };

			if (std::abs(worker.x - static_cast<float>(target.x)) < 1.0f && std::abs(worker.y - static_cast<float>(target.y)) < 1.0f)
			{
				worker.x = target.x;
				worker.y = target.y;

				current_path_index++;
				return;
			}

			VGAIL::Vec2f direction = target - VGAIL::Vec2f{ worker.x, worker.y };
			direction.normalize();

			worker.x += direction.x * delta * worker_speed;
			worker.y += direction.y * delta * worker_speed;
		}
	};

	locateHomeState->onExitCallback = [&]() {
		path.clear();
		current_path_index = -1;
	};

	locateMineState->onEnterCallback = [&](){
		current_state_index = 2;		

		VGAIL::Vec2ui worker_pos = VGAIL::Vec2ui{ static_cast<uint32_t>(worker.x / tile_size), static_cast<uint32_t>(worker.y / tile_size) };

		uint32_t closest_mine_index = 0;
		uint32_t closest_distance = 10000;

		for(uint32_t i = 0; i < mines.size(); i++)
		{
			if(mines[i]->size == 0)
				continue;
		
			path = navmesh->A_star(worker_pos, mines[i]->position);

			if (path.size() > 0)
			{
				if(path.size() < closest_distance)
				{
					closest_mine_index = i;
					closest_distance = path.size();
				}
			}
		}

		current_mine = mines[closest_mine_index];
		current_path_index = 0;
		path = navmesh->A_star(worker_pos, current_mine->position);
	};

	locateMineState->onUpdateCallback = [&](float delta){
		if (current_path_index < path.size() - 1)
		{
			VGAIL::Vec2ui target_node = path[current_path_index];
			VGAIL::Vec2f target = VGAIL::Vec2f{ static_cast<float>(target_node.x) * tile_size, static_cast<float>(target_node.y) * tile_size };

			if (std::abs(worker.x - static_cast<float>(target.x)) < 1.0f && std::abs(worker.y - static_cast<float>(target.y)) < 1.0f)
			{
				worker.x = target.x;
				worker.y = target.y;

				current_path_index++;
				return;
			}

			VGAIL::Vec2f direction = target - VGAIL::Vec2f{ worker.x, worker.y };
			direction.normalize();

			worker.x += direction.x * delta * worker_speed;
			worker.y += direction.y * delta * worker_speed;
		}
	};

	locateMineState->onExitCallback = [&]() {
		path.clear();
		current_path_index = -1;
	};

	collectState->onEnterCallback = [&]() {
		current_state_index = 3;
	};

	collectState->onUpdateCallback = [&](float delta){
		if (current_load <= max_load_capacity && current_mine->size > 0)
		{
			load_time += delta;
			if (load_time > load_time_cooldown)
			{
				current_load++;
				current_mine->size--;
				load_time = 0.0f;
			}
		}
	};

	idleState->onEnterCallback = [&]() {
		current_state_index = 4;
	};


	while (!WindowShouldClose())
	{
		float delta = GetFrameTime();

		state_machine.update(delta);

		BeginDrawing();
		ClearBackground(WHITE);

		for (uint32_t y = 0; y < navmesh_height; y++)
		{
			for (uint32_t x = 0; x < navmesh_width; x++)
			{
				VGAIL::NodeData& node = navmesh->get_node(VGAIL::Vec2ui(x, y));

				if (node.state == VGAIL::NodeState::OBSTRUCTABLE)
				{
					Rectangle obstacle_texture_src = { 0.0f, 0.0f, static_cast<float>(obstacle_texture.width), static_cast<float>(obstacle_texture.height) };
					Rectangle obstacle_texture_dest = { node.pos.x * tile_size, node.pos.y * tile_size, tile_size, tile_size };
					DrawTexturePro(obstacle_texture, obstacle_texture_src, obstacle_texture_dest, Vector2{0.0f, 0.0f}, 0.0f, WHITE);
				}
				else 
				{
					Rectangle ground_texture_src = { 0.0f, 0.0f, static_cast<float>(ground_texture.width), static_cast<float>(ground_texture.height) };
					Rectangle ground_texture_dest = { node.pos.x  * tile_size, node.pos.y * tile_size, tile_size, tile_size };
					DrawTexturePro(ground_texture, ground_texture_src, ground_texture_dest, Vector2{0.0f, 0.0f}, 0.0f, WHITE);
				}
			}
		}

		DrawTexturePro(home_texture, home_texture_src, home_texture_dest, Vector2{0.0f, 0.0f}, 0.0f, WHITE);
		DrawTexturePro(worker_texture, worker_texture_src, worker, Vector2{0.0f, 0.0f}, 0.0f, WHITE);

		for(Mine* mine : mines)
		{
			DrawTexturePro(mine_texture, mine_texture_src, mine->get_rectangle(), Vector2{0.0f, 0.0f}, 0.0f, WHITE);
		}

		std::stringstream ss;
		ss << current_load;
		DrawTextEx(tiny_font, ss.str().c_str(), Vector2{worker.x + 7.0f, worker.y}, 15, 1.0f, BLACK);

		ss.str(std::string());
		ss << home_load;
		DrawTextEx(tiny_font, ss.str().c_str(), Vector2{home.x + 2.5f, home.y}, 15, 1.0f, BLACK);

		for(Mine* mine : mines)
		{
			ss.str(std::string());
			ss << mine->size;
			DrawTextEx(tiny_font, ss.str().c_str(), Vector2{mine->position.x * tile_size + 3.0f, mine->position.y * tile_size}, 15, 1.0f, BLACK);
		}

		std::string current_state = "";

		switch(current_state_index)
		{
			case 0:
				current_state = "Dropping off materials";
				break;
			case 1:
				current_state = "Locating home";
				break;
			case 2:
				current_state = "Locating nearest mine";
				break;			
			case 3:
				current_state = "Collecting materials";
				break;
			case 4:
				current_state = "Idle";
				break;
			default:
				current_state = "";
		}

		ss.str(std::string());
		ss << "Current state: " << current_state;
		const char* full_text = ss.str().c_str();
		Vector2 text_position = Vector2{ screen_width / 2.0f - 200.0f, screen_height - 70.0f };
		DrawTextEx(large_font, full_text, text_position, large_font.baseSize, 3.0f, BLACK);
		EndDrawing();
	}
	
	UnloadTexture(obstacle_texture);
	UnloadTexture(ground_texture);
	UnloadTexture(home_texture);
	UnloadTexture(mine_texture);
	UnloadTexture(worker_texture);
	
	CloseWindow();

	for(Mine* mine: mines)
	{
		delete mine;
	}
	
	return 0;
}