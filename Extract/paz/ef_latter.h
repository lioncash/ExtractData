#pragma once

#include "Extract/paz/HaruotoFD.h"

class CEFLatter final : public CHaruotoFD
{
public:
	bool Mount(CArcFile* archive) override;
	bool Decode(CArcFile* archive) override;

private:
	bool IsSupported(const CArcFile* archive);

	void InitMountKey(CArcFile* archive) override;
	void InitDecodeKey(CArcFile* archive) override;

	u32 InitMovieTable(const u8* table) override;

	void Decrypt(u8* target, size_t size) override;
	void Decrypt2(u8* target, size_t size) override;

	void SetDecryptKey2(CArcFile* archive);

	void DecodeMovieData(u8* target, size_t size) override;

	u8   m_key = 0;

	char m_key_string[256];
	std::array<u8, 256> m_key_table;
	u32  m_key_table_index1 = 0;
	u32  m_key_table_index2 = 0;

	std::array<u8, 65536> m_movie_decode_table;
	u32  m_movie_decode_table_ptr = 0;
};
