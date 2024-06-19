#pragma once
#include "opencv2/opencv.hpp"
#include "pch.h"

class c_keys
{
private:
	uint64_t gafAsyncKeyStateExport = 0;
	uint8_t state_bitmap[64] { };
	uint8_t previous_state_bitmap[256 / 8] { };
	uint64_t win32kbase = 0;

	int win_logon_pid = 0;

	std::chrono::time_point<std::chrono::system_clock> start = std::chrono::system_clock::now();

public:
	c_keys() = default;

	~c_keys() = default;

	bool InitKeyboard();

	void UpdateKeys();
	bool IsKeyDown(uint32_t virtual_key_code);
};

typedef struct moves{
	int x = 0;
	int y = 0;
};

class QMP {
 public:
  	QMP();

  	bool Connect(std::string_view address, uint32_t port);

  	void Disconnect();

  	bool EnableCommands() const;

  	bool MoveMouse(int32_t delta_x, int32_t delta_y) const;

	void SmoothMove(int x, int y);

	bool InitMouseAI();

 private:
  	bool connected_;
  	int socket_;
	std::mutex m;

	std::vector<moves> getSmoothPoints(moves start, moves control, moves end);

	void moveto(moves to);

	//ai
	cv::dnn::Net net;

	std::vector<moves> AIForward(int x, int y);

  	bool Send(std::string_view message) const;
};
