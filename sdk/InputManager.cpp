#include "pch.h"
#include "InputManager.h"
#include "Memory.h"

//TODO: Restart winlogon.exe when it doesn't exist.
bool c_keys::InitKeyboard()
{
	int Winver = 23000;
	win_logon_pid = mem.GetPidFromName("winlogon.exe");

	if (Winver > 22000)
	{
		auto pids = mem.GetPidListFromName("csrss.exe");
		for (size_t i = 0; i < pids.size(); i++)
		{
			auto pid = pids[i];
			uintptr_t tmp = VMMDLL_ProcessGetModuleBaseU(mem.vHandle, pid, const_cast<LPSTR>("win32ksgd.sys"));
			uintptr_t g_session_global_slots = tmp + 0x3110;
			uintptr_t user_session_state = mem.ReadPID<uintptr_t>(mem.ReadPID<uintptr_t>(mem.ReadPID<uintptr_t>(g_session_global_slots, pid), pid), pid);
			gafAsyncKeyStateExport = user_session_state + 0x3690;
			if (gafAsyncKeyStateExport > 0x7FFFFFFFFFFF)
				break;
		}
		if (gafAsyncKeyStateExport > 0x7FFFFFFFFFFF)
			return true;
		return false;
	}
	else
	{
		PVMMDLL_MAP_EAT eat_map = NULL;
		PVMMDLL_MAP_EATENTRY eat_map_entry;
		bool result = VMMDLL_Map_GetEATU(mem.vHandle, mem.GetPidFromName("winlogon.exe") | VMMDLL_PID_PROCESS_WITH_KERNELMEMORY, const_cast<LPSTR>("win32kbase.sys"), &eat_map);
		if (!result)
			return false;

		if (eat_map->dwVersion != VMMDLL_MAP_EAT_VERSION)
		{
			VMMDLL_MemFree(eat_map);
			eat_map_entry = NULL;
			return false;
		}

		for (int i = 0; i < eat_map->cMap; i++)
		{
			eat_map_entry = eat_map->pMap + i;
			if (strcmp(eat_map_entry->uszFunction, "gafAsyncKeyState") == 0)
			{
				gafAsyncKeyStateExport = eat_map_entry->vaFunction;

				break;
			}
		}

		VMMDLL_MemFree(eat_map);
		eat_map = NULL;
		if (gafAsyncKeyStateExport < 0x7FFFFFFFFFFF)
		{
			PVMMDLL_MAP_MODULEENTRY module_info;
			auto result = VMMDLL_Map_GetModuleFromNameU(mem.vHandle, mem.GetPidFromName("winlogon.exe") | VMMDLL_PID_PROCESS_WITH_KERNELMEMORY, const_cast<LPSTR>("win32kbase.sys"), &module_info, VMMDLL_MODULE_FLAG_NORMAL);
			if (!result)
			{
				LOG("failed to get module info\n");
				return false;
			}

			char str[32];
			if (!VMMDLL_PdbLoad(mem.vHandle, mem.GetPidFromName("winlogon.exe") | VMMDLL_PID_PROCESS_WITH_KERNELMEMORY, module_info->vaBase, str))
			{
				LOG("failed to load pdb\n");
				return false;
			}

			ULONG64 gafAsyncKeyState;
			if (!VMMDLL_PdbSymbolAddress(mem.vHandle, str, const_cast<LPSTR>("gafAsyncKeyState"), &gafAsyncKeyState))
			{
				LOG("failed to find gafAsyncKeyState\n");
				return false;
			}
			LOG("found gafAsyncKeyState at: 0x%p\n", gafAsyncKeyState);
		}
		if (gafAsyncKeyStateExport > 0x7FFFFFFFFFFF)
			return true;
		return false;
	}
}

void c_keys::UpdateKeys()
{
	uint8_t previous_key_state_bitmap[64] = {0};
	memcpy(previous_key_state_bitmap, state_bitmap, 64);

	VMMDLL_MemReadEx(mem.vHandle, this->win_logon_pid | VMMDLL_PID_PROCESS_WITH_KERNELMEMORY, gafAsyncKeyStateExport, reinterpret_cast<PBYTE>(&state_bitmap), 64, NULL, VMMDLL_FLAG_NOCACHE);
	for (int vk = 0; vk < 256; ++vk)
		if ((state_bitmap[(vk * 2 / 8)] & 1 << vk % 4 * 2) && !(previous_key_state_bitmap[(vk * 2 / 8)] & 1 << vk % 4 * 2))
			previous_state_bitmap[vk / 8] |= 1 << vk % 8;
}

