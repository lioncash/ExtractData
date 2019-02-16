#include "StdAfx.h"
#include "Extract/krkr/Krkr.h"

#include "ArcFile.h"
#include "Arc/Zlib.h"
#include "Extract/krkr/Tlg.h"
#include "FindFile.h"
#include "Image.h"
#include "Sound/Ogg.h"

namespace
{
bool IsXP3File(const CArcFile* archive)
{
	return memcmp(archive->GetHeader(), "XP3\r\n \n\x1A\x8B\x67\x01", 11) == 0;
}

bool IsEXEFile(const CArcFile* archive)
{
	return memcmp(archive->GetHeader(), "MZ", 2) == 0;
}
} // Anonymous namespace

/// Mount
///
/// @param archive Archive
///
bool CKrkr::Mount(CArcFile* archive)
{
	size_t offset;

	if (IsXP3File(archive))
	{
		offset = 0;
	}
	else if (IsEXEFile(archive))
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
	s64 index_pos;
	archive->SeekHed(11 + offset);
	archive->ReadS64(&index_pos);
	archive->SeekCur(index_pos - 19);

	std::array<u8, 256> work;
	archive->Read(work.data(), work.size());

	switch (work[0])
	{
	case 0x80:
		index_pos = *reinterpret_cast<const s64*>(&work[9]);
		break;
	}

	// Read the index header
	u8 cmp_index;
	archive->SeekHed(index_pos + offset);
	archive->ReadU8(&cmp_index);

	u64 comp_index_size = 0;
	u64 index_size = 0;

	// Index is compressed
	if (cmp_index)
	{
		archive->ReadU64(&comp_index_size);
	}

	archive->ReadU64(&index_size);

	// Ensure buffer
	std::vector<u8> index(index_size);

	// If the index header is compressed, decompress it
	if (cmp_index)
	{
		CZlib zlib;

		// Ensure buffer
		std::vector<u8> comp_index(comp_index_size);

		// zlib Decompression
		archive->Read(comp_index.data(), comp_index.size());
		zlib.Decompress(index.data(), index.size(), comp_index.data(), comp_index.size());
	}
	else // Index is not compressed
	{
		archive->Read(index.data(), index.size());
	}

	// Get index file information
	for (u64 i = 0; i < index_size;)
	{
		// "File" Chunk
		FileChunk file_chunk;

		memcpy(file_chunk.name, &index[i], 4);

		file_chunk.size = *reinterpret_cast<const u64*>(&index[i + 4]);

		if (memcmp(file_chunk.name, "File", 4) != 0)
		{
			break;
		}

		i += 12;

		// "info" Chunk
		InfoChunk info_chunk;

		memcpy(info_chunk.name, &index[i], 4);

		info_chunk.size     = *reinterpret_cast<const u64*>(&index[i + 4]);
		info_chunk.protect  = *reinterpret_cast<const u32*>(&index[i + 12]);
		info_chunk.orgSize  = *reinterpret_cast<const u64*>(&index[i + 16]);
		info_chunk.arcSize  = *reinterpret_cast<const u64*>(&index[i + 24]);
		info_chunk.nameLen  = *reinterpret_cast<const u16*>(&index[i + 32]);
		info_chunk.filename = reinterpret_cast<wchar_t*>(&index[i + 34]);

		if (memcmp(info_chunk.name, "info", 4) != 0)
		{
			break;
		}

		i += 12 + info_chunk.size;

		// "segm" Chunk
		SegmChunk segm_chunk = {};

		memcpy(segm_chunk.name, &index[i], 4);

		segm_chunk.size = *reinterpret_cast<const u64*>(&index[i + 4]);

		if (memcmp(segm_chunk.name, "segm", 4) != 0)
		{
			break;
		}

		i += 12;

		SFileInfo file_info;

		const u64 segm_count = segm_chunk.size / 28;

		for (u64 j = 0; j < segm_count; j++)
		{
			segm_chunk.comp    = *reinterpret_cast<const u32*>(&index[i]);
			segm_chunk.start   = *reinterpret_cast<const u64*>(&index[i + 4]) + offset;
			segm_chunk.orgSize = *reinterpret_cast<const u64*>(&index[i + 12]);
			segm_chunk.arcSize = *reinterpret_cast<const u64*>(&index[i + 20]);

			file_info.compress_checks.push_back(segm_chunk.comp);
			file_info.starts.push_back(segm_chunk.start);
			file_info.sizes_org.push_back(segm_chunk.orgSize);
			file_info.sizes_cmp.push_back(segm_chunk.arcSize);

			i += 28;
		}

		// Check for any other chunks
		const u64 remainder = file_chunk.size - 12 - info_chunk.size - 12 - segm_chunk.size;

		if (remainder > 0)
		{
			// "adlr" Chunk
			if (memcmp(&index[i], "adlr", 4) == 0)
			{
				AdlrChunk adlr_chunk;

				memcpy(adlr_chunk.name, &index[i], 4);

				adlr_chunk.size = *reinterpret_cast<const u64*>(&index[i + 4]);
				adlr_chunk.key = *reinterpret_cast<const u32*>(&index[i + 12]);

				file_info.key = adlr_chunk.key;
			}

			i += remainder;
		}

		// Store and show the stucture in a listview
		file_info.name.Copy(info_chunk.filename, info_chunk.nameLen);
		file_info.size_org = info_chunk.orgSize;
		file_info.size_cmp = info_chunk.arcSize;
		file_info.start = file_info.starts[0];
		file_info.end = file_info.start + file_info.size_cmp;

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

	if (archive->GetArcExten() != _T(".xp3") && archive->GetArcExten() != _T(".exe"))
		return false;

	YCString file_ext = PathFindExtension(file_info->name);
	file_ext.MakeLower();

	InitDecrypt(archive);

	// Ensure buffer
	size_t buffer_size = archive->GetBufSize();
	std::vector<u8> buffer;

	// Whether or not it's bound to memory
	bool compose_memory = false;

	// TLS, OGG (fix CRC), BMP
	if ((file_ext == _T(".tlg")) ||
	    ((file_ext == _T(".ogg")) && archive->GetOpt()->bFixOgg) ||
	    (file_ext == _T(".bmp")))
	{
		buffer.resize(file_info->size_org + 3);
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
		buffer.resize(file_info->size_org + 3);
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
		if (file_info->compress_checks[i])
		{
			CZlib zlib;

			// Ensure buffer
			const size_t src_size = file_info->sizes_cmp[i];
			std::vector<u8> src(src_size);

			const size_t dst_size = file_info->sizes_org[i];
			std::vector<u8> dst(dst_size + 3);

			// zlib Decompression
			archive->Read(src.data(), src_size);
			zlib.Decompress(dst.data(), dst_size, src.data(), src_size);

			const size_t data_size = Decrypt(dst.data(), dst_size, total_wrote_size);

			if (compose_memory)
			{
				memcpy(&buffer[buffer_ptr], dst.data(), data_size);

				buffer_ptr += data_size;
			}
			else // Output
			{
				archive->WriteFile(dst.data(), data_size, dst_size);
			}

			total_wrote_size += dst_size;
		}
		else // Uncompressed data
		{
			if (compose_memory)
			{
				// Bound to the buffer

				const size_t dst_size = file_info->sizes_org[i];
				archive->Read(&buffer[buffer_ptr], dst_size);

				const size_t data_size = Decrypt(&buffer[buffer_ptr], dst_size, total_wrote_size);

				buffer_ptr += data_size;
				total_wrote_size += dst_size;
			}
			else
			{
				const size_t dst_size = file_info->sizes_org[i];

				for (size_t wrote_size = 0; wrote_size != dst_size; wrote_size += buffer_size)
				{
					// Adjust buffer size
					archive->SetBufSize(&buffer_size, wrote_size, dst_size);
					archive->Read(buffer.data(), buffer_size);

					const size_t data_size = Decrypt(buffer.data(), buffer_size, total_wrote_size);

					archive->WriteFile(buffer.data(), data_size);
					total_wrote_size += buffer_size;
				}
			}
		}
	}
  
	// Convert TLG to BMP
	if (file_ext == _T(".tlg"))
	{
		CTlg tlg;
		tlg.Decode(archive, buffer.data());
	}
	// Fix CRC of OGG files
	else if (file_ext == _T(".ogg") && archive->GetOpt()->bFixOgg)
	{
		COgg ogg;
		ogg.Decode(archive, buffer.data());
	}
	// BMP output (PNG conversion)
	else if (file_ext == _T(".bmp"))
	{
		CImage image;
		image.Init(archive, buffer.data());
		image.Write(file_info->size_org);
	}
	// Text file
	else if (m_decrypt_key == 0 &&
	    archive->GetOpt()->bEasyDecrypt && (
	    (file_ext == _T(".tjs")) ||
	    (file_ext == _T(".ks")) ||
	    (file_ext == _T(".asd")) ||
	    (file_ext == _T(".txt"))))
	{
		const size_t dst_size = file_info->size_org;

		SetDecryptRequirement(true);

		m_decrypt_key = archive->InitDecryptForText(buffer.data(), dst_size);

		const size_t data_size = Decrypt(buffer.data(), dst_size, 0);

		archive->OpenFile();
		archive->WriteFile(buffer.data(), data_size, dst_size);
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

	const size_t buffer_size_base = archive->GetBufSize();
	size_t buffer_size = buffer_size_base;

	std::vector<u8> buffer(buffer_size);

	archive->OpenFile();

	for (size_t i = 0; i < file_info->starts.size(); i++)
	{
		buffer_size = buffer_size_base;

		archive->SeekHed(file_info->starts[i]);

		const size_t dst_size = file_info->sizes_org[i];

		for (size_t wrote_size = 0; wrote_size != dst_size; wrote_size += buffer_size)
		{
			// Adjust buffer size

			archive->SetBufSize(&buffer_size, wrote_size, dst_size);

			archive->Read(buffer.data(), buffer_size);
			archive->WriteFile(buffer.data(), buffer_size);
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
bool CKrkr::FindXP3FromExecuteFile(CArcFile* archive, size_t* offset)
{
	// Is not a kirikiri executable
	if (archive->GetArcSize() <= 0x200000)
	{
		return false;
	}

	*offset = 16;

	archive->SeekHed(16);

	std::array<u8, 4096> buffer;
	size_t read_size;

	do
	{
		read_size = archive->Read(buffer.data(), buffer.size());

		for (size_t i = 0, j = 0; i < read_size / 16; i++, j += 16)
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
			throw -1;
		}
	} while (read_size == buffer.size());

	// No XP3 archive
	*offset = 0;
	archive->SeekHed();

	return false;
}
