#include "stdafx.h"
#include "../../ExtractBase.h"
#include "../../Arc/Zlib.h"
#include "../../Image.h"
#include "../../Sound/Ogg.h"
#include "../../FindFile.h"
#include "Tlg.h"
#include "Krkr.h"

/// Mount
///
/// @param archive Archive
///
bool CKrkr::Mount(CArcFile* archive)
{
	DWORD offset;

	// XP3
	if (memcmp(archive->GetHed(), "XP3\r\n \n\x1A\x8B\x67\x01", 11) == 0)
	{
		// XP3

		offset = 0;
	}
	// EXE type
	else if (memcmp(archive->GetHed(), "MZ", 2) == 0)
	{
		if (!FindXP3FromExecuteFile(archive, &offset))
		{
			return false;
		}
	}
	else
	{
		return false;
	}

	m_archive = archive;

	// Set MD5 value for tpm
	SetMD5ForTpm(archive);

	// Check if archive can be decoded

	// Archive can't be decoded
	if (!OnCheckDecrypt(archive))
	{
		return false;
	}

	// Get index position
	INT64 index_pos;
	archive->SeekHed(11 + offset);
	archive->Read(&index_pos, 8);
	archive->SeekCur(index_pos - 19);

	BYTE work[256];
	archive->Read(work, sizeof(work));

	switch (work[0])
	{
	case 0x80:
		index_pos = *(INT64*)&work[9];
		break;
	}

	// Read the index header
	BYTE cmp_index;

	archive->SeekHed(index_pos + offset);
	archive->Read(&cmp_index, 1);

	UINT64 comp_index_size;
	UINT64 index_size;

	// Index is compressed
	if (cmp_index)
	{
		archive->Read(&comp_index_size, 8);
	}

	archive->Read(&index_size, 8);

	// Ensure buffer
	YCMemory<BYTE> index(index_size);
	DWORD index_ptr = 0;

	// If the index header is compressed, decompress it
	if (cmp_index)
	{
		CZlib zlib;

		// Ensure buffer
		YCMemory<BYTE> comp_index(comp_index_size);

		// zlib Decompression
		archive->Read(&comp_index[0], comp_index_size);
		zlib.Decompress(&index[0], index_size, &comp_index[0], comp_index_size);
	}
	else // Index is not compressed
	{
		archive->Read(&index[0], index_size);
	}

	// Get index file information
	for (UINT64 i = 0; i < index_size;)
	{
		// "File" Chunk
		FileChunk file_chunk;

		memcpy(file_chunk.name, &index[i], 4);

		file_chunk.size = *(UINT64*)&index[i + 4];

		if (memcmp(file_chunk.name, "File", 4) != 0)
		{
			break;
		}

		i += 12;

		// "info" Chunk
		InfoChunk info_chunk;

		memcpy(info_chunk.name, &index[i], 4);

		info_chunk.size = *(UINT64*)&index[i + 4];
		info_chunk.protect = *(DWORD*)&index[i + 12];
		info_chunk.orgSize = *(UINT64*)&index[i + 16];
		info_chunk.arcSize = *(UINT64*)&index[i + 24];
		info_chunk.nameLen = *(WORD*)&index[i + 32];
		info_chunk.filename = (wchar_t*)&index[i + 34];

		if (memcmp(info_chunk.name, "info", 4) != 0)
		{
			break;
		}

		i += 12 + info_chunk.size;

		// "segm" Chunk
		SegmChunk segm_chunk;

		memcpy(segm_chunk.name, &index[i], 4);

		segm_chunk.size = *(UINT64*)&index[i + 4];

		if (memcmp(segm_chunk.name, "segm", 4) != 0)
		{
			break;
		}

		i += 12;

		SFileInfo file_info;

		const UINT64 segm_count = (segm_chunk.size / 28);

		for (UINT64 j = 0; j < segm_count; j++)
		{
			segm_chunk.comp = *(DWORD*)&index[i];
			segm_chunk.start = *(UINT64*)&index[i + 4] + offset;
			segm_chunk.orgSize = *(UINT64*)&index[i + 12];
			segm_chunk.arcSize = *(UINT64*)&index[i + 20];

			file_info.bCmps.push_back(segm_chunk.comp);
			file_info.starts.push_back(segm_chunk.start);
			file_info.sizesOrg.push_back(segm_chunk.orgSize);
			file_info.sizesCmp.push_back(segm_chunk.arcSize);

			i += 28;
		}

		// Check for any other chunks
		const UINT64 remainder = file_chunk.size - 12 - info_chunk.size - 12 - segm_chunk.size;

		if (remainder > 0)
		{
			// "adlr" Chunk
			if (memcmp(&index[i], "adlr", 4) == 0)
			{
				AdlrChunk adlr_chunk;

				memcpy(adlr_chunk.name, &index[i], 4);

				adlr_chunk.size = *(UINT64*)&index[i + 4];
				adlr_chunk.key = *(DWORD*)&index[i + 12];

				file_info.key = adlr_chunk.key;
			}

			i += remainder;
		}

		// Store and show the stucture in a listview
		file_info.name.Copy(info_chunk.filename, info_chunk.nameLen);
		file_info.sizeOrg = info_chunk.orgSize;
		file_info.sizeCmp = info_chunk.arcSize;
		file_info.start = file_info.starts[0];
		file_info.end = file_info.start + file_info.sizeCmp;

		if (segm_chunk.comp)
		{
			file_info.format = _T("zlib");
		}

		archive->AddFileInfo(file_info);
	}

	return true;
}

