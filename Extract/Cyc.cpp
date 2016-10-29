#include "stdafx.h"
#include "../Image.h"
#include "Cyc.h"

///////////////////////////////////////////////////////////////////////////
// Mount
///////////////////////////////////////////////////////////////////////////
bool CCyc::Mount(CArcFile* pclArc)
{
	if (MountGpk(pclArc))
		return true;
	if (MountVpk(pclArc))
		return true;
	if (MountDwq(pclArc))
		return true;
	if (MountWgq(pclArc))
		return true;
	if (MountVaw(pclArc))
		return true;
	if (MountXtx(pclArc))
		return true;
	if (MountFxf(pclArc))
		return true;

	return false;
}

bool CCyc::MountGpk(CArcFile* pclArc)
{
	if (pclArc->GetArcExten() != _T(".gpk"))
		return false;
	if (memcmp(&pclArc->GetHed()[48], "PACKTYPE=", 9) != 0)
		return false;

	// Get gtb file path
	TCHAR szGtbPath[MAX_PATH];
	lstrcpy(szGtbPath, pclArc->GetArcPath());
	PathRenameExtension(szGtbPath, _T(".gtb"));

	// Open gtb file
	CFile GtbFile;
	if (GtbFile.Open(szGtbPath, FILE_READ) == INVALID_HANDLE_VALUE)
		return false;

	// Number of files in the gtb file
	DWORD ctFile;
	GtbFile.Read(&ctFile, 4);

	// Determine table size
	DWORD tblSize = ctFile * 4;

	// Get the gtb file filename from the table
	YCMemory<DWORD> fnLenTbl(ctFile);
	GtbFile.Read(&fnLenTbl[0], tblSize);

	// Get the table offset of the gtb file
	YCMemory<DWORD> ofsTbl(ctFile);
	GtbFile.Read(&ofsTbl[0], tblSize);

	// Determine the size of the filename table
	DWORD fnTblSize = GtbFile.GetFileSize() - 4 - tblSize * 2;

	// Get the gtb file from the filename table
	YCMemory<char> fnTbl(fnTblSize);
	GtbFile.Read(&fnTbl[0], fnTblSize);

	LPSTR pfnTbl = &fnTbl[0];

	for (DWORD i = 0; i < ctFile; i++)
	{
		SFileInfo infFile;
		infFile.name = pfnTbl;
		infFile.name += _T(".dwq"); // dwq is archived within the gtb
		infFile.start = ofsTbl[i];
		infFile.end = (i + 1 == ctFile) ? pclArc->GetArcSize() : ofsTbl[i+1]; // End-of-file position is the very end of the gpk file
		infFile.sizeCmp = infFile.end - infFile.start;
		infFile.sizeOrg = infFile.sizeCmp;
		pclArc->AddFileInfo(infFile);

		pfnTbl += infFile.name.GetLength() - 3;
	}

	return true;
}

bool CCyc::MountVpk(CArcFile* pclArc)
{
	if (pclArc->GetArcExten() != _T(".vpk"))
		return false;
	if (memcmp(&pclArc->GetHed()[48], "PACKTYPE=", 9) != 0)
		return false;

	// Get vtb file path
	TCHAR szVtbPath[MAX_PATH];
	lstrcpy(szVtbPath, pclArc->GetArcPath());
	PathRenameExtension(szVtbPath, _T(".vtb"));
	
	// Open vtb file
	CFile VtbFile;
	if (VtbFile.Open(szVtbPath, FILE_READ) == INVALID_HANDLE_VALUE)
		return false;

	// Read vtb file
	DWORD vtbSize = VtbFile.GetFileSize();
	YCMemory<BYTE> vtb(vtbSize);
	VtbFile.Read(&vtb[0], vtbSize);

	LPBYTE pvtb = &vtb[0];

	while (true)
	{
		char szFileTitle[12];
		memcpy( szFileTitle, pvtb, 8 );
		szFileTitle[8] = '\0';

		SFileInfo infFile;
		infFile.name = szFileTitle;
		infFile.name += _T(".vaw");
		infFile.start = *(LPDWORD)&pvtb[8];
		infFile.end = *(LPDWORD)&pvtb[20];
		infFile.sizeCmp = infFile.end - infFile.start;
		infFile.sizeOrg = infFile.sizeCmp;
		pclArc->AddFileInfo(infFile);

		pvtb += 12;
		if (pvtb[0] == _T('\0'))
			break;
	}

	return true;
}

