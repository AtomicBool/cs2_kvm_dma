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

uint64_t client_base; //client.dll base-address

//Main
uint64_t local_pawn;
int localhealth;

bool loop = false; //to exit thread

//Read Function
static void ReadLoop(){
	loop = true;
	while(loop && client_base != 0){
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
		
		//localpawn index
		csgo_mem.Read<uint64_t>(client_base + LocalPawn, local_pawn);

		csgo_mem.Read<int>(local_pawn + m_iHealth, localhealth);

		printf("%d\n",localhealth);
	}
	loop = false;
}

int main(int argc, char *argv[])
{
	if(geteuid() != 0)
	{
		printf("[Error] %s is not running as root\n", argv[0]);
		return 0;
	}

	std::srand(static_cast<unsigned int>(std::time(nullptr)));

	const char* csgo_proc_name = "cs2.exe";

	std::thread read_thr;

	while(active)
	{
		if(csgo_mem.get_proc_status() != process_status::FOUND_READY)
		{
			if(loop){
				loop = false;
				read_thr.~thread();
			}
			std::this_thread::sleep_for(std::chrono::seconds(1));
			printf("[Warn]Searching for CS2..\n");

			csgo_mem.open_proc(csgo_proc_name);

			if(csgo_mem.get_proc_status() == process_status::FOUND_READY)
			{
				client_base = csgo_mem.GetModuleAddress(csgo_proc_name,"client.dll");
				printf("Found CounterStrike 2!\n");
				printf("[Info]client.dll Base: %lx\n", client_base);

				read_thr = std::thread(ReadLoop);

				read_thr.detach();
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