/// Decode
///
/// @param archive Archive
///
bool CKrkr::Decode(CArcFile* archive)
{
	const SFileInfo* file_info = archive->GetOpenFileInfo();

	if ((archive->GetArcExten() != _T(".xp3")) && (archive->GetArcExten() != _T(".exe")))
		return false;

	YCString file_ext = PathFindExtension(file_info->name);
	file_ext.MakeLower();

	InitDecrypt(archive);

	//char s[256];
	//_stprintf(s, "%08X", pInfFile->key);
	//MessageBox(NULL, s, "", 0);

	// Ensure buffer
	DWORD buffer_size = archive->GetBufSize();
	YCMemory<BYTE> buffer;

	// Whether or not it's bound to memory
	bool compose_memory = false;

	// TLS, OGG (fix CRC), BMP
	if ((file_ext == _T(".tlg")) ||
	    ((file_ext == _T(".ogg")) && archive->GetOpt()->bFixOgg) ||
	    (file_ext == _T(".bmp")))
	{
		buffer.resize(file_info->sizeOrg + 3);
		compose_memory = true;
	}
	// TJS, KS, ASD, TXT
	else if (m_decrypt_key == 0 &&
	   archive->GetOpt()->bEasyDecrypt && (
	   (file_ext == _T(".tjs")) ||
	   (file_ext == _T(".ks")) ||
	   (file_ext == _T(".asd")) ||
	   (file_ext == _T(".txt"))))
	{
		buffer.resize(file_info->sizeOrg + 3);
		compose_memory = true;
	}
	else // Other
	{
		buffer.resize(buffer_size + 3);
	}

	// Create output file
	if (!compose_memory)
	{
		archive->OpenFile();
	}

	size_t buffer_ptr = 0;
	const size_t buffer_size_base = buffer_size;
	size_t total_wrote_size = 0;

	for (size_t i = 0; i < file_info->starts.size(); i++)
	{
		buffer_size = buffer_size_base;

		archive->SeekHed(file_info->starts[i]);

		// Compressed data
		if (file_info->bCmps[i])
		{
			CZlib zlib;

			// Ensure buffer
			const DWORD src_size = file_info->sizesCmp[i];
			YCMemory<u8> src(src_size);

			const DWORD dst_size = file_info->sizesOrg[i];
			YCMemory<u8> dst(dst_size + 3);

			// zlib Decompression
			archive->Read(&src[0], src_size);
			zlib.Decompress(&dst[0], dst_size, &src[0], src_size);

			const size_t data_size = Decrypt(&dst[0], dst_size, total_wrote_size);

			if (compose_memory)
			{
				memcpy(&buffer[buffer_ptr], &dst[0], data_size);

				buffer_ptr += data_size;
			}
			else // Output
			{
				archive->WriteFile(&dst[0], data_size, dst_size);
			}

			total_wrote_size += dst_size;
		}
		else // Uncompressed data
		{
			if (compose_memory)
			{
				// Bound to the buffer

				const size_t dst_size = file_info->sizesOrg[i];
				archive->Read(&buffer[buffer_ptr], dst_size);

				const size_t data_size = Decrypt(&buffer[buffer_ptr], dst_size, total_wrote_size);

				buffer_ptr += data_size;
				total_wrote_size += dst_size;
			}
			else
			{
				const size_t dst_size = file_info->sizesOrg[i];

				for (size_t wrote_size = 0; wrote_size != dst_size; wrote_size += buffer_size)
				{
					// Adjust buffer size
					archive->SetBufSize(&buffer_size, wrote_size, dst_size);
					archive->Read(&buffer[0], buffer_size);

					const size_t data_size = Decrypt(&buffer[0], buffer_size, total_wrote_size);

					archive->WriteFile(&buffer[0], data_size);
					total_wrote_size += buffer_size;
				}
			}
		}
	}
  
	// Convert TLG to BMP
	if (file_ext == _T(".tlg"))
	{
		CTlg tlg;
		tlg.Decode(archive, &buffer[0]);
	}
	// Fix CRC of OGG files
	else if (file_ext == _T(".ogg") && archive->GetOpt()->bFixOgg)
	{
		COgg ogg;
		ogg.Decode(archive, &buffer[0]);
	}
	// BMP output (PNG conversion)
	else if (file_ext == _T(".bmp"))
	{
		CImage image;
		image.Init(archive, &buffer[0]);
		image.Write(file_info->sizeOrg);
	}
	// Text file
	else if (m_decrypt_key == 0 &&
	    archive->GetOpt()->bEasyDecrypt && (
	    (file_ext == _T(".tjs")) ||
	    (file_ext == _T(".ks")) ||
	    (file_ext == _T(".asd")) ||
	    (file_ext == _T(".txt"))))
	{
		const size_t dst_size = file_info->sizeOrg;

		SetDecryptRequirement(true);

		m_decrypt_key = archive->InitDecryptForText(&buffer[0], dst_size);

		const size_t data_size = Decrypt(&buffer[0], dst_size, 0);

		archive->OpenFile();
		archive->WriteFile(&buffer[0], data_size, dst_size);
	}

	return true;
}

