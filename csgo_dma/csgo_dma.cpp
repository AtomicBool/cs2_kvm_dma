#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <random>
#include <chrono>
#include <iostream>
#include <cfloat>
#include "Game.h"
#include <thread>
#include <ctime>
#include <cstdlib>

Memory csgo_mem;

bool active = true;

uint64_t engine_base; //engine.dll base-address
uint64_t client_base; //client.dll base-address

//Main
int localplayer;
int local_team;
int localhealth;

//TriggerBot Settings
bool trigger_t = false; //to exit thread
bool trigger_bot_enable = true;	//triggerbot key down
int release_time_max = 180;
int release_time_min = 150;
int press_time_max = 80;
int press_time_min = 50;

int RandomInt(int max, int min) {
    int result = min + rand() % (max - min + 1);
    return result;
}

//TriggerBot Void
static void TriggerbotLoop(){
	trigger_t = true;
	while(trigger_t && client_base != 0){
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	
		if(!trigger_bot_enable){
			continue;
		}
		
		//localplayer index
		csgo_mem.Read<int>(offset::dwLocalPlayer + client_base, localplayer);
		//sometimes ,index is under 0 ,convert it with abs()
        localplayer = abs(localplayer);		

		csgo_mem.Read<int>(localplayer + offset::m_iHealth, localhealth);
		
		csgo_mem.Read<int>(localplayer + offset::m_iTeamNum, local_team);	

		if(!localhealth){
			continue;
		}

		int CrosshairId;
		csgo_mem.Read<int>(localplayer + offset::m_iCrosshairId, CrosshairId);
		
		if(!CrosshairId || CrosshairId > 64){
			continue;
		}
		
		int player_index;
		csgo_mem.Read<int>(client_base + offset::dwEntityList + (CrosshairId -1) * 0x10, player_index);
		//each player in memory -> 0x10
		//convert index when under 0
		player_index = abs(player_index);

		//aiming target		
		int player_health;
		csgo_mem.Read<int>(player_index + offset::m_iHealth, player_health);
		
		int player_team;
		csgo_mem.Read<int>(player_index + offset::m_iTeamNum, player_team);			
		
		if (!player_health){
			continue;
		}
		if(player_team == local_team){
			continue;	
		}
		
		//random press/release time
		std::this_thread::sleep_for(std::chrono::milliseconds(RandomInt(release_time_max,release_time_min)));
		csgo_mem.Write<int>(client_base + offset::dwForceAttack, 6);
		
		std::this_thread::sleep_for(std::chrono::milliseconds(RandomInt(press_time_max,press_time_min)));
		csgo_mem.Write<int>(client_base + offset::dwForceAttack, 4);
	}
	trigger_t = false;
}

int main(int argc, char *argv[])
{
	if(geteuid() != 0)
	{
		printf("[Error] %s is not running as root\n", argv[0]);
		return 0;
	}

	std::srand(static_cast<unsigned int>(std::time(nullptr)));

	const char* csgo_proc_name = "csgo.exe";

	std::thread triggerbot_thr;

	while(active)
	{
		if(csgo_mem.get_proc_status() != process_status::FOUND_READY)
		{
			if(trigger_t){
				trigger_t = false;
				triggerbot_thr.~thread();
			}
			std::this_thread::sleep_for(std::chrono::seconds(1));
			printf("[Warn]Searching for CS:GO..\n");

			csgo_mem.open_proc(csgo_proc_name);

			if(csgo_mem.get_proc_status() == process_status::FOUND_READY)
			{
				client_base = csgo_mem.GetModuleAddress("csgo.exe","client.dll");
				engine_base = csgo_mem.GetModuleAddress("csgo.exe","engine.dll");
				printf("Found CounterStrike!\n");
				printf("[Info]client.dll Base: %lx\n", client_base);
				printf("[Info]engine.dll Base: %lx\n", engine_base);

				triggerbot_thr = std::thread(TriggerbotLoop);

				triggerbot_thr.detach();
			}
		}
		else
		{
			csgo_mem.check_proc();
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}

	return 0;
}
