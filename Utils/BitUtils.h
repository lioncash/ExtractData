#pragma once

namespace BitUtils
{

unsigned short Swap16(unsigned short value);
unsigned int Swap32(unsigned int value);
unsigned long long Swap64(unsigned long long value);

unsigned int RotateLeft(unsigned int value, int shift);
unsigned int RotateRight(unsigned int value, int shift);

}
