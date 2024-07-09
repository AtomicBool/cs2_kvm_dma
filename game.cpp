#include "game.h"

extern uint64_t client_base;

//Pawn
int PlayerPawn::getHealth()
{
    return mem.Read<int>(ptr + OFFSET_HEALTH);
}

int PlayerPawn::getArmor()
{
    return mem.Read<int>(ptr + OFFSET_ARMOR);
}

int PlayerPawn::getShotsFired(){
    return mem.Read<int>(ptr + OFFSET_SHOTS_FIRED);
}

Vector2D PlayerPawn::getViewAngles(){
    return mem.Read<Vector2D>(ptr + OFFSET_EYE_ANGLE);
}

Vector2D PlayerPawn::getAimPunch(){
    return mem.Read<Vector2D>(ptr + OFFSET_AIM_PUNCH);
}

Vector PlayerPawn::getPos()
{
    return mem.Read<Vector>(ptr + OFFSET_OLD_ORIGIN);
}

Vector PlayerPawn::getCameraPos(){
	return mem.Read<Vector>(ptr + OFFSET_CAMERA_POS);
}

std::vector<BonePos> PlayerPawn::getBones(){
    std::vector<BonePos> bones;
    uint64_t GameSceneNode = mem.Read<uint64_t>(ptr + OFFSET_GAME_SCENE_NODE);
    uint64_t BoneArrayAddress = mem.Read<uint64_t>(GameSceneNode + OFFSET_BONE_ARRAY);
    BoneJointData BoneData;
    BonePos bones_tmp;
    for (int i = 0; i < 30; i++){
        BoneData = mem.Read<BoneJointData>(BoneArrayAddress + sizeof(BoneJointData) * i);
        bones_tmp.Pos = BoneData.Pos;
        Vector2D screen_pos;
        VMatrix m = mem.Read<VMatrix>(client_base + OFFSET_VIEW);
        if(WorldToScreen(bones_tmp.Pos, screen_pos, m, 1920, 1080)){
            bones_tmp.ScreenPos = screen_pos;
            bones_tmp.in_screen = true;
        }
        bones.push_back(bones_tmp);
    }
    return bones;
}

std::string PlayerPawn::getModelName()
{
    uint64_t GameSceneNode = mem.Read<uint64_t>(ptr + OFFSET_GAME_SCENE_NODE);
    uint64_t model_ptr = mem.Read<uint64_t>(GameSceneNode + OFFSET_MODEL_STATE + OFFSET_MODEL_NAME);
    std::string model_path = mem.readString(model_ptr);
    return model_path.substr(model_path.rfind("/") + 1, model_path.rfind(".") - model_path.rfind("/") - 1);
}

PlayerPawn getLocalPawn(){
    PlayerPawn local_pawn = PlayerPawn();
    local_pawn.ptr = mem.Read<uint64_t>(client_base + OFFSET_LOCAL_PAWN);
    return local_pawn;
}

//Controller
int PlayerController::getTeam(){
    return mem.Read<int>(ptr + OFFSET_TEAM_NUM);
}

int PlayerController::getCash()
{
    return mem.Read<int>(mem.Read<uint64_t>(ptr + OFFSET_MONEY_SERVICE) + OFFSET_MONEY_ACCOUNT);
}

e_color PlayerController::getColor()
{
    e_color color = mem.Read<e_color>(ptr + OFFSET_COLOR);
    if (color == static_cast<e_color>(-1))
	{
    	return e_color::white;
	}
    return color;
}

std::string PlayerController::getName()
{
    return mem.readString(mem.Read<uint64_t>(ptr + OFFSET_NAME));
}

std::string PlayerController::getSteamID()
{
    return std::to_string(mem.Read<uint64_t>(ptr + OFFSET_STEAMID));
}

