/*
   Copyright (c) 2005 Peter Jolly

   Permission is hereby granted, free of charge, to any person obtaining a copy 
   of this software and associated documentation files (the "Software"), to deal 
   in the Software without restriction, including without limitation the rights 
   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell 
   copies of the Software, and to permit persons to whom the Software is 
   furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included in 
   all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, 
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE 
   SOFTWARE.
*/

#include "stdafx.h"
#include "../ExtractBase.h"
#include "../Image.h"
#include "Majiro.h"

/// Mounting
///
/// @param pclArc Archive
///
bool CMajiro::Mount(CArcFile* pclArc)
{
	if (MountArc1(pclArc))
		return true;

	if (MountArc2(pclArc))
		return true;

	if (MountMJO(pclArc))
		return true;

	if (MountRC(pclArc))
		return true;

	return false;
}

/// Mounting V1 Archives
///
/// @param pclArc Archive
///
bool CMajiro::MountArc1(CArcFile* pclArc)
{
	if (pclArc->GetArcExten() != _T(".arc"))
		return false;

	if (memcmp(pclArc->GetHed(), "MajiroArcV1.000", 15) != 0)
		return false;

	// Get file count
	DWORD dwFiles;
	pclArc->SeekHed(16);
	pclArc->Read(&dwFiles, 4);

	// Get the index size of the file entry
	DWORD dwIndexSizeOfFileEntry;
	pclArc->Read(&dwIndexSizeOfFileEntry, 4);
	dwIndexSizeOfFileEntry -= 28;

	// Get index size
	DWORD dwIndexSize;
	pclArc->Read(&dwIndexSize, 4);
	dwIndexSize -= 28;

	// Get index
	YCMemory<BYTE> clmbtIndex(dwIndexSize);
	pclArc->Read(&clmbtIndex[0], dwIndexSize);

	// Get the index filename
	BYTE* pbtFileNameIndex = &clmbtIndex[dwIndexSizeOfFileEntry];

	// Getting file information
	std::vector<SFileInfo> vcFileInfo;
	std::vector<SFileInfo> vcMaskFileInfo;
	std::vector<SFileInfo> vcNotMaskFileInfo;

	for (DWORD i = 0; i < dwFiles; i++)
	{
		SFileInfo stFileInfo;

		// Get the filename from the filename index
		TCHAR szFileName[256];
		lstrcpy(szFileName, (LPCTSTR)pbtFileNameIndex);
		stFileInfo.name = szFileName;

		// Get the starting and ending addresses from the index
		stFileInfo.start = *(DWORD*)&clmbtIndex[i * 8 + 4];
		stFileInfo.end = *(DWORD*)&clmbtIndex[i * 8 + 12];

		// Get filesize
		stFileInfo.sizeOrg = stFileInfo.end - stFileInfo.start;
		stFileInfo.sizeCmp = stFileInfo.sizeOrg;

		// Add file information to list
		if (lstrcmpi(PathFindExtension(stFileInfo.name), _T(".rc8")) == 0)
		{
			if (stFileInfo.name[stFileInfo.name.GetLength() - lstrlen(_T("_.rc8"))] == _T('_'))
			{
				vcMaskFileInfo.push_back(stFileInfo);
			}
			else
			{
				vcNotMaskFileInfo.push_back(stFileInfo);
			}
		}
		else
		{
			vcFileInfo.push_back(stFileInfo);
		}

		pbtFileNameIndex += lstrlen(szFileName) + 1;
	}

	// Sort by filename
	std::sort(vcFileInfo.begin(), vcFileInfo.end(), CArcFile::CompareForFileInfo);

	// Get file information from the masked image
	for (auto& pstsiMask : vcMaskFileInfo)
	{
		// Get the name of the file to be created
		TCHAR szRCTName[_MAX_FNAME];
		lstrcpy(szRCTName, pstsiMask.name);
		szRCTName[lstrlen(szRCTName) - lstrlen(_T("_.rc8"))] = _T('\0');
		PathRenameExtension(szRCTName, _T(".rct"));

		// Get the file information to be created
		SFileInfo* pstsiTarget = pclArc->SearchForFileInfo(vcFileInfo, szRCTName);
		if (pstsiTarget != nullptr)
		{
			// Image is masked
			pstsiTarget->starts.push_back(pstsiMask.start);
			pstsiTarget->sizesCmp.push_back(pstsiMask.sizeCmp);
			pstsiTarget->sizesOrg.push_back(pstsiMask.sizeOrg);

			// Update progress
			pclArc->GetProg()->UpdatePercent(pstsiMask.sizeCmp);
		}
		else
		{
			// Image is not masked
			vcNotMaskFileInfo.push_back(pstsiMask);
		}
	}

	// Add to list view
	for (auto& fileInfo : vcFileInfo)
	{
		pclArc->AddFileInfo(fileInfo);
	}

	for (auto& notMaskFileInfo : vcNotMaskFileInfo)
	{
		pclArc->AddFileInfo(notMaskFileInfo);
	}

	return true;
}

