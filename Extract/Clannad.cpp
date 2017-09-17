#include "StdAfx.h"
#include "Extract/Clannad.h"

#include "ArcFile.h"
#include "Extract/Ahx.h"
#include "File.h"
#include "Mzx.h"
#include "Sound/Wav.h"

// Functions to get file information for Clannad's VOICE.MRG
bool CClannad::Mount(CArcFile* archive)
{
	if (archive->GetArcExten() != _T(".MRG") ||
	    memcmp(archive->GetHeader(), "LV", 2) != 0 ||
	    memcmp(archive->GetHeader() + 7, "MZX0", 4) != 0)
	{
		return false;
	}

	// Open VOICE.HED
	TCHAR hed_file_path[MAX_PATH];
	lstrcpy(hed_file_path, archive->GetArcPath());
	PathRenameExtension(hed_file_path, _T(".HED"));
	CFile hed_file;
	if (!hed_file.OpenForRead(hed_file_path))
		return false;

	// Get index size
	const size_t index_size = hed_file.GetFileSize() - 16;

	// Get index
	std::vector<u8> index(index_size);
	hed_file.Read(index.data(), index.size());
	const u8* index_ptr = index.data();

	// Number of files retrieved from index size
	const u32 num_files = index_size >> 2;

	// If its VOICE.MRG, start from 20000
	u32 count = (archive->GetArcName() == _T("VOICE.MRG")) ? 0 : 20000;

	for (u32 i = 0; i < num_files; i++)
	{
		const u16 pos = *reinterpret_cast<const u16*>(&index_ptr[0]);
		const u16 arg_size = *reinterpret_cast<const u16*>(&index_ptr[2]);

		const u32 section = count / 1000;
		TCHAR file_name[_MAX_FNAME];
		_stprintf(file_name, _T("KOE\\%04u\\Z%04u%05u.wav"), section, section, count++);

		// Add to listview
		SFileInfo infFile;
		infFile.name = file_name;
		infFile.sizeCmp = (arg_size & 0xfff) * 0x800;
		infFile.sizeOrg = infFile.sizeCmp * 6; // Appropriate. File size can not be obtained after the expansion to an unpacked 32-byte
		infFile.start = pos * 0x800 + (arg_size & 0xf000) * 0x8000;
		infFile.end = infFile.start + infFile.sizeCmp;
		infFile.format = _T("AHX");
		infFile.title = _T("CLANNAD");
		archive->AddFileInfo(infFile);

		index_ptr += 4;
	}

	return true;
}

// Function to perform extraction
bool CClannad::Decode(CArcFile* archive)
{
	const SFileInfo* file_info = archive->GetOpenFileInfo();

	if (file_info->title != _T("CLANNAD") || file_info->format != _T("AHX"))
		return false;

	// Ensure buffer
	std::vector<u8> mzx_buf(file_info->sizeCmp);

	// Read
	archive->Read(mzx_buf.data(), mzx_buf.size());

	// MZX Decompression
	const u32 ahx_buf_len = *reinterpret_cast<const u32*>(&mzx_buf[11]);
	std::vector<u8> ahx_buf(ahx_buf_len + 1024);
	CMzx mzx;
	mzx.Decompress(ahx_buf.data(), ahx_buf_len, &mzx_buf[7]);

	// Decode AHX to WAV
	CAhx ahx;
	ahx.Decode(archive, ahx_buf.data(), ahx_buf_len);

/*
	size_t wav_buf_len = BitUtils::Swap32(*reinterpret_cast<const u32*>(&ahx_buf[12])) * 2;
	std::vector<u8> wav_buf(wav_buf_len + 1152 * 2); // margen = layer-2 frame size
	CAhx ahx;
	wav_buf_len = ahx.Decompress(wav_buf.data(), ahx_buf.data(), ahx_buf_len);

	// Output
	archive->OpenFile();
	CWav wav;
	wav.WriteHed(archive, wav_buf_len, BitUtils::Swap32(*reinterpret_cast<const u32*>(&ahx_buf[8])), 1, 16);
	archive->WriteFile(wav_buf.data(), wav_buf_len);
*/
	return true;
}