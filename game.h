#pragma once

#include "sdk/Memory.h"
#include "offset.h"

struct BoneJointData
{
	Vector Pos;
	char pad[0x14];
};

struct BonePos
{
	Vector Pos;
	Vector2D ScreenPos;
	bool in_screen = false;
};

//Pawn
class PlayerPawn
{
public:
	uint64_t ptr = 0;
public:
    int getHealth();
    int getArmor();
    int getShotsFired();
    Vector2D getViewAngles();
    Vector2D getAimPunch();
    Vector getPos();
    Vector getCameraPos();
    std::vector<BonePos> getBones();
    std::string getModelName();
};

PlayerPawn getLocalPawn();

//Controller
class PlayerController
{
public:
	uint64_t ptr = 0;
public:
    PlayerPawn getPawn(uint64_t EntityListEntry);
    int getTeam();
    int getCash();
    e_color getColor();
    std::string getName();
    std::string getSteamID();
};
PlayerController getLocalController();
PlayerController getController(uint64_t EntityListEntry, int i);

//Weapon
class Weapon
{
public:
    uint64_t ptr = 0;
    std::string current_weapon;
    std::string primary_weapon;
    std::string secondary_weapon;
    std::set<std::string> utilities_set{};
    std::set<std::string> melee_set{};

public:
    uint64_t getWeaponData(uint64_t weapon_ptr);
    std::string getWeaponName(uint64_t weapon_data_ptr);
    e_weapon_type getWeaponID(uint64_t weapon_data_ptr);
    uint64_t getWeaponFromIndex(uint64_t weapon_ptr, int index);
    uint64_t getActiveWeapon(PlayerPawn pawn);
    std::pair<int, uint64_t> getWeapons(PlayerPawn pawn);

    void Update(PlayerPawn pawn);
};


//EntityList
uint64_t getControllerByID(int idx);
uint64_t getEntityByClassName(std::string class_name);

//View
bool WorldToScreen(const Vector& Pos, Vector2D& ToPos, VMatrix Matrix, int width, int height);
void setViewAngles(Vector2D angles);
Vector2D NormalizeAngles(Vector2D angles);