/// Mounting V2 Archives
///
/// @param pclArc Archive
///
bool CMajiro::MountArc2(CArcFile* pclArc)
{
	if (pclArc->GetArcExten() != _T(".arc"))
		return false;

	if (memcmp(pclArc->GetHed(), "MajiroArcV2.000", 15) != 0)
		return false;

	// Get file count
	DWORD dwFiles;
	pclArc->SeekHed(16);
	pclArc->Read(&dwFiles, 4);

	// Get the index size of the file entry
	DWORD dwIndexSizeOfFileEntry;
	pclArc->Read(&dwIndexSizeOfFileEntry, 4);
	dwIndexSizeOfFileEntry -= 28;

	// Get index size
	DWORD dwIndexSize;
	pclArc->Read(&dwIndexSize, 4);
	dwIndexSize -= 28;

	// Get index
	YCMemory<BYTE> clmbtIndex(dwIndexSize);
	pclArc->Read(&clmbtIndex[0], dwIndexSize);

	// Get filename index
	BYTE* pbtFileNameIndex = &clmbtIndex[dwIndexSizeOfFileEntry];

	// Getting file information
	std::vector<SFileInfo> vcFileInfo;
	std::vector<SFileInfo> vcMaskFileInfo;
	std::vector<SFileInfo> vcNotMaskFileInfo;

	for (DWORD i = 0; i < dwFiles; i++)
	{
		SFileInfo stFileInfo;

		// Get filename from the filename index
		TCHAR szFileName[256];
		lstrcpy(szFileName, (LPCTSTR)pbtFileNameIndex);
		stFileInfo.name = szFileName;

		// Get the starting address from the index and filesize
		stFileInfo.start = *(LPDWORD)&clmbtIndex[i * 12 + 4];
		stFileInfo.sizeCmp = *(LPDWORD)&clmbtIndex[i * 12 + 8];
		stFileInfo.sizeOrg = stFileInfo.sizeCmp;

		// Get the ending address
		stFileInfo.end = stFileInfo.start + stFileInfo.sizeCmp;

		// Add file information to list
		if (lstrcmpi(PathFindExtension(stFileInfo.name), _T(".rc8")) == 0)
		{
			if (stFileInfo.name[stFileInfo.name.GetLength() - lstrlen(_T("_.rc8"))] == _T('_'))
			{
				vcMaskFileInfo.push_back(stFileInfo);
			}
			else
			{
				vcNotMaskFileInfo.push_back(stFileInfo);
			}
		}
		else
		{
			vcFileInfo.push_back(stFileInfo);
		}

		pbtFileNameIndex += lstrlen(szFileName) + 1;
	}

	// Sort by filename
	std::sort(vcFileInfo.begin(), vcFileInfo.end(), CArcFile::CompareForFileInfo);

	// Get file information from the masked image
	for (auto& pstsiMask : vcMaskFileInfo)
	{
		// Get the name of the file to be created
		TCHAR szRCTName[_MAX_FNAME];
		lstrcpy(szRCTName, pstsiMask.name);
		szRCTName[lstrlen(szRCTName) - lstrlen(_T("_.rc8"))] = _T('\0');
		PathRenameExtension(szRCTName, _T(".rct"));

		// Get the file information to be created
		SFileInfo* pstsiTarget = pclArc->SearchForFileInfo(vcFileInfo, szRCTName);

		if (pstsiTarget != nullptr)
		{
			// Image is masked
			pstsiTarget->starts.push_back(pstsiMask.start);
			pstsiTarget->sizesCmp.push_back(pstsiMask.sizeCmp);
			pstsiTarget->sizesOrg.push_back(pstsiMask.sizeOrg);

			// Update progress
			pclArc->GetProg()->UpdatePercent(pstsiMask.sizeCmp);
		}
		else
		{
			// Image is not masked
			vcNotMaskFileInfo.push_back(pstsiMask);
		}
	}

	// Add to list view
	for (auto& fileInfo : vcFileInfo)
	{
		pclArc->AddFileInfo(fileInfo);
	}

	for (auto& notMaskFileInfo : vcNotMaskFileInfo)
	{
		pclArc->AddFileInfo(notMaskFileInfo);
	}

	return true;
}