PlayerPawn PlayerController::getPawn(uint64_t EntityListEntry)
{
    PlayerPawn pawn = PlayerPawn();

	uint64_t EntityPawnListEntry = mem.Read<uint64_t>(EntityListEntry);
        uint64_t hPlayerPawn = mem.Read<uint64_t>(ptr + OFFSET_PLAYER_PAWN);
	EntityPawnListEntry = mem.Read<uint64_t>(EntityPawnListEntry + 0x10 + 8 * ((hPlayerPawn & 0x7FFF) >> 9));

	pawn.ptr = mem.Read<uint64_t>(EntityPawnListEntry + 0x78 * (hPlayerPawn & 0x1FF));
    return pawn;
}

PlayerController getLocalController()
{
    PlayerController local_controller = PlayerController();
    local_controller.ptr = mem.Read<uint64_t>(client_base + OFFSET_LOCAL_CONTROLLER);
    return local_controller;
}

PlayerController getController(uint64_t EntityListEntry, int i){
    PlayerController controller = PlayerController();
    controller.ptr = mem.Read<uint64_t>(EntityListEntry + (i + 1) * 0x78);
    return controller;
}

//Weapons

/**
 * @param weapon_ptr: address of weapon
 * @return the address of weapon data
*/
uint64_t Weapon::getWeaponData(uint64_t weapon_ptr){
    return mem.Read<uint64_t>(weapon_ptr + OFFSET_SUBCLASS_ID + 0x8);
}

/**
 * @param pawn: playerpawn
 * @return the address of ActiveWeapon's WeaponData
*/
uint64_t Weapon::getActiveWeapon(PlayerPawn pawn){
    uint64_t weapon_service_ptr = mem.Read<uint64_t>(pawn.ptr + OFFSET_WEAPON_SERVICE);
    uint64_t handle = mem.Read<uint64_t>(weapon_service_ptr + OFFSET_ACTIVE_WEAPON);
    uint64_t idx = handle & 0x7fff;
    uint64_t active_weapon_ptr = getControllerByID(idx);
    uint64_t active_weapon_data_ptr = mem.Read<uint64_t>(active_weapon_ptr + OFFSET_SUBCLASS_ID + 0x8);
    return active_weapon_data_ptr;
}

/**
 * @param weapon_data_ptr: Address of WeaponData
 * @param index: index you want to get
 * @return the address of weapon
*/
uint64_t Weapon::getWeaponFromIndex(uint64_t weapon_ptr, int index){
    uint64_t handle = mem.Read<uint64_t>(weapon_ptr + index * 0x4);
    uint64_t idx = handle & 0x7fff;
    return getControllerByID(idx);
}

/**
 * @param weapon_data_ptr: Address of WeaponData
 * @return name of weapon
*/
std::string Weapon::getWeaponName(uint64_t weapon_data_ptr)
{
	uint64_t name_ptr = mem.Read<uint64_t>(weapon_data_ptr + OFFSET_WEAPON_NAME);
	if (!name_ptr)
		return "invalid";

	std::string name = mem.readString(name_ptr);
	if (name.empty())
		return "invalid";
	// @note: remove "weapon_" from the string
	name.erase(name.begin(), name.begin() + 7);
    return name;
}

/**
 * @param weapon_data_ptr: Address of WeaponData
 * @return ID of weapon
*/
e_weapon_type Weapon::getWeaponID(uint64_t weapon_data_ptr){
    return mem.Read<e_weapon_type>(weapon_data_ptr + OFFSET_WEAPON_ID);
}

std::pair<int, uint64_t> Weapon::getWeapons(PlayerPawn pawn){
    // @note: num weapons is at + 0x0, data is at + 0x8 (https://www.unknowncheats.me/forum/3711351-post132.html)
    uint64_t weapon_service_ptr = mem.Read<uint64_t>(pawn.ptr + OFFSET_WEAPON_SERVICE);
    return
	{
	    mem.Read<int>(weapon_service_ptr + OFFSET_MWEAPONS),
		mem.Read<uint64_t>(weapon_service_ptr + OFFSET_MWEAPONS + 0x08)
	};
}