/// Extraction
///
/// @param archive Archive
///
bool CKrkr::Extract(CArcFile* archive)
{
	const SFileInfo* file_info = archive->GetOpenFileInfo();

	DWORD buffer_size = archive->GetBufSize();
	const DWORD buffer_size_base = buffer_size;

	YCMemory<BYTE> buffer(buffer_size);

	archive->OpenFile();

	for (size_t i = 0; i < file_info->starts.size(); i++)
	{
		buffer_size = buffer_size_base;

		archive->SeekHed(file_info->starts[i]);

		const DWORD dst_size = file_info->sizesOrg[i];

		for (DWORD wrote_size = 0; wrote_size != dst_size; wrote_size += buffer_size)
		{
			// Adjust buffer size

			archive->SetBufSize(&buffer_size, wrote_size, dst_size);

			archive->Read(&buffer[0], buffer_size);
			archive->WriteFile(&buffer[0], buffer_size);
		}
	}

	archive->CloseFile();

	return true;
}

/// Set MD5 value for tpm in the archive folder
///
/// @param archive Archive
///
void CKrkr::SetMD5ForTpm(CArcFile* archive)
{
	if (archive->CheckMD5OfSet())
	{
		// If it has already been set
		return;
	}

	// Get directory path to the archive
	TCHAR base_path_to_tpm[MAX_PATH];

	lstrcpy(base_path_to_tpm, archive->GetArcPath());
	PathRemoveFileSpec(base_path_to_tpm);

	// Get the tpm file path
	CFindFile find_file;
	std::vector<YCString> tpm_paths = find_file.DoFind(base_path_to_tpm, _T("*.tpm"));

	// Set the tpm MD5 value
	CMD5 md5;

	for (const auto& path : tpm_paths)
	{
		archive->SetMD5(md5.Calculate(path));
	}

	archive->SetMD5OfFlag(true);
}