/// MJO Mounting
///
/// @param pclArc Archive
///
bool CMajiro::MountMJO(CArcFile* pclArc)
{
	if (memcmp(pclArc->GetHed(), "MajiroObjX", 10) != 0)
		return false;

	return pclArc->Mount();
}

/// RC Mounting
///
/// @param pclArc Archive
///
bool CMajiro::MountRC(CArcFile* pclArc)
{
	if (memcmp(pclArc->GetHed(), "六丁", 4) != 0)
		return false;

	return pclArc->Mount();
}

/// Decoding
///
/// @param pclArc Archive
///
bool CMajiro::Decode(CArcFile* pclArc)
{
	if (DecodeMJO(pclArc))
		return true;

	if (DecodeRC(pclArc))
		return true;

	return false;
}

/// MJO Decoding
///
/// @param pclArc Archive
///
bool CMajiro::DecodeMJO(CArcFile* pclArc)
{
	SFileInfo* pstFileInfo = pclArc->GetOpenFileInfo();
	if (pstFileInfo->format != _T("MJO"))
		return false;

	// Decoding table

	static const DWORD dwKeyTable[] =
	{
		0x00000000, 0x77073096, 0xEE0E612C, 0x990951BA, 0x076DC419, 0x706AF48F, 0xE963A535, 0x9E6495A3,
		0x0EDB8832, 0x79DCB8A4, 0xE0D5E91E, 0x97D2D988, 0x09B64C2B, 0x7EB17CBD, 0xE7B82D07, 0x90BF1D91,
		0x1DB71064, 0x6AB020F2, 0xF3B97148, 0x84BE41DE, 0x1ADAD47D, 0x6DDDE4EB, 0xF4D4B551, 0x83D385C7,
		0x136C9856, 0x646BA8C0, 0xFD62F97A, 0x8A65C9EC, 0x14015C4F, 0x63066CD9, 0xFA0F3D63, 0x8D080DF5,
		0x3B6E20C8, 0x4C69105E, 0xD56041E4, 0xA2677172, 0x3C03E4D1, 0x4B04D447, 0xD20D85FD, 0xA50AB56B,
		0x35B5A8FA, 0x42B2986C, 0xDBBBC9D6, 0xACBCF940, 0x32D86CE3, 0x45DF5C75, 0xDCD60DCF, 0xABD13D59,
		0x26D930AC, 0x51DE003A, 0xC8D75180, 0xBFD06116, 0x21B4F4B5, 0x56B3C423, 0xCFBA9599, 0xB8BDA50F,
		0x2802B89E, 0x5F058808, 0xC60CD9B2, 0xB10BE924, 0x2F6F7C87, 0x58684C11, 0xC1611DAB, 0xB6662D3D,
		0x76DC4190, 0x01DB7106, 0x98D220BC, 0xEFD5102A, 0x71B18589, 0x06B6B51F, 0x9FBFE4A5, 0xE8B8D433,
		0x7807C9A2, 0x0F00F934, 0x9609A88E, 0xE10E9818, 0x7F6A0DBB, 0x086D3D2D, 0x91646C97, 0xE6635C01,
		0x6B6B51F4, 0x1C6C6162, 0x856530D8, 0xF262004E, 0x6C0695ED, 0x1B01A57B, 0x8208F4C1, 0xF50FC457,
		0x65B0D9C6, 0x12B7E950, 0x8BBEB8EA, 0xFCB9887C, 0x62DD1DDF, 0x15DA2D49, 0x8CD37CF3, 0xFBD44C65,
		0x4DB26158, 0x3AB551CE, 0xA3BC0074, 0xD4BB30E2, 0x4ADFA541, 0x3DD895D7, 0xA4D1C46D, 0xD3D6F4FB,
		0x4369E96A, 0x346ED9FC, 0xAD678846, 0xDA60B8D0, 0x44042D73, 0x33031DE5, 0xAA0A4C5F, 0xDD0D7CC9,
		0x5005713C, 0x270241AA, 0xBE0B1010, 0xC90C2086, 0x5768B525, 0x206F85B3, 0xB966D409, 0xCE61E49F,
		0x5EDEF90E, 0x29D9C998, 0xB0D09822, 0xC7D7A8B4, 0x59B33D17, 0x2EB40D81, 0xB7BD5C3B, 0xC0BA6CAD,
		0xEDB88320, 0x9ABFB3B6, 0x03B6E20C, 0x74B1D29A, 0xEAD54739, 0x9DD277AF, 0x04DB2615, 0x73DC1683,
		0xE3630B12, 0x94643B84, 0x0D6D6A3E, 0x7A6A5AA8, 0xE40ECF0B, 0x9309FF9D, 0x0A00AE27, 0x7D079EB1,
		0xF00F9344, 0x8708A3D2, 0x1E01F268, 0x6906C2FE, 0xF762575D, 0x806567CB, 0x196C3671, 0x6E6B06E7,
		0xFED41B76, 0x89D32BE0, 0x10DA7A5A, 0x67DD4ACC, 0xF9B9DF6F, 0x8EBEEFF9, 0x17B7BE43, 0x60B08ED5,
		0xD6D6A3E8, 0xA1D1937E, 0x38D8C2C4, 0x4FDFF252, 0xD1BB67F1, 0xA6BC5767, 0x3FB506DD, 0x48B2364B,
		0xD80D2BDA, 0xAF0A1B4C, 0x36034AF6, 0x41047A60, 0xDF60EFC3, 0xA867DF55, 0x316E8EEF, 0x4669BE79,
		0xCB61B38C, 0xBC66831A, 0x256FD2A0, 0x5268E236, 0xCC0C7795, 0xBB0B4703, 0x220216B9, 0x5505262F,
		0xC5BA3BBE, 0xB2BD0B28, 0x2BB45A92, 0x5CB36A04, 0xC2D7FFA7, 0xB5D0CF31, 0x2CD99E8B, 0x5BDEAE1D,
		0x9B64C2B0, 0xEC63F226, 0x756AA39C, 0x026D930A, 0x9C0906A9, 0xEB0E363F, 0x72076785, 0x05005713,
		0x95BF4A82, 0xE2B87A14, 0x7BB12BAE, 0x0CB61B38, 0x92D28E9B, 0xE5D5BE0D, 0x7CDCEFB7, 0x0BDBDF21,
		0x86D3D2D4, 0xF1D4E242, 0x68DDB3F8, 0x1FDA836E, 0x81BE16CD, 0xF6B9265B, 0x6FB077E1, 0x18B74777,
		0x88085AE6, 0xFF0F6A70, 0x66063BCA, 0x11010B5C, 0x8F659EFF, 0xF862AE69, 0x616BFFD3, 0x166CCF45,
		0xA00AE278, 0xD70DD2EE, 0x4E048354, 0x3903B3C2, 0xA7672661, 0xD06016F7, 0x4969474D, 0x3E6E77DB,
		0xAED16A4A, 0xD9D65ADC, 0x40DF0B66, 0x37D83BF0, 0xA9BCAE53, 0xDEBB9EC5, 0x47B2CF7F, 0x30B5FFE9,
		0xBDBDF21C, 0xCABAC28A, 0x53B39330, 0x24B4A3A6, 0xBAD03605, 0xCDD70693, 0x54DE5729, 0x23D967BF,
		0xB3667A2E, 0xC4614AB8, 0x5D681B02, 0x2A6F2B94, 0xB40BBE37, 0xC30C8EA1, 0x5A05DF1B, 0x2D02EF8D
	};

	// Skip
	DWORD dwOffset;
	pclArc->SeekCur(24);
	pclArc->Read(&dwOffset, 4);
	dwOffset *= 8;

	// Get the output size
	DWORD dwDstSize;
	pclArc->SeekCur(dwOffset);
	pclArc->Read(&dwDstSize, 4);

	// Read
	YCMemory<BYTE> clmbtDst(dwDstSize + 3);
	pclArc->Read(&clmbtDst[0], dwDstSize);

	// Decode
	for (DWORD i = 0, j = 0; i < dwDstSize; i += 4)
	{
		*(DWORD*)&clmbtDst[i] ^= dwKeyTable[j++];
		j &= 255;
	}

	// Output
	pclArc->OpenScriptFile();
	pclArc->WriteFile(&clmbtDst[0], dwDstSize);

	return true;
}