bool CCyc::MountDwq(CArcFile* pclArc)
{
	if (pclArc->GetArcExten() != _T(".dwq"))
		return false;
	if (memcmp(&pclArc->GetHed()[48], "PACKTYPE=", 9) != 0)
		return false;

	return pclArc->Mount();
}

bool CCyc::MountWgq(CArcFile* pclArc)
{
	if (pclArc->GetArcExten() != _T(".wgq"))
		return false;
	if (memcmp(&pclArc->GetHed()[48], "PACKTYPE=", 9) != 0)
		return false;

	return pclArc->Mount();
}

bool CCyc::MountVaw(CArcFile* pclArc)
{
	if (pclArc->GetArcExten() != _T(".vaw"))
		return false;
	if (memcmp(&pclArc->GetHed()[48], "PACKTYPE=", 9) != 0)
		return false;

	return pclArc->Mount();
}

bool CCyc::MountXtx(CArcFile* pclArc)
{
	if (pclArc->GetArcExten() != _T(".xtx"))
		return false;

	return pclArc->Mount();
}

bool CCyc::MountFxf(CArcFile* pclArc)
{
	if (pclArc->GetArcExten() != _T(".fxf"))
		return false;

	return pclArc->Mount();
}


///////////////////////////////////////////////////////////////////////////
// Decode
///////////////////////////////////////////////////////////////////////////
bool CCyc::Decode(CArcFile* pclArc)
{
	if (DecodeDwq(pclArc))
		return true;
	if (DecodeWgq(pclArc))
		return true;
	if (DecodeVaw(pclArc))
		return true;
	if (DecodeXtx(pclArc))
		return true;
	if (DecodeFxf(pclArc))
		return true;

	return false;
}

