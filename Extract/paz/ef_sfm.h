#pragma once

#include "HaruotoFD.h"

//----------------------------------------------------------------------------------------
//-- ef - Second Fan Mix -----------------------------------------------------------------
//----------------------------------------------------------------------------------------

class CEFsfm final : public CHaruotoFD
{
public:
	bool Mount(CArcFile* pclArc) override;
	bool Decode(CArcFile* pclArc) override;

private:
	bool IsSupported(CArcFile* pclArc) const;

	void InitMountKey(CArcFile* pclArc) override;
	void InitDecodeKey(CArcFile* pclArc) override;

	DWORD InitMovieTable(void* pvTable) override;

	void DecodeMovieData(void* pvTarget, DWORD dwSize) override;

	DWORD GetMovieBufSize(CArcFile* pclArc) override;

	BYTE  m_btKey;
	BYTE  m_aabtMovieTable[256][256];
	DWORD m_dwMovieTableID;
};
