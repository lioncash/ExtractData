#pragma once

#include "HaruotoFD.h"

class CEFLatter final : public CHaruotoFD
{
public:
	bool Mount(CArcFile* archive) override;
	bool Decode(CArcFile* archive) override;

private:
	bool IsSupported(CArcFile* archive);

	void InitMountKey(CArcFile* archive) override;
	void InitDecodeKey(CArcFile* archive) override;

	DWORD InitMovieTable(void* table) override;

	void Decrypt(void* target, DWORD size) override;
	void Decrypt2(void * target, DWORD size) override;

	void SetDecryptKey2(CArcFile* archive);

	void DecodeMovieData(void* target, DWORD size) override;

	BYTE  m_key;

	char  m_key_string[256];
	BYTE  m_key_table[256];
	DWORD m_key_table_index1;
	DWORD m_key_table_index2;

	BYTE  m_movie_decode_table[65536];
	DWORD m_movie_decode_table_ptr;
};