/// RC Decoding
///
/// @param pclArc Archive
///
bool CMajiro::DecodeRC(CArcFile* pclArc)
{
	SFileInfo* pstFileInfo = pclArc->GetOpenFileInfo();
	if ((pstFileInfo->format != _T("RCT")) && (pstFileInfo->format != _T("RC8")))
	{
		return false;
	}

	// rc8/rct reading
	DWORD dwSrcSize = pstFileInfo->sizeCmp;
	YCMemory<BYTE> clmbtSrc(dwSrcSize);
	pclArc->Read(&clmbtSrc[0], dwSrcSize);

	// rc8/rct not supported
	//if ((memcmp(z_pbuf, "六丁TC00", 8) != 0) && (memcmp(z_pbuf, "六丁8_00", 8) != 0)) {
	// Output
	//    pclArc->OpenFile();
	//    pclArc->WriteFile(z_pbuf, pInfFile->sizeCmp);
	//    return TRUE;
	//}

	WORD  wBpp = (memcmp(&clmbtSrc[4], "8_00", 4) == 0) ? 8 : 24;
	long  lWidth = *(long*)&clmbtSrc[8];
	long  lHeight = *(long*)&clmbtSrc[12];
	DWORD dwSrcSizeOfData = *(DWORD*)&clmbtSrc[16];

	DWORD dwSrcPtr = 20;

	// Ensure output buffer exists
	DWORD          dwDstSize = lWidth * lHeight * (wBpp >> 3);
	YCMemory<BYTE> clmbtDst(dwDstSize);
	DWORD          dwDstSize2;
	YCMemory<BYTE> clmbtDst2;
	BYTE*          pbtDst = &clmbtDst[0];

	if (wBpp == 8)
	{
		// rc8

		read_bits_8(&clmbtDst[0], dwDstSize, &clmbtSrc[dwSrcPtr + 768], dwSrcSizeOfData, lWidth);
	}
	else if (memcmp(&clmbtSrc[4], "TC00", 4) == 0)
	{
		// rct

		read_bits_24(&clmbtDst[0], dwDstSize, &clmbtSrc[dwSrcPtr], dwSrcSizeOfData, lWidth);

		// マスク画像を付加して32bit化
		dwDstSize2 = lWidth * lHeight * 4;
		clmbtDst2.resize(dwDstSize2);

		if (AppendMask(pclArc, &clmbtDst2[0], dwDstSize2, &clmbtDst[0], dwDstSize))
		{
			// Successfully appended the mask image

			wBpp = 32;
			dwDstSize = dwDstSize2;
			pbtDst = &clmbtDst2[0];
		}
	}
	else if (memcmp(&clmbtSrc[4], "TC01", 4) == 0)
	{
		// rct(Difference information)

		// Get the base image file name
		TCHAR szFileName[_MAX_FNAME];
		WORD  wFileNameLen = *(WORD*)&clmbtSrc[dwSrcPtr];
		memcpy(szFileName, &clmbtSrc[dwSrcPtr + 2], wFileNameLen);
		dwSrcPtr += 2 + wFileNameLen;

		// Get file info for the base image
		const SFileInfo* pstFileInfoOfBase = pclArc->GetFileInfo(szFileName);

		if (pstFileInfoOfBase != nullptr)
		{
			// Base image file exists
			YCMemory<BYTE> clmbtSrcOfBase(pstFileInfoOfBase->sizeCmp);
			pclArc->SeekHed(pstFileInfoOfBase->start);
			pclArc->Read(&clmbtSrcOfBase[0], pstFileInfoOfBase->sizeCmp);
			read_bits_24(&clmbtDst[0], dwDstSize, &clmbtSrcOfBase[20], *(DWORD*)&clmbtSrcOfBase[16], lWidth);

			// Decompress the difference image
			YCMemory<BYTE> clmbtDstOfdiff(dwDstSize);
			read_bits_24(&clmbtDstOfdiff[0], dwDstSize, &clmbtSrc[dwSrcPtr], dwSrcSizeOfData, lWidth);

			// Synthesize the difference between the difference image and the base image
			for (DWORD i = 0; i < dwDstSize; i += 3)
			{
				if (memcmp(&clmbtDstOfdiff[i], "\x00\x00\xFF", 3) != 0)
				{
					// Red part is determined to be the transparent color in the difference image
					memcpy(&clmbtDst[i], &clmbtDstOfdiff[i], 3);
				}
			}
		}

		// マスク画像を付加して32bit化

		dwDstSize2 = lWidth * lHeight * 4;
		clmbtDst2.resize(dwDstSize2);

		if (AppendMask(pclArc, &clmbtDst2[0], dwDstSize2, &clmbtDst[0], dwDstSize))
		{
			// Success in adding the mask image

			wBpp = 32;
			dwDstSize = dwDstSize2;
			pbtDst = &clmbtDst2[0];
		}
	}
	else
	{
		pclArc->OpenFile();
		pclArc->WriteFile(&clmbtSrc[0], dwSrcSize);

		return true;
	}

	// Output
	CImage clImage;
	clImage.Init(pclArc, lWidth, lHeight, wBpp, &clmbtSrc[dwSrcPtr], 768);
	clImage.WriteReverse(pbtDst, dwDstSize);

	return true;
}

