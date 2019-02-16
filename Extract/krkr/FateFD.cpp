#include "StdAfx.h"
#include "Extract/krkr/FateFD.h"

#include "ArcFile.h"
#include "Common.h"

/// Determine if decryption is possible
///
/// @param archive Archive
///
bool CFateFD::OnCheckDecrypt(CArcFile* archive)
{
	if (archive->GetArcName() != _T("video.xp3"))
		return false;

	return CheckTpm("9C5BB86A5BBD1B77A311EC504DB45653");
}

/// Initialization of the decryption process
///
/// @param archive Archive
///
u32 CFateFD::OnInitDecrypt(CArcFile* archive)
{
	const SFileInfo* file_info = archive->GetOpenFileInfo();

	if (file_info->name == _T("fd_op01.mpg"))
	{
		// OP1

		m_key[0] = 0xCE;
		m_key[1] = 0x94;
		m_key[2] = 0xB3;
		m_key[3] = 0x94;
		m_key[4] = 0xF8;
		m_key[5] = 0x94;

		m_offset[0] = 0;
		m_offset[1] = 0xB6B - file_info->start;
		m_offset[2] = 0x7036 - file_info->start;
		m_offset[3] = 0x7037 - file_info->start;
		m_offset[4] = 0xAF27 - file_info->start;
		m_offset[5] = 0xAF28 - file_info->start;
	}
	else if (file_info->name == _T("fd_op02.mpg"))
	{
		// OP2

		m_key[0] = 0x5A;
		m_key[1] = 0x21;
		m_key[2] = 0xFE;
		m_key[3] = 0x21;
		m_key[4] = 0xDB;
		m_key[5] = 0x21;

		m_offset[0] = 0;
		m_offset[1] = 0x552036D - file_info->start;
		m_offset[2] = 0x552254A - file_info->start;
		m_offset[3] = 0x552254B - file_info->start;
		m_offset[4] = 0x552CDF6 - file_info->start;
		m_offset[5] = 0x552CDF7 - file_info->start;
	}
	else
	{
		// Other

		SetDecryptRequirement(false);
	}

	return 0;
}

/// Decryption Process
///
/// @param target      Data to be decoded
/// @param target_size Data size
/// @param offset      Location of data to be decoded
/// @param decrypt_key Decryption key
///
size_t CFateFD::OnDecrypt(u8* target, size_t target_size, size_t offset, u32 decrypt_key)
{
	for (size_t i = 0; i < target_size; i++)
	{
		const size_t byte_offset = offset + i;

		if (byte_offset < m_offset[1])
		{
			target[i] ^= m_key[0];
		}
		else if (byte_offset < m_offset[2])
		{
			target[i] ^= m_key[1];
		}
		else if (byte_offset < m_offset[3])
		{
			target[i] ^= m_key[2];
		}
		else if (byte_offset < m_offset[4])
		{
			target[i] ^= m_key[3];
		}
		else if (byte_offset < m_offset[5])
		{
			target[i] ^= m_key[4];
		}
		else
		{
			target[i] ^= m_key[5];
		}
	}

	return target_size;
}
