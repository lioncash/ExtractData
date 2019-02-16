#pragma once

#include "ExtractBase.h"

class CAxl final : public CExtractBase
{
public:
	bool Mount(CArcFile* archive) override;
	bool Decode(CArcFile* archive) override;

private:
	void InitMountKey(const void* decryption_key);
	bool CreateKey(u8* key, size_t* key_length, const u8* index, size_t index_size);
	bool DecryptIndex(u8* index, size_t index_size, u64 archive_size);

	std::array<u8, 32> m_decryption_key{};
	size_t m_length = 0;
};