bool CCyc::DecodeDwq(CArcFile* pclArc)
{
	const SFileInfo* file_info = pclArc->GetOpenFileInfo();

	if (file_info->format != _T("DWQ"))
		return false;
	if (memcmp(&pclArc->GetHed()[48], "PACKTYPE=", 9) != 0)
		return false;

	// Read
	YCMemory<BYTE> buf(file_info->sizeCmp);
	pclArc->Read(&buf[0], file_info->sizeCmp);

	// JPEG + MASK(PACKBMP)
	if (memcmp(&buf[0], "JPEG+MASK", 9) == 0)
	{
		pclArc->OpenFile(_T(".jpg"));
		pclArc->WriteFile(&buf[64], *(LPDWORD)&buf[32]);

		//CJpg jpg;
		//DWORD dstSize;
		//LPBYTE dst = jpg.Decomp(&buf[64], *(LPDWORD)&buf[32], &dstSize);

		//DWORD maskOfs = 64 + *(LPDWORD)&buf[32];

		//// Get BMP header
		//LPBITMAPFILEHEADER fHed = (LPBITMAPFILEHEADER)&buf[maskOfs];
		//LPBITMAPINFOHEADER iHed = (LPBITMAPINFOHEADER)&buf[maskOfs+14];

		//// Ensure output buffer
		//DWORD maskSize = fHed->bfSize - fHed->bfOffBits;
		//YCMemory<BYTE> mask(maskSize);

		//// RLE Decompression
		//DecompRLE(&mask[0], &buf[maskOfs+fHed->bfOffBits], iHed->biWidth, iHed->biHeight);

		//// Alpha Blend
		//jpg.AlphaBlend(dst, &mask[0], 0x00);

		//// Output
		//CImage image;
		//image.Init(pclArc, jpg.GetWidth(), jpg.GetHeight(), 24);
		//image.WriteReverse(dst, dstSize);
	}
	// JPEG
	else if (memcmp(&buf[0], "JPEG", 4) == 0)
	{
		pclArc->OpenFile(_T(".jpg"));
		pclArc->WriteFile(&buf[64], *(LPDWORD)&buf[32]);
	}
	// BMP(RGB)
	else if (memcmp(&buf[0], "BMP", 3) == 0)
	{
		// Get BMP Header
		LPBITMAPFILEHEADER fHed = (LPBITMAPFILEHEADER)&buf[64];
		LPBITMAPINFOHEADER iHed = (LPBITMAPINFOHEADER)&buf[78];

		// BGR has to sort along with RGB
		if (iHed->biBitCount == 24)
			RGBtoBGR(&buf[64+54], iHed->biWidth, iHed->biHeight);

		// Deviation of the size adjustment
		DWORD dstSize = fHed->bfSize - fHed->bfOffBits;
		if (((iHed->biWidth * (iHed->biBitCount >> 3) + 3) & 0xFFFFFFFC) * iHed->biHeight != iHed->biSizeImage)
			dstSize -= 2;

		// Output
		CImage image;
		image.Init(pclArc, iHed->biWidth, iHed->biHeight, iHed->biBitCount, &buf[64+54], fHed->bfOffBits - 54);
		image.WriteReverse(&buf[64 + fHed->bfOffBits], dstSize);
	}
	// PACKBMP
	else if (memcmp(&buf[0], "PACKBMP", 7) == 0)
	{
		// Get BMP Header
		LPBITMAPFILEHEADER fHed = (LPBITMAPFILEHEADER)&buf[64];
		LPBITMAPINFOHEADER iHed = (LPBITMAPINFOHEADER)&buf[78];

		// Ensure output buffer
		DWORD dstSize = fHed->bfSize - fHed->bfOffBits;
		YCMemory<BYTE> dst(dstSize);

		// RLE Decompression
		DecompRLE(&dst[0], &buf[64+fHed->bfOffBits], iHed->biWidth, iHed->biHeight);

		// Output
		CImage image;
		image.Init(pclArc, iHed->biWidth, iHed->biHeight, iHed->biBitCount, &buf[64+54], fHed->bfOffBits - 54);
		image.WriteReverse(&dst[0], dstSize - 1); // Dummy output data
	}
	// BMP
	else if (memcmp(&buf[64], "BM", 2) == 0)
	{
		// Get BMP Header
		LPBITMAPFILEHEADER fHed = (LPBITMAPFILEHEADER)&buf[64];
		LPBITMAPINFOHEADER iHed = (LPBITMAPINFOHEADER)&buf[78];

		if (memcmp(&buf[48], "PACKTYPE=1", 10) == 0)
		{
			// Ensure output buffer
			DWORD dstSize = fHed->bfSize - fHed->bfOffBits;
			YCMemory<BYTE> dst(dstSize);

			// RLE Decompression
			DecompRLE(&dst[0], &buf[64+fHed->bfOffBits], iHed->biWidth, iHed->biHeight);

			// Output
			CImage image;
			image.Init(pclArc, iHed->biWidth, iHed->biHeight, iHed->biBitCount, &buf[64+54], fHed->bfOffBits - 54);
			image.WriteReverse(&dst[0], dstSize - 1); // Dummy output data
		}
		else
		{
			// Output
			CImage image;
			image.Init(pclArc, iHed->biWidth, iHed->biHeight, iHed->biBitCount, &buf[64+54], fHed->bfOffBits - 54);
			image.WriteReverse(&buf[64 + fHed->bfOffBits], fHed->bfSize - fHed->bfOffBits);
		}
	}
	// Other
	else
	{
		pclArc->OpenFile(_T(".dwq"));
		pclArc->WriteFile(&buf[0], file_info->sizeCmp);
	}

	return true;
}

bool CCyc::DecodeWgq(CArcFile* pclArc)
{
	const SFileInfo* file_info = pclArc->GetOpenFileInfo();

	if (file_info->format != _T("WGQ"))
		return false;
	if (memcmp(&pclArc->GetHed()[48], "PACKTYPE=", 9) != 0)
		return false;

	// Read
	YCMemory<BYTE> buf(file_info->sizeCmp);
	pclArc->Read(&buf[0], file_info->sizeCmp);

	// Output
	pclArc->OpenFile(_T(".ogg"));
	pclArc->WriteFile(&buf[64], file_info->sizeCmp);

	return true;
}

