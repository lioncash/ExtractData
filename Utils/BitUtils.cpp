#include <climits>
#include "StdAfx.h"
#include "Utils/BitUtils.h"

namespace BitUtils
{

unsigned int RotateLeft(const unsigned int value, int shift)
{
	if ((shift &= sizeof(value) * CHAR_BIT - 1) == 0)
		return value;

	return (value << shift) | (value >> (sizeof(value) * CHAR_BIT - shift));
}

unsigned int RotateRight(const unsigned int value, int shift)
{
	if ((shift &= sizeof(value) * CHAR_BIT - 1) == 0)
		return value;

	return (value >> shift) | (value << (sizeof(value) * CHAR_BIT - shift));
}

}
