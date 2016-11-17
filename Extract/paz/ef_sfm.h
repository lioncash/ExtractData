#pragma once

#include "HaruotoFD.h"

//----------------------------------------------------------------------------------------
//-- ef - Second Fan Mix -----------------------------------------------------------------
//----------------------------------------------------------------------------------------

class CEFsfm final : public CHaruotoFD
{
public:
	bool Mount(CArcFile* archive) override;
	bool Decode(CArcFile* archive) override;

private:
	bool IsSupported(CArcFile* archive) const;

	void InitMountKey(CArcFile* archive) override;
	void InitDecodeKey(CArcFile* archive) override;

	DWORD InitMovieTable(void* table) override;

	void DecodeMovieData(void* target, DWORD size) override;

	DWORD GetMovieBufSize(CArcFile* archive) override;

	BYTE  m_key;
	BYTE  m_movie_table[256][256];
	DWORD m_movie_table_id;
};
