#pragma once

class CPB
{
public:
	virtual ~CPB() = default;

	void DecompLZSS(u8* dst, size_t dst_size, const u8* flags, size_t flags_size, const u8* src, size_t src_size);
};