/// RCT Extraction
///
/// @param pbtDst    Destination
/// @param dwDstSize Destination size
/// @param pbtSrc    Compressed data
/// @param dwSrcSize Compressed data size
/// @param lWidth    Width
///
void CMajiro::read_bits_24(BYTE* pbtDst, DWORD dwDstSize, const BYTE* pbtSrc, DWORD dwSrcSize, long lWidth)
{
	DWORD dwSrcPtr = 0;
	DWORD dwDstPtr = 0;

	memcpy(pbtDst, pbtSrc, 3);

	dwSrcPtr += 3;
	dwDstPtr += 3;

	while ((dwSrcPtr < dwSrcSize) && (dwDstPtr < dwDstSize))
	{
		BYTE btCode = pbtSrc[dwSrcPtr++];

		if (btCode <= 0x7F)
		{
			DWORD dwLength;
			if (btCode == 0x7F)
			{
				dwLength = *(WORD*)&pbtSrc[dwSrcPtr] + 128;
				dwSrcPtr += 2;
			}
			else
			{
				dwLength = btCode + 1;
			}

			dwLength *= 3;

			if ((dwSrcPtr + dwLength) > dwSrcSize)
			{
				dwLength = (dwSrcSize - dwSrcPtr);
			}

			if ((dwDstPtr + dwLength) > dwDstSize)
			{
				dwLength = (dwDstSize - dwDstPtr);
			}

			memcpy(&pbtDst[dwDstPtr], &pbtSrc[dwSrcPtr], dwLength);

			dwSrcPtr += dwLength;
			dwDstPtr += dwLength;
		}
		else
		{
			DWORD dwLength = btCode & 0x03;
			if (dwLength == 3)
			{
				dwLength = *(WORD*)&pbtSrc[dwSrcPtr] + 4;
				dwSrcPtr += 2;
			}
			else
			{
				dwLength++;
			}

			DWORD dwBack = (btCode >> 2) - 32;
			if (dwBack < 6)
			{
				dwBack++;
			}
			else if (dwBack <= 12)
			{
				dwBack = lWidth + dwBack - 9;
			}
			else if (dwBack <= 19)
			{
				dwBack = lWidth * 2 + dwBack - 16;
			}
			else if (dwBack <= 26)
			{
				dwBack = lWidth * 3 + dwBack - 23;
			}
			else
			{
				dwBack = lWidth * 4 + dwBack - 29;
			}

			dwLength *= 3;
			dwBack *= 3;

			if ((dwDstPtr + dwLength) > dwDstSize)
			{
				dwLength = (dwDstSize - dwDstPtr);
			}

			for (DWORD i = 0; i < dwLength; i++)
			{
				pbtDst[dwDstPtr + i] = pbtDst[dwDstPtr - dwBack + i];
			}

			dwDstPtr += dwLength;
		}
	}
}

