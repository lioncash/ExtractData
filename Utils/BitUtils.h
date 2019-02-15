#pragma once

namespace BitUtils
{
u16 Swap16(u16 value);
u32 Swap32(u32 value);
u64 Swap64(u64 value);

u32 RotateLeft(u32 value, int shift);
u32 RotateRight(u32 value, int shift);
}
