#include "game.h"

using json = nlohmann::json;
using ws = easywsclient::WebSocket;

uint64_t game_base;
uint64_t client_base;
uint64_t match_base;

typedef struct local_player{
    int teamID;
    int shotsFired;
    float sensitivity;
    Vector2D viewAngles;
    Vector2D punchAngles;
    Vector CameraPos;
};

typedef struct player{
    std::vector<BonePos> bones;
};

player players[64];
local_player local;

//input
c_keys keyboard;
QMP qmp;

//map
map_loader map;

//radar
ws::pointer web_socket;
std::string map_name;
json m_data{};

//aimbot
BonePos selected_bone;

/*
    Cheat Settings
*/
//web-radar
bool radar = false;

//rcs
bool rcs = true;

//aimbot
bool aimbot = true;
uint32_t aim_key = VK_XBUTTON2;
uint32_t secondary_key = VK_XBUTTON1;
float aim_range = 70.0f;

void RCS(){
    Vector2D prev_punchAngles = Vector2D{ 0 , 0 };

    while(game_base && rcs){
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

	    Vector2D viewAngles = local.viewAngles;
        Vector2D punchAngles = local.punchAngles;

        if(local.shotsFired > 1){
            Vector2D delta = -(prev_punchAngles - (punchAngles * 2.f));

            int mouse_relative_x = (int) (delta.y / (local.sensitivity * 0.0224f));
            int mouse_relative_y = (int) (delta.x / (local.sensitivity * 0.0224f));

            if(keyboard.IsKeyDown(0x01)) qmp.SmoothMove(mouse_relative_x, -mouse_relative_y);

            prev_punchAngles.x = punchAngles.x * 2.f;
            prev_punchAngles.y = punchAngles.y * 2.f;
        }else{
            prev_punchAngles.x = prev_punchAngles.y = 0.f;
        }
    }
}

