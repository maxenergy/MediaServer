﻿#ifndef CRC32_H_
#define CRC32_H_

#include <string>
#include <cstdint>

class CRC32
{
public:
    static uint32_t encode(const unsigned char *data, int length);
};

#endif