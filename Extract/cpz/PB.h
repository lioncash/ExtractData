#pragma once

class CPB
{
public:
	virtual ~CPB() = default;

	void DecompLZSS(void* pvDst, DWORD dwDstSize, const void* pvFlags, DWORD dwFlagsSize, const void* pvSrc, DWORD dwSrcSize);
};
