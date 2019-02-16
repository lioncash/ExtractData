#include "StdAfx.h"
#include "Extract/Windmill.h"

#include "ArcFile.h"
#include "Arc/Zlib.h"
#include "Common.h"

bool CWindmill::Mount(CArcFile* archive)
{
	if (archive->GetArcExten() != _T(".int") || memcmp(archive->GetHeader(), "KIF", 3) != 0)
		return false;

	// Get file count
	u32 num_files;
	archive->Seek(4, FILE_BEGIN);
	archive->ReadU32(&num_files);

	// Number of files retrieved from the index size
	const u32 index_size = num_files * 40;

	// Get index
	std::vector<u8> index(index_size);
	archive->Read(index.data(), index.size());

	const u8* index_ptr = index.data();

	for (u32 i = 0; i < num_files; i++)
	{
		// Get file name
		TCHAR file_name[32];
		memcpy(file_name, index_ptr, 32);

		// Add to listview
		SFileInfo file_info;
		file_info.name = file_name;
		file_info.start = *reinterpret_cast<const u32*>(&index_ptr[32]);
		file_info.size_cmp = *reinterpret_cast<const u32*>(&index_ptr[36]);
		file_info.size_org = file_info.size_cmp;
		file_info.end = file_info.start + file_info.size_cmp;
		archive->AddFileInfo(file_info);

		index_ptr += 40;
	}

	return true;
}

bool CWindmill::Decode(CArcFile* archive)
{
	const SFileInfo* file_info = archive->GetOpenFileInfo();

	if (file_info->format != _T("HG2"))
		return false;

	return false; // TODO: Not completed yet

	// Read HG2 header
	u8 header[48];
	archive->Read(header, sizeof(header));

	// Supports HG2 from Happiness! and later
	u8 header2[4];
	archive->Read(header2, sizeof(header2));
	if (memcmp(header2, "\x78\xDA", 2) != 0)
		archive->Seek(*reinterpret_cast<u32*>(header) - 4, FILE_CURRENT);
	else
		archive->Seek(-4, FILE_CURRENT);

	// Width, height, number of bits
	const s32 width = *reinterpret_cast<const s32*>(&header[0x0C]);
	const s32 height = *reinterpret_cast<const s32*>(&header[0x10]);
	const u16 bpp = *reinterpret_cast<const u16*>(&header[0x14]);

	// Get buffer size
	auto dwSrcSize1 = *reinterpret_cast<const u32*>(&header[0x20]);
	auto dwDstSize1 = *reinterpret_cast<const u32*>(&header[0x24]);
	auto dwSrcSize2 = *reinterpret_cast<const u32*>(&header[0x28]);
	auto dwDstSize2 = *reinterpret_cast<const u32*>(&header[0x2C]);
	u32 dwDstSize = width * height * 4 * 2 + 256;

	// Ensure buffers exist
	std::vector<u8> clmbtSrc1(dwSrcSize1);
	std::vector<u8> clmbtDst1(dwDstSize1);
	std::vector<u8> clmbtSrc2(dwSrcSize2);
	std::vector<u8> clmbtDst2(dwDstSize2);
	std::vector<u8> clmbtDst(dwDstSize);

	// zlib decompression
	archive->Read(clmbtSrc1.data(), clmbtSrc1.size());
	archive->Read(clmbtSrc2.data(), clmbtSrc2.size());

	CZlib zlib;
	zlib.Decompress(clmbtDst1.data(), &dwDstSize1, clmbtSrc1.data(), clmbtSrc1.size());
	zlib.Decompress(clmbtDst2.data(), &dwDstSize2, clmbtSrc2.data(), clmbtSrc2.size());

	// Decoding
	u8 abyTable[128] = {};

	u32 dwDst1 = 0;
	u32 dwDst2a = 0;
	u32 dwDst2b = 0;
	u32 dwDst = 0;

	u32 EAX = 0;
	u32 EBX = 0;
	u32 ECX = 0;
	u32 EDX = 0;
	u32 EDI = 0;
	u32 ESI = 0;
	u32 EBP = 0;

	u32 SP[128];

	abyTable[0x44] = clmbtDst2[dwDst2b++];

	ECX = abyTable[0x40];
	EDX = abyTable[0x44];

	ESI = (EDX >> ECX) & 1;

	EAX++;
	EAX &= 0x07;

	abyTable[0x40] = EAX;

	SP[0] = ESI;


//-- 004049C0 ----------------------------------------------------------------------------

	EBP = 1;

	while (true)
	{
		ESI = abyTable[0x40];

		if (ESI == 0)
		{
			abyTable[0x44] = clmbtDst2[dwDst2b++];
		}

		ECX = abyTable[0x40];
		EDX = abyTable[0x44];

		EDX = (EDX >> ECX) & 1;

		ESI++;
		ESI &= 0x07;

		abyTable[0x40] = ESI;

		if (EDX != 0)
		{
			break;
		}

		EBP <<= 1;
	}

	EBX = EBP;
	ESI = 0;
	EBP >>= 1;

	if (EBP != 0)
	{
		while (true)
		{
			EDI = abyTable[0x40];

			ESI <<= 1;

			if (EDI == 0)
			{
				abyTable[0x44] = clmbtDst2[dwDst2b++];
			}

			ECX = abyTable[0x40];
			EDX = abyTable[0x44];

			EDX = (EDX >> ECX) & 0x01;

			EDI++;
			EDI &= 0x07;

			ESI |= EDX;

			EBP >>= 1;

			abyTable[0x40] = EDI;

			if (EBP == 0)
			{
				break;
			}
		}
	}

	EAX = ESI + EBX;

//----------------------------------------------------------------------------------------

	ESI = SP[0];
	EBX = EAX;
	ECX = EBX;
	EAX = 0;
	EDX = ECX;

	ECX >>= 2;

	EAX = abyTable[0x40];

	EDX = clmbtDst1[dwDst1++];

	clmbtDst[dwDst++] = EDX;

/*
	{
		u8* pbuf2 = &buf2[0];
		u32 tmp1 = 0;
		u32 tmp2 = 0;

		u32 EAX = tmp1;
		if (tmp1 == 0)
			tmp2 = *pbuf2++;
		u32 CL = tmp1;
		u32 DL = tmp2;
		u32 ESI = DL;
		ESI >>= CL;
		ESI &= 1;
		EAX++;
		EAX &= 7;
		tmp1 = EAX;

		EAX = Decode1(&pbuf2, &tmp1, &tmp2);
	}
*/
	return true;
}
