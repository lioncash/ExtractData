#pragma once

#include "../../ExtractBase.h"

class CCpz final : public CExtractBase
{
public:
	bool Mount(CArcFile* archive) override;
	bool Decode(CArcFile* archive) override;

private:
	struct SCPZ5Header
	{
		char  szIdentifier[4];            // Identifier
		DWORD dwDirs;                     // Number of directories
		DWORD dwTotalDirIndexSize;        // Total size of the directory index
		DWORD dwTotalFileIndexSize;       // Total size of the file index
		DWORD adwUnknown1[4];             // Unknown
		DWORD adwMD5[4];                  // MD5
		DWORD dwIndexKey;                 // Index Key
		DWORD adwUnKnown2[3];             // Unknown
	};

	bool MountCpz1(CArcFile* archive);
	bool MountCpz2(CArcFile* archive);
	bool MountCpz3(CArcFile* archive);
	bool MountCpz5(CArcFile* archive);

	bool DecodeCpz1(CArcFile* archive);
	bool DecodeCpz2(CArcFile* archive);
	bool DecodeCpz3(CArcFile* archive);
	bool DecodeCpz5(CArcFile* archive);

	static void Decrypt1(BYTE* target, DWORD size);
	static void Decrypt2(BYTE* target, DWORD size, DWORD key);
	static void Decrypt3(BYTE* target, DWORD size, DWORD key);
	static void Decrypt5(BYTE* target, DWORD size, DWORD key);

	static const BYTE* InitDecryptWithTable5(DWORD key, DWORD seed);
	static void DecryptWithTable5(BYTE* target, DWORD size, const BYTE* decryption_table, DWORD key);
	static void DecryptOfData5(BYTE* target, DWORD size, const BYTE* decryption_table, const DWORD* pdwKey, DWORD seed);

	// Callback function from PB3B
	static void OnDecrypt3FromPB3B(BYTE* target, DWORD target_size, CArcFile* archive, const SFileInfo& file_info);
	static void OnDecrypt5FromPB3B(BYTE* target, DWORD target_size, CArcFile* archive, const SFileInfo& file_info);
};