bool CCyc::DecodeVaw(CArcFile* pclArc)
{
	const SFileInfo* file_info = pclArc->GetOpenFileInfo();

	if (file_info->format != _T("VAW"))
		return false;
	if (memcmp(&pclArc->GetHed()[48], "PACKTYPE=", 9) != 0)
		return false;

	// Read
	YCMemory<BYTE> buf(file_info->sizeCmp);
	pclArc->Read(&buf[0], file_info->sizeCmp);

	// ogg
	if (memcmp(&buf[108], "OggS", 4) == 0)
	{
		pclArc->OpenFile(_T(".ogg"));
		pclArc->WriteFile(&buf[108], file_info->sizeCmp - 108);
	}
	// wav
	else
	{
		pclArc->OpenFile(_T(".wav"));
		pclArc->WriteFile(&buf[64], file_info->sizeCmp - 64);
	}

	return true;
}

bool CCyc::DecodeXtx(CArcFile* pclArc)
{
	const SFileInfo* file_info = pclArc->GetOpenFileInfo();

	if (file_info->format != _T("XTX"))
		return false;

	// Read
	YCMemory<BYTE> buf(file_info->sizeCmp);
	pclArc->Read(&buf[0], file_info->sizeCmp);

	// Output
	pclArc->OpenScriptFile();
	pclArc->WriteFile(&buf[0], file_info->sizeCmp);

	return true;
}

bool CCyc::DecodeFxf(CArcFile* pclArc)
{
	const SFileInfo* file_info = pclArc->GetOpenFileInfo();

	if (file_info->format != _T("FXF"))
		return false;

	// Read
	YCMemory<BYTE> buf(file_info->sizeCmp);
	pclArc->Read(&buf[0], file_info->sizeCmp);

	// Decryption
	for (DWORD i = 0; i < file_info->sizeCmp; i++)
		buf[i] ^= 0xFF;

	// Output
	pclArc->OpenScriptFile();
	pclArc->WriteFile(&buf[0], file_info->sizeCmp);

	return true;
}

void CCyc::DecompRLE(LPBYTE dst, LPBYTE src, LONG width, LONG height)
{
	// Ensure a 1 row/line buffer
	YCMemory<BYTE> dataWidth(width);
	YCMemory<BYTE> predataWidth(width);
	memset(&predataWidth[0], 0, width);

	for (int i = 0; i < height; i++)
	{
		// Pointer to the line buffer
		LPBYTE pdataWidth = &dataWidth[0];

		// Continue to process the rows
		for (int j = 0; j < width; )
		{
			// Get sequential input 
			BYTE data = *src++;

			// RLE Compression if data is zero
			if (data == 0)
			{
				// Following input data is written sequentially
				BYTE len = *src++;

				// RLE Decompression
				std::fill(pdataWidth, pdataWidth + len, data);

				// Proceed with decompressed length
				j += len;
			}
			// Entered into the dictionary as it is otherwise non-compressed data
			else
			{
				*pdataWidth++ = data;
				j++;
			}
		}

		// Decoded using the data of one row of data before this line
		for (int j = 0; j < width; j++)
		{
			dataWidth[j] ^= predataWidth[j];
			// For use in decoding of the next line, save the data in this row
			predataWidth[j] = dataWidth[j];
		}

		// Output the decoded data to the output buffer.
		pdataWidth = &dataWidth[0];
		std::copy(pdataWidth, pdataWidth + width, dst);
	}
}

void CCyc::RGBtoBGR(LPBYTE buf, LONG width, LONG height)
{
	// Seek the width
	LONG lwidth = width * 3;
	// Compute padding width
	LONG lwidth_rev = (lwidth + 3) & 0xFFFFFFFC;

	for (int y = 0; y < height; y++)
	{
		// Replace the R and B
		for (int x = 0; x < lwidth; x += 3)
		{
			std::swap(buf[0], buf[2]);
			buf += 3;
		}

		// Skip padding
		for (int x = lwidth; x < lwidth_rev; x++)
		{
			buf++;
		}
	}
}
