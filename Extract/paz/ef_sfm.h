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
	bool IsSupported(const CArcFile* archive) const;

	void InitMountKey(CArcFile* archive) override;
	void InitDecodeKey(CArcFile* archive) override;

	u32 InitMovieTable(const u8* table) override;

	void DecodeMovieData(u8* target, size_t size) override;

	size_t GetMovieBufSize(CArcFile* archive) override;

	u8  m_key = 0;
	u8  m_movie_table[256][256];
	u32 m_movie_table_id = 0;
};
