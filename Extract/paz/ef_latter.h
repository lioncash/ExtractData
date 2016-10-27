#pragma once

#include "HaruotoFD.h"

class CEFLatter final : public CHaruotoFD
{
public:
	bool Mount(CArcFile* pclArc) override;
	bool Decode(CArcFile* pclArc) override;

private:
	bool IsSupported(CArcFile* pclArc);

	void InitMountKey(CArcFile* pclArc) override;
	void InitDecodeKey(CArcFile* pclArc) override;

	DWORD InitMovieTable(void* pvTable) override;

	void Decrypt(void* pvTarget, DWORD dwSize) override;
	void Decrypt2(void * pvTarget, DWORD dwSize) override;

	void SetDecryptKey2(CArcFile* pclArc);

	void DecodeMovieData(void* pvTarget, DWORD dwSize) override;

	BYTE  m_btKey;

	char  m_szKey[256];
	BYTE  m_abtKeyTable[256];
	DWORD m_dwKeyTableIndex1;
	DWORD m_dwKeyTableIndex2;

	BYTE  m_abtMovieDecodeTable[65536];
	DWORD m_dwMovieDecodeTablePtr;
};
