#pragma once

namespace BitUtils
{

unsigned short Swap16(unsigned short);
unsigned int Swap32(unsigned int);
unsigned long long Swap64(unsigned long long);

unsigned int RotateLeft(unsigned int value, int shift);
unsigned int RotateRight(unsigned int value, int shift);

}