bool c_keys::IsKeyDown(uint32_t virtual_key_code)
{
	if (gafAsyncKeyStateExport < 0x7FFFFFFFFFFF)
		return false;
	if (std::chrono::system_clock::now() - start > std::chrono::milliseconds(1))
	{
		UpdateKeys();
		start = std::chrono::system_clock::now();
	}
	return state_bitmap[(virtual_key_code * 2 / 8)] & 1 << virtual_key_code % 4 * 2;
}

QMP::QMP()
    : connected_(false),
      socket_(0) {}
 
bool QMP::Connect(std::string_view address, uint32_t port) {
  if (connected_) {
    printf("[QMP] Already Connected");
    return true;
  }
 
  socket_ = socket(AF_INET, SOCK_STREAM, 0);
  if (socket_ == -1) {
    printf("[QMP] Cannot creat socket descriptor\n");
    return false;
  }
 
  struct sockaddr_in socket_address{};
  socket_address.sin_family = AF_INET;
  socket_address.sin_port = htons(port);
  socket_address.sin_addr.s_addr = inet_addr(address.data());
 
  // Connect to the socket.
  auto result = connect(
      socket_,
      reinterpret_cast<const sockaddr *>(&socket_address),
      sizeof(socket_address));
  if (result == -1) {
    printf("[QMP] Cannot connect to %s\n", address);
    close(socket_);
    return false;
  }
 
  connected_ = true;
  return true;
}
 
void QMP::Disconnect() {
  if (!connected_) {
    return;
  }
 
  close(socket_);
  connected_ = false;
}
 
bool QMP::EnableCommands() const {
  if (!connected_) {
    return false;
  }

  std::string_view message{R"({ "execute": "qmp_capabilities" })"};
  return Send(message);
}

bool QMP::MoveMouse(int32_t delta_x, int32_t delta_y) const {
  if (!connected_) {
    return false;
  }

  std::string message{
    "{\n"
    "  \"execute\": \"input-send-event\",\n"
    "  \"arguments\": {\n"
    "    \"events\": [\n"
    "      {\n"
    "        \"type\": \"rel\",\n"
    "        \"data\": {\n"
    "          \"axis\": \"x\",\n"
    "          \"value\": " + std::to_string(delta_x) + "\n"
    "        }\n"
    "      },\n"
    "      {\n"
    "        \"type\": \"rel\",\n"
    "        \"data\": {\n"
    "          \"axis\": \"y\",\n"
    "          \"value\": " + std::to_string(delta_y) + "\n"
    "        }\n"
    "      }\n"
    "    ]\n"
    "  }\n"
    "}"
  };

  return Send(message);
}

/**
 * out put points list (relative pos)
*/
void QMP::moveto(moves to){
	if( abs(to.x) > 1 || abs(to.y) > 1){
		while(true){
			moves point_to_move = { 0 , 0 };
			std::this_thread::sleep_for(std::chrono::milliseconds(1));

			if(to.x > 1){
				to.x -= 1;
				point_to_move.x = 1;
			}
			if(to.y > 1){
				to.y -= 1;
				point_to_move.y = 1;
			}
			if(to.x < 1){
				to.x += 1;
				point_to_move.x = -1;
			}
			if(to.y < 1){
				to.y += 1;
				point_to_move.y = -1;
			}

			MoveMouse(point_to_move.x, point_to_move.y);

			if( abs(to.x) == 1 && abs(to.y) == 1){
				break;
			}
		}
	}else{
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
		MoveMouse(to.x, to.y);
	}
}

void QMP::SmoothMove(int x, int y){
	std::lock_guard<std::mutex> l(m);

	std::vector<moves> mouse_moves = AIForward(x, y);

	int prev_x = 0, prev_y = 0;

	for(moves point_to_move: mouse_moves){
		moveto({point_to_move.x - prev_x, point_to_move.y - prev_y});
		prev_x = point_to_move.x;
		prev_y = point_to_move.y;
	}
}

//ai
bool QMP::InitMouseAI()
{
	net = cv::dnn::readNetFromONNX("mouse.onnx");
    if (!net.empty())
    {
		return true;
    }
    return false;
}

std::vector<moves> QMP::AIForward(int x, int y)
{
	std::vector<moves> result;
	if (!net.empty())
    {
		cv::Mat matblob = (cv::Mat_<float>(1, 2) << x, y);
        net.setInput(matblob, "input");
        cv::Mat output = net.forward("output");//输出1*20*2

        for(int i = 0; i < 20; i++){
		result.push_back({(int)output.at<float>(0,i,0), (int)output.at<float>(0,i,1)});
	}
    }
    return result;
}

bool QMP::Send(std::string_view message) const
{
    size_t sent = send(socket_, message.data(), message.size(), 0);
    return sent == message.size();
}