/// RC8 Extraction
///
/// @param pbtDst    Destination
/// @param dwDstSize Destination size
/// @param pbtSrc    Compressed data
/// @param dwSrcSize Compressed data size
/// @param lWidth    Width
///
void CMajiro::read_bits_8(BYTE* pbtDst, DWORD dwDstSize, const BYTE* pbtSrc, DWORD dwSrcSize, long lWidth)
{
	DWORD dwSrcPtr = 0;
	DWORD dwDstPtr = 0;

	pbtDst[dwDstPtr++] = pbtSrc[dwSrcPtr++];

	while ((dwSrcPtr < dwSrcSize) && (dwDstPtr < dwDstSize))
	{
		BYTE btCode = pbtSrc[dwSrcPtr++];
		if (btCode <= 0x7F)
		{
			DWORD dwLength;

			if (btCode == 0x7F)
			{
				dwLength = *(WORD*)&pbtSrc[dwSrcPtr] + 128;
				dwSrcPtr += 2;
			}
			else
			{
				dwLength = btCode + 1;
			}

			if ((dwSrcPtr + dwLength) > dwSrcSize)
			{
				dwLength = (dwSrcSize - dwSrcPtr);
			}

			if ((dwDstPtr + dwLength) > dwDstSize)
			{
				dwLength = (dwDstSize - dwDstPtr);
			}

			memcpy(&pbtDst[dwDstPtr], &pbtSrc[dwSrcPtr], dwLength);

			dwSrcPtr += dwLength;
			dwDstPtr += dwLength;
		}
		else
		{
			DWORD dwLength = btCode & 0x07;
			if (dwLength == 7)
			{
				dwLength = *(WORD*)&pbtSrc[dwSrcPtr] + 10;
				dwSrcPtr += 2;
			}
			else
			{
				dwLength += 3;
			}

			DWORD dwBack = (btCode >> 3) - 16;
			if (dwBack < 4)
			{
				dwBack++;
			}
			else if (dwBack <= 10)
			{
				dwBack = lWidth + dwBack - 7;
			}
			else
			{
				dwBack = lWidth * 2 + dwBack - 13;
			}

			if ((dwDstPtr + dwLength) > dwDstSize)
			{
				dwLength = (dwDstSize - dwDstPtr);
			}

			for (DWORD i = 0; i < dwLength; i++)
			{
				pbtDst[dwDstPtr + i] = pbtDst[dwDstPtr - dwBack + i];
			}

			dwDstPtr += dwLength;
		}
	}
}