void UpdatePlayers(){
    auto start = std::chrono::system_clock::now();
    while(game_base){
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
	    const auto now = std::chrono::system_clock::now();
        const auto duration = now - start;

        m_data = nlohmann::json{};

        //localpawn
	    PlayerPawn LocalPawn = getLocalPawn();
        PlayerController LocalController = getLocalController();
        local.teamID = LocalController.getTeam();
        local.shotsFired = LocalPawn.getShotsFired();
        local.viewAngles = LocalPawn.getViewAngles();
        local.punchAngles = LocalPawn.getAimPunch();
	    local.CameraPos = LocalPawn.getCameraPos();

        //mouse sensitivity
        uint64_t sensitivity_ptr = mem.Read<uint64_t>(client_base + OFFSET_SENSITIVITY);
        local.sensitivity = mem.Read<float>(sensitivity_ptr + 0x40); //0x40: dwSensitivity_sensitivity

	    //map name
        map_name = mem.readString(mem.Read<uint64_t>(match_base + OFFSET_MAP_NAME) + 0x04);
	    m_data["m_map"] = map_name;

        //radar
        int bomb_owner_idx;
        if(radar){
            //c4 info
            m_data["m_bomb"] = nlohmann::json{};
            uint64_t weapon_c4 = getEntityByClassName("weapon_c4");
            //weapon c4
            bomb_owner_idx = mem.Read<uint64_t>(weapon_c4 + OFFSET_OWNER_ENTITY) & 0xffff;
            uint64_t weapon_c4_game_scene_node = mem.Read<uint64_t>(weapon_c4 + OFFSET_GAME_SCENE_NODE);
            Vector weapon_c4_origin = mem.Read<Vector>(weapon_c4_game_scene_node + OFFSET_ORIGIN);
            m_data["m_bomb"]["x"] = weapon_c4_origin.x;
            m_data["m_bomb"]["y"] = weapon_c4_origin.y;
            //planted c4
            uint64_t planted_c4 = mem.Read<uint64_t>(mem.Read<uint64_t>(client_base + OFFSET_C4));
            bool state = mem.Read<bool>(client_base + OFFSET_C4 - 0x8);
            uint64_t c4_game_scene_node = mem.Read<uint64_t>(planted_c4 + OFFSET_GAME_SCENE_NODE);
            Vector c4_origin = mem.Read<Vector>(c4_game_scene_node + OFFSET_ORIGIN);
            if(state){
                m_data["m_bomb"]["x"] = c4_origin.x;
                m_data["m_bomb"]["y"] = c4_origin.y;
                m_data["m_bomb"]["m_blow_time"] = mem.Read<float>(planted_c4 + OFFSET_C4_BLOW_TIME) - mem.Read<float>(mem.Read<uint64_t>(client_base + OFFSET_GLOBAL_VARS) + 0x34);
                m_data["m_bomb"]["m_is_defused"] = mem.Read<bool>(planted_c4 + OFFSET_C4_DEFUSED);
                m_data["m_bomb"]["m_is_defusing"] = mem.Read<bool>(planted_c4 + OFFSET_C4_DEFUSING);
                m_data["m_bomb"]["m_defuse_time"] = mem.Read<float>(planted_c4 + OFFSET_C4_DEFUSE_TIME) - mem.Read<float>(mem.Read<uint64_t>(client_base + OFFSET_GLOBAL_VARS) + 0x34);
            }

            //radar
            m_data["m_local_team"] = local.teamID;

            //entitylist
            m_data["m_players"] = nlohmann::json::array();
        }

        uint64_t EntityListEntry = mem.Read<uint64_t>(mem.Read<uint64_t>(client_base + OFFSET_ENTITY_LIST) + 0x10);

        for(int i = 0; i < 64; i++){
            PlayerController PlayerController = getController(EntityListEntry, i);
            PlayerPawn PlayerPawn = PlayerController.getPawn(EntityListEntry);

            std::string player_name = PlayerController.getName();
            bool is_dead = (PlayerPawn.getHealth() <= 0);

            if(radar && !player_name.empty()){ //stupid way but works
                json m_player_data{};
                m_player_data["m_idx"] = i;
                m_player_data["m_name"] = PlayerController.getName();
                m_player_data["m_color"] = PlayerController.getColor();
                m_player_data["m_team"] = PlayerController.getTeam();
                m_player_data["m_health"] = PlayerPawn.getHealth();
                m_player_data["m_armor"] = PlayerPawn.getArmor();
                m_player_data["m_position"]["x"] = PlayerPawn.getPos().x;
                m_player_data["m_position"]["y"] = PlayerPawn.getPos().y;
                m_player_data["m_eye_angle"] = PlayerPawn.getViewAngles().y;
                m_player_data["m_is_dead"] = is_dead;
                m_player_data["m_model_name"] = PlayerPawn.getModelName();
                m_player_data["m_steam_id"] = PlayerController.getSteamID();
                m_player_data["m_money"] = PlayerController.getCash();
                uint64_t item_services = mem.Read<uint64_t>(PlayerPawn.ptr + OFFSET_ITEM_SERVICE);
                m_player_data["m_has_helmet"] = mem.Read<bool>(item_services + OFFSET_ITEM_HAS_HELMET);
                m_player_data["m_has_defuser"] = mem.Read<bool>(item_services + OFFSET_ITEM_HAS_DEFUSER);
                m_player_data["m_weapons"] = nlohmann::json{};
                m_player_data["m_has_bomb"] = (bomb_owner_idx == (mem.Read<uint64_t>(PlayerController.ptr + OFFSET_PLAYER_PAWN) & 0xffff));

                //weapons
                Weapon PlayerWeapon;
                PlayerWeapon.Update(PlayerPawn);
                m_player_data["m_weapons"]["m_active"] = PlayerWeapon.current_weapon;
                m_player_data["m_weapons"]["m_primary"] = PlayerWeapon.primary_weapon;
                m_player_data["m_weapons"]["m_secondary"] = PlayerWeapon.secondary_weapon;
                m_player_data["m_weapons"]["m_melee"] = std::vector<std::string>(PlayerWeapon.melee_set.begin(), PlayerWeapon.melee_set.end());
                m_player_data["m_weapons"]["m_utilities"] = std::vector<std::string>(PlayerWeapon.utilities_set.begin(), PlayerWeapon.utilities_set.end());

                m_data["m_players"].push_back(m_player_data);
            }

	    }
        //websocket
	    if (duration >= std::chrono::milliseconds(50) && radar)
        	{
            start = now;
	        web_socket->send(m_data.dump());
	    }
	    if(radar) web_socket->poll();
	    //printf("radar: %s\n", m_data.dump().c_str());
    }
}

typedef struct aimPosList{
    PlayerPawn pawn;
    float dist;
    int x = 0;
    int y = 0;
}aimPosList;

