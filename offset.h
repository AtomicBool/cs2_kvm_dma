//https://github.com/a2x/cs2-dumper/blob/main/output/offsets.hpp
#define OFFSET_LOCAL_PAWN           0x181A9B8   	//dwLocalPlayerPawn
#define OFFSET_LOCAL_CONTROLLER     0x1A04768   	//dwLocalPlayerController
#define OFFSET_VIEW_ANGLES          0x1A23848   	//dwViewAngles
#define OFFSET_SENSITIVITY          0x1A13248   	//dwSensitivity
#define OFFSET_VIEW					0x1A16A60		//dwViewMatrix
#define OFFSET_ENTITY_LIST			0x19B49B8		//dwEntityList
#define OFFSET_GLOBAL_VARS			0x180E500		//dwGlobalVars
#define OFFSET_MAP_NAME				0x1A42E0 		//dwGameTypes_mapName
#define OFFSET_C4					0x1A1B7A8		//dwPlantedC4

//https://github.com/a2x/cs2-dumper/blob/main/output/client.dll.hpp
#define OFFSET_SHOTS_FIRED          0x22A4      	//m_iShotsFired
#define OFFSET_EYE_ANGLE            0x1388      	//m_angEyeAngles
#define OFFSET_AIM_PUNCH	    	0x14CC			//m_aimPunchAngle
#define OFFSET_GAME_SCENE_NODE		0x308    		//m_pGameSceneNode
#define OFFSET_BONE_ARRAY			0x170 + 0x88	//m_modelState + CGameSceneNode::m_vecOrigin
#define OFFSET_CAMERA_POS	    	0x12D4			//m_vecLastClipCameraPos
#define OFFSET_PLAYER_PAWN			0x7DC			//m_hPlayerPawn
#define OFFSET_TEAM_NUM				0x3C3			//m_iTeamNum
#define OFFSET_HEALTH				0x324			//m_iHealth
#define OFFSET_ARMOR				0x22c0			//m_armorvalue
#define OFFSET_WEAPON_SERVICE		0x10F8			//m_pweaponservices
#define OFFSET_ACTIVE_WEAPON		0x58			//m_hactiveweapon
#define OFFSET_SUBCLASS_ID			0x358			//m_nSubclassID
#define OFFSET_WEAPON_NAME			0xC28			//m_szName
#define OFFSET_MWEAPONS				0x40			//m_hmyweapons
#define OFFSET_WEAPON_ID			0x250			//m_weapontype
#define	OFFSET_OWNER_ENTITY			0x420			//m_hownerentity
#define OFFSET_ORIGIN				0xD0			//m_vecabsorigin
#define OFFSET_OLD_ORIGIN			0x1274			//m_vOldOrigin
#define OFFSET_NAME					0x740			//m_sSanitizedPlayerName
#define OFFSET_COLOR				0x728			//m_iCompTeammateColor
#define OFFSET_STEAMID				0x6B8			//m_steamID
#define OFFSET_MODEL_STATE			0x170			//m_modelstate
#define OFFSET_MODEL_NAME			0xa8			//m_ModelName
#define OFFSET_MONEY_SERVICE		0x6f0			//m_pingamemoneyservices
#define OFFSET_MONEY_ACCOUNT		0x40			//m_iAccount
#define OFFSET_ITEM_SERVICE			0x1100			//m_pItemServices
#define OFFSET_ITEM_HAS_DEFUSER		0x40			//m_bHasDefuser
#define OFFSET_ITEM_HAS_HELMET		0x41			//m_bHasHelmet

//c4
#define OFFSET_C4_BLOW_TIME			0xF00			//m_flC4Blow
#define OFFSET_C4_DEFUSED			0xF24			//m_bBombDefused
#define OFFSET_C4_DEFUSING			0xF0C			//m_bBeingDefused
#define OFFSET_C4_DEFUSE_TIME		0xF20			//m_fldefusecountdown

//keyboard
#define VK_F4                       0x73

//bone
enum BONE: int
{
	head=6,
	neck_0=5,
	spine_1=4,
	spine_2=2,
	pelvis=0,
	arm_upper_L=8,
	arm_lower_L=9,
	hand_L=10,
	arm_upper_R=13,
	arm_lower_R=14,
	hand_R=15,
	leg_upper_L=22,
	leg_lower_L=23,
	ankle_L=24,
	leg_upper_R=25,
	leg_lower_R=26,
	ankle_R=27,
};

enum class e_weapon_type : int
{
	knife,
	pistol,
	submachine_gun,
	rifle,
	shotgun,
	sniper_rifle,
	machine_gun,
	c4,
	taser,
	grenade,
	equipment = 10
};

enum class e_color : int
{
	blue,
	green,
	yellow,
	orange,
	purple,
	white
};