/// マスク画像を付加して32bit化する
///
/// @param pclArc    Archive
/// @param pbtDst    Destination
/// @param dwDstSize Destination size
/// @param pbtSrc    24-bit data
/// @param dwSrcSize Data size
///
bool CMajiro::AppendMask(CArcFile* pclArc, BYTE* pbtDst, DWORD dwDstSize, const BYTE* pbtSrc, DWORD dwSrcSize)
{
	SFileInfo* pstFileInfo = pclArc->GetOpenFileInfo();
	if (pstFileInfo->starts.empty())
	{
		// Mask image doesn't exist.

		return false;
	}

	// Read image mask
	DWORD          dwSrcSizeOfMask = pstFileInfo->sizesCmp[0];
	YCMemory<BYTE> clmbtSrcOfMask(dwSrcSizeOfMask);
	pclArc->SeekHed(pstFileInfo->starts[0]);
	pclArc->Read(&clmbtSrcOfMask[0], dwSrcSizeOfMask);

	// Get header information
	SRCHeader strchMask;
	std::memcpy(&strchMask, &clmbtSrcOfMask[0], sizeof(SRCHeader));

	// Decompress masked image
	DWORD          dwDstSizeOfMask = strchMask.lWidth * strchMask.lHeight;
	YCMemory<BYTE> clmbtDstOfMask(dwDstSizeOfMask);
	read_bits_8(&clmbtDstOfMask[0], dwDstSizeOfMask, &clmbtSrcOfMask[20 + 768], strchMask.dwDataSize, strchMask.lWidth);

	// Make file
	DWORD dwSrcPtr = 0;
	DWORD dwMaskPtr = 0;
	DWORD dwDstPtr = 0;

	while (dwSrcPtr < dwSrcSize && dwMaskPtr < dwDstSizeOfMask && dwDstPtr < dwDstSize)
	{
		memcpy(&pbtDst[dwDstPtr], &pbtSrc[dwSrcPtr], 3);

		pbtDst[dwDstPtr + 3] = ~clmbtDstOfMask[dwMaskPtr];

		dwSrcPtr += 3;
		dwMaskPtr += 1;
		dwDstPtr += 4;
	}

	return true;
}