void Weapon::Update(PlayerPawn pawn)
{
    current_weapon = getWeaponName(getActiveWeapon(pawn));
    std::pair<int, uint64_t> my_weapons = getWeapons(pawn);
    if(!my_weapons.first) return;

    for(int i = 0; i < my_weapons.first; i++){
        uint64_t weapon_ptr = getWeaponFromIndex(my_weapons.second, i);
        uint64_t weapon_data_ptr = getWeaponData(weapon_ptr);

        std::string weapon_name = getWeaponName(weapon_data_ptr);
        e_weapon_type weapon_type = getWeaponID(weapon_data_ptr);

        switch (weapon_type)
		{
			case e_weapon_type::submachine_gun:
			case e_weapon_type::rifle:
			case e_weapon_type::shotgun:
			case e_weapon_type::sniper_rifle:
			case e_weapon_type::machine_gun:
				primary_weapon = weapon_name;
				break;

			case e_weapon_type::pistol:
				secondary_weapon = weapon_name;
				break;

			case e_weapon_type::knife:
            case e_weapon_type::taser:
				melee_set.insert(weapon_name);
				break;

			case e_weapon_type::grenade:
		    	utilities_set.insert(weapon_name);
			    break;
	}
    }
}

/**
 * @return controller address
*/
uint64_t getControllerByID(int idx)
{
    uint64_t entry_list = mem.Read<uint64_t>(mem.Read<uint64_t>(client_base + OFFSET_ENTITY_LIST) + 8 * (idx >> 9) + 16);
    return mem.Read<uint64_t>(entry_list + 120 * (idx & 0x1ff));
}

uint64_t getEntityByClassName(std::string class_name)
{
    for(int i = 64; i < 1024; i++){
        uint64_t entity_addr = getControllerByID(i);
        // m_pentity: 0x10;  m_designername: 0x20;
        std::string entity_type_name = mem.readString(mem.Read<uint64_t>(mem.Read<uint64_t>(entity_addr + 0x10) + 0x20));
        if(entity_type_name.find(class_name) != std::string::npos){
            return entity_addr;
        }
    }
    return 0;
}

// other funcs
bool WorldToScreen(const Vector& Pos, Vector2D& ToPos, VMatrix Matrix, int width, int height)
{
	/*
        printf("%f, %f, %f, %f\n", Matrix[0][1], Matrix[0][2], Matrix[0][3], Matrix[0][4]);
        printf("%f, %f, %f, %f\n", Matrix[1][1], Matrix[1][2], Matrix[1][3], Matrix[1][4]);
        printf("%f, %f, %f, %f\n", Matrix[2][1], Matrix[2][2], Matrix[2][3], Matrix[2][4]);
        printf("%f, %f, %f, %f\n\n", Matrix[3][1], Matrix[3][2], Matrix[3][3], Matrix[3][4]);
	*/
	float View = 0.f;
	float SightX = width / 2;
    	float SightY = height / 2;

	View = Matrix[3][0] * Pos.x + Matrix[3][1] * Pos.y + Matrix[3][2] * Pos.z + Matrix[3][3];

	if(View <= 0.01f) return false;

	ToPos.x = SightX + (Matrix[0][0] * Pos.x + Matrix[0][1] * Pos.y + Matrix[0][2] * Pos.z + Matrix[0][3]) / View * SightX;
	ToPos.y = SightY - (Matrix[1][0] * Pos.x + Matrix[1][1] * Pos.y + Matrix[1][2] * Pos.z + Matrix[1][3]) / View * SightY;

	return true;
}

void setViewAngles(Vector2D angles){
    mem.Write<Vector2D>(client_base + OFFSET_VIEW_ANGLES, angles);
}

Vector2D NormalizeAngles(Vector2D angles){
    if(angles.x > 89.0f && angles.x <= 180.0f) angles.x = 89.f;
    if(angles.x > 180.0f) angles.x -= 360.0f;
    if(angles.x < -89.f) angles.x = -89.f;
    if(angles.y > 180.f) angles.y -= 360.f;
    if(angles.y < -180.f) angles.y += 360.f;
    return angles;
}