void Aimbot(){
    uint64_t last_target;

    while(game_base && aimbot){
        PlayerPawn LocalPawn = getLocalPawn();
        Vector local_pos = LocalPawn.getPos();
        
        uint64_t EntityListEntry = mem.Read<uint64_t>(mem.Read<uint64_t>(client_base + OFFSET_ENTITY_LIST) + 0x10);

        std::vector<aimPosList> list;

        for(int i = 0; i < 64; i++){
            PlayerController PlayerController = getController(EntityListEntry, i);
            PlayerPawn PlayerPawn = PlayerController.getPawn(EntityListEntry);

            std::string player_name = PlayerController.getName();
            bool is_dead = (PlayerPawn.getHealth() <= 0);
        
            //aimbot
            if(keyboard.IsKeyDown(aim_key)){
                BonePos preferred_bone;
                float arrange_distence = FLT_MAX; //arrange
                if(player_name.empty()) continue;
                players[i].bones = PlayerPawn.getBones();
                //player not in screen
                if(!players[i].bones[head].in_screen && !players[i].bones[leg_lower_L].in_screen && !players[i].bones[leg_lower_R].in_screen){
                    continue;
                }else if(PlayerPawn.ptr == LocalPawn.ptr){
                    continue;
                }else if (!is_dead)
                {
                    bool has_target = false;
                    //select the cloest bone
                    for(BonePos bone: players[i].bones){
                        if(bone.in_screen){
                            float screen_dist = bone.ScreenPos.DistTo(Vector2D{960, 540});
                            if(screen_dist < arrange_distence && screen_dist <= aim_range && map.is_visible(local.CameraPos, bone.Pos)){
                                //if the dist is cloest and bone is visible
                                arrange_distence = screen_dist;
                                preferred_bone = bone;
                                has_target = true;
                            }
                        }
                    }
                    float screen_dist = players[i].bones[head].ScreenPos.DistTo(Vector2D{960, 540});
                    
                    if(keyboard.IsKeyDown(secondary_key) && screen_dist <= aim_range && map.is_visible(local.CameraPos, players[i].bones[head].Pos)){
                        preferred_bone = players[i].bones[head];
                    }
                    selected_bone = preferred_bone;
                    Vector2D aimPos_screen = selected_bone.ScreenPos;
                    if(abs(aimPos_screen.x - 960) < 2 && abs(aimPos_screen.y - 960) < 2) continue;
                    aimPos_screen.x -= 960;
                    aimPos_screen.y -= 540;
                    
                    if(has_target) list.push_back({PlayerPawn, PlayerPawn.getPos().DistTo(local_pos), (int)aimPos_screen.x, (int)aimPos_screen.y});

                    has_target = false;
                }
            }
        }
        //qmp.SmoothMove((int)aimPos_screen.x, (int)aimPos_screen.y);

        aimPosList aimPos;
        float minimum_dist = FLT_MAX;
        for(auto pos: list){
            aimPos = (minimum_dist > pos.dist) ? pos : aimPos;
            if(pos.pawn.ptr == last_target){
                aimPos = pos;   
                break;
            }
        }

        if(abs(aimPos.dist) > 1){
            qmp.SmoothMove(aimPos.x, aimPos.y);
            last_target = aimPos.pawn.ptr;
        }
    }
}

int main(int argc, char *argv[]){
    if (!mem.Init("cs2.exe", true, false))
    {
	    std::cout << "[-] Failed to open game process!" << std::endl;
    }std::cout << "[+] Memory API initialized!" << std::endl;

    if(argc > 1){
        std::cout << "[+] Loading meshes from file " << argv[1] << ".tri" << std::endl;
        map.load_map(argv[1]);
    }
    else printf("[-] Please input map name\n");

    game_base = mem.get_base_address();
    client_base = mem.get_base_address("client.dll");
    match_base = mem.get_base_address("matchmaking.dll");

    LOG("[+] CS2 Process Found\n");
    LOG("   - cs2.exe 	        Base: 0x%lx\n", game_base);
    LOG("   - client.dll         Base: 0x%lx\n", client_base);

    bool init_keyboard = keyboard.InitKeyboard();
    if (!init_keyboard)
    {
        std::cout << "[-] Failed to initialize keyboard , skipped mouse init!" << std::endl;
    }else{
        std::cout << "[+] Keyboard initialized!" << std::endl;
        qmp.Connect("127.0.0.1", 9567);
        qmp.EnableCommands();
        std::cout << "[+] Mouse initialized!" << std::endl;
        if(qmp.InitMouseAI()){
            std::cout << "[+] Mouse AI initialized!" << std::endl;
        }
    }

    web_socket = ws::from_url("ws://127.0.0.1:8081/cs2_webradar");
    if (!web_socket)
    {
        std::cout << "[-] Failed to initialize WebSocket!" << std::endl;
    }std::cout << "[+] WebSocket initialized!" << std::endl;

    /*  !DO IN SAME TIME!
        1.  <domain xmlns:qemu="http://libvirt.org/schemas/domain/qemu/1.0"
        2.  <qemu:commandline>
                <qemu:arg value="-qmp"/>
                <qemu:arg value="tcp:127.0.0.1:6448,server,nowait"/>
            </qemu:commandline>
    */

    std::thread rcs_thr = std::thread(RCS);
    rcs_thr.detach();
    std::thread aimbot_thr = std::thread(Aimbot);
    aimbot_thr.detach();
    std::thread update_players_thr = std::thread(UpdatePlayers);
    update_players_thr.detach();

    while(game_base){
        if(keyboard.IsKeyDown(VK_F4)){
            web_socket->close();

            map.unload();

            qmp.Disconnect();

            //threads
            rcs_thr.~thread();
            aimbot_thr.~thread();
            update_players_thr.~thread();

            game_base = 0;
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    return 0;
}
