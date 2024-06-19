#ifndef PCH_H
#define PCH_H

// add headers that you want to pre-compile here
#include <cstdio>
#include <iostream>
#include <sstream>
#include <fstream>
#include <filesystem>
#include <unistd.h>
#include <cstring>
#include <string.h>
#include <thread>
#include <vector>
#include <mutex>
#include <chrono>
#include <algorithm>

#include <math.h>
#include <stdlib.h>

#include "../includes/json.hpp"
#include "../includes/vmmdll.h"
#include "../includes/easywsclient.hpp"

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/ioctl.h>


// credits tni & learn_more (www.unknowncheats.me/forum/3868338-post34.html)
#define INRANGE(x,a,b)		(x >= a && x <= b) 
#define getBits( x )		(INRANGE(x,'0','9') ? (x - '0') : ((x&(~0x20)) - 'A' + 0xa))
#define get_byte( x )		(getBits(x[0]) << 4 | getBits(x[1]))

template <typename Ty>
std::vector<Ty> bytes_to_vec(const std::string& bytes)
{
    const auto num_bytes = bytes.size() / 3;
    const auto num_elements = num_bytes / sizeof(Ty);

    std::vector<Ty> vec;
    vec.resize(num_elements + 1);

    const char* p1 = bytes.c_str();
    uint8_t* p2 = reinterpret_cast<uint8_t*>(vec.data());
    while (*p1 != '\0')
    {
        if (*p1 == ' ')
        {
            ++p1;
        }
        else
        {
            *p2++ = get_byte(p1);
            p1 += 2;
        }
    }

    return vec;
}

#define DEBUG_INFO
#ifdef DEBUG_INFO
#define LOG(fmt, ...) std::printf(fmt, ##__VA_ARGS__)
#define LOGW(fmt, ...) std::wprintf(fmt, ##__VA_ARGS__)
#else
#define LOG
#define LOGW
#endif

#define THROW_EXCEPTION
#ifdef THROW_EXCEPTION
#define THROW(fmt, ...) throw std::runtime_error(fmt, ##__VA_ARGS__)
#endif

#endif //PCH_H