/// Determine if decryption is possible
///
/// @param archive Archive
///
bool CKrkr::OnCheckDecrypt(CArcFile* archive)
{
	return true;
}

/// Verifies that the MD5 value of tpm in the archive folder matches
///
/// @param md5 MD5
///
bool CKrkr::CheckTpm(const char* md5) const
{
	// Comparison
	for (size_t i = 0; i < m_archive->GetMD5().size(); i++)
	{
		if (memcmp(md5, m_archive->GetMD5()[i].szABCD, 32) == 0)
		{
			// Matches
			return true;
		}
	}

	return false;
}

/// Initialization of the decryption process
///
/// @param archive Archive
///
void CKrkr::InitDecrypt(CArcFile* archive)
{
	m_archive = archive;

	// Enable decryption request
	SetDecryptRequirement(true);

	// Set decryption size
	SetDecryptSize(0);

	// Call the initialization function that has been overwritten
	m_decrypt_key = OnInitDecrypt(archive);
}

/// By default, use simple decoding
///
/// @param archive Archive
///
u32 CKrkr::OnInitDecrypt(CArcFile* archive)
{
	const u32 decrypt_key = archive->InitDecrypt();

	// Unencrypted
	if (decrypt_key == 0)
	{
		SetDecryptRequirement(false);
	}

	return decrypt_key;
}

/// Decryption Process
///
/// @param target      Data to be decoded
/// @param target_size Decoding size
/// @param offset      Offset of data to be decoded
///
size_t CKrkr::Decrypt(u8* target, size_t target_size, size_t offset)
{
	// No decryption requests
	if (!m_decrypt)
		return target_size;

	size_t decrypt_size = m_decrypt_size;

	// Decoding size has not been set
	if (decrypt_size == 0)
		return OnDecrypt(target, target_size, offset, m_decrypt_key);

	// Don't decode anymore
	if (offset >= decrypt_size)
		return target_size;

	// Size is larger than the predetermined decryption data size
	if (decrypt_size > target_size)
		decrypt_size = target_size;

	OnDecrypt(target, decrypt_size, offset, m_decrypt_key);

	return target_size;
}

/// By default, use simple decoding
///
/// Remark: The decrypt_key returns the value from OnInitDecrypt
///
/// @param target      Data to be decoded
/// @param target_size Data size
/// @param offset      Location of data to be decoded
/// @param decrypt_key Decryption key
///
size_t CKrkr::OnDecrypt(u8* target, size_t target_size, size_t offset, u32 decrypt_key)
{
	m_archive->Decrypt(target, target_size);

	return target_size;
}

/// Set decryption request
///
/// @param decrypt Decryption request
///
void CKrkr::SetDecryptRequirement(bool decrypt)
{
	m_decrypt = decrypt;
}

/// Set decoding size
///
/// @param decrypt_size - Decoding size
///
void CKrkr::SetDecryptSize(size_t decrypt_size)
{
	m_decrypt_size = decrypt_size;
}

/// Get the offset for the location of the archive within an EXE file.
///
/// Remark: KiriKiri allows its resources to be stored within an executable
///
/// @param archive Archive
/// @param offset  Offset within archive
///
bool CKrkr::FindXP3FromExecuteFile(CArcFile* archive, DWORD* offset)
{
	// Is not a kirikiri executable
	if (archive->GetArcSize() <= 0x200000)
	{
		return false;
	}

	*offset = 16;

	archive->SeekHed(16);

	BYTE buffer[4096];
	DWORD read_size;

	do
	{
		read_size = archive->Read(buffer, sizeof(buffer));

		for (DWORD i = 0, j = 0; i < (read_size / 16); i++, j += 16)
		{
			// Found XP3 archive
			if (memcmp(&buffer[j], "XP3\r\n \n\x1A\x8B\x67\x01", 11) == 0)
			{
				*offset += j;
				return true;
			}
		}

		*offset += read_size;

		if (*offset >= 0x500000)
		{
			// Truncate search
			break;
		}

		// If canceled
		if (archive->GetProg()->OnCancel())
		{
			throw - 1;
		}
	} while (read_size == sizeof(buffer));

	// No XP3 archive
	*offset = 0;
	archive->SeekHed();

	return false;
}
