//credits: https://github.com/frk1/hazedumper/blob/master/csgo.hpp

#include <cstdint>

namespace offset{
     //int format,because there's some error while using player_index/localplayer_ptr to r memory
    
    constexpr ::std::ptrdiff_t m_iHealth = 256;
    constexpr ::std::ptrdiff_t m_iTeamNum = 244;
    constexpr ::std::ptrdiff_t m_iCrosshairId = 71736;

   
    constexpr ::std::ptrdiff_t dwEntityList = 0x4DFFF7C;
    constexpr ::std::ptrdiff_t dwForceAttack = 52616680;
    constexpr ::std::ptrdiff_t dwLocalPlayer = 0xDEA98C;
}
