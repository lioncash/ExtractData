#pragma once

class CPB
{
public:
	virtual ~CPB() = default;

	void DecompLZSS(u8* dst, DWORD dst_size, const u8* flags, DWORD flags_size, const u8* src, DWORD src_size);
};
