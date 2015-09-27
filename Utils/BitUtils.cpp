#include <climits>
#include "StdAfx.h"
#include "Utils/BitUtils.h"

namespace BitUtils
{

unsigned short Swap16(unsigned short value)
{
#ifdef _MSC_VER
	return _byteswap_ushort(value);
#elif __linux__
	return __builtin_bswap16(value);
#else
	return (value << 8) | (value >> 8);
#endif
}

unsigned int Swap32(unsigned int value)
{
#ifdef _MSC_VER
	return _byteswap_ulong(value);
#elif __linux__
	return __builtin_bswap32(value);
#else
	return (Swap16(data) << 16) | Swap16(data >> 16);
#endif
}

unsigned long long Swap64(unsigned long long value)
{
#ifdef _MSC_VER
	return _byteswap_uint64(value);
#elif __linux__
	return __builtin_bswap64(value);
#else
	return (static_cast<unsigned long long>(Swap32(value)) << 32) | Swap32(value >> 32);
#endif
}

unsigned int RotateLeft(const unsigned int value, int shift)
{
	shift &= sizeof(value) * CHAR_BIT - 1;

	if (shift == 0)
		return value;

	return (value << shift) | (value >> (sizeof(value) * CHAR_BIT - shift));
}

unsigned int RotateRight(const unsigned int value, int shift)
{
	shift &= sizeof(value) * CHAR_BIT - 1;

	if (shift == 0)
		return value;

	return (value >> shift) | (value << (sizeof(value) * CHAR_BIT - shift));
}

}
