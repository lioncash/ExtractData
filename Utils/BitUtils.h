#pragma once

namespace BitUtils
{
u16 Swap16(u16);
u32 Swap32(u32);
u64 Swap64(u64);

u32 RotateLeft(u32 value, int shift);
u32 RotateRight(u32 value, int shift);
}
