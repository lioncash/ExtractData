#pragma once

#include "HaruotoFD.h"

class Cef_first final : public CHaruotoFD
{
public:
	BOOL Mount(CArcFile* pclArc) override;
	BOOL Decode(CArcFile* pclArc) override;

private:
	bool IsSupported(CArcFile* pclArc);

	void InitMountKey(CArcFile* pclArc) override;
	void InitDecodeKey(CArcFile* pclArc) override;

	DWORD InitMovieTable(void* pvTable) override;

	void DecodeTable1() override;

	void Decrypt(void* pvTarget, DWORD dwSize) override;

	void DecodeMovieData(void* pvTarget, DWORD dwSize) override;

	DWORD GetMovieBufSize(CArcFile* pclArc) override;

	BYTE  m_btKey;
	BYTE  m_aabtMovieTable[256][256];
	DWORD m_dwMovieTableID;
};
