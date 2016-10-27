#include "stdafx.h"
#include "../ExtractBase.h"
#include "../Arc/LZSS.h"
#include "../Image.h"
#include "Axl.h"

bool CAxl::Mount(CArcFile* pclArc)
{
	if (pclArc->GetArcExten() != _T(".arc"))
		return false;

	if (*(LPDWORD)&pclArc->GetHed()[4] != 0 && *(LPDWORD)&pclArc->GetHed()[4] != 1)
		return false;

	// Get file count
	DWORD ctFile = *(LPDWORD)&pclArc->GetHed()[0];

	// Get filetype
	DWORD type = *(LPDWORD)&pclArc->GetHed()[4];

	// Number of files retrieved from the index size
	DWORD index_size = ctFile * 44;

	// Archive file size cannot be smaller than the index size
	if (pclArc->GetArcSize() < index_size)
		return false;

	pclArc->Seek(8, FILE_BEGIN);

	// Get index
	YCMemory<BYTE> index(index_size);
	LPBYTE pIndex = &index[0];
	pclArc->Read(pIndex, index_size);

	// Decrypt Index
	if (!DecryptIndex(pIndex, index_size, pclArc->GetArcSize()))
	{
		// Cannot be decoded
		pclArc->SeekHed();
		return false;
	}

	for (DWORD i = 0; i < ctFile; i++)
	{
		// Get filename
		TCHAR szFileName[33];
		memcpy(szFileName, pIndex, 32);
		szFileName[32] = _T('\0');

		// Add to listview
		SFileInfo infFile;
		infFile.name = szFileName;
		infFile.sizeOrg = *(LPDWORD)&pIndex[32];
		infFile.sizeCmp = *(LPDWORD)&pIndex[36];
		infFile.start = *(LPDWORD)&pIndex[40];
		infFile.end = infFile.start + infFile.sizeCmp;
		if (type == 1) infFile.format = _T("LZ");
		infFile.title = _T("AXL");
		pclArc->AddFileInfo(infFile);

		pIndex += 44;
	}

	return true;
}

bool CAxl::Decode(CArcFile* pclArc)
{
	const SFileInfo* file_info = pclArc->GetOpenFileInfo();

	if (file_info->title != _T("AXL"))
		return false;

	// Ensure buffer
	YCMemory<BYTE> buf(file_info->sizeOrg);

	// LZ Compressed File
	if (file_info->format == _T("LZ"))
	{
		// Reading
		YCMemory<BYTE> z_buf(file_info->sizeCmp);
		pclArc->Read(&z_buf[0], file_info->sizeCmp);

		// LZSS Decompression
		CLZSS clLZSS;
		clLZSS.Decomp( &buf[0], file_info->sizeOrg, &z_buf[0], file_info->sizeCmp, 4096, 4078, 3 );
	}
	else // Uncompressed File
	{
		pclArc->Read(&buf[0], file_info->sizeCmp);
	}

	// BMP File
	if (lstrcmpi(PathFindExtension(file_info->name), _T(".bmp")) == 0)
	{
		LPBITMAPFILEHEADER fHed = (LPBITMAPFILEHEADER)&buf[0];
		LPBITMAPINFOHEADER iHed = (LPBITMAPINFOHEADER)&buf[14];

		if (fHed->bfSize != file_info->sizeOrg)
		{
			// 32bit BMP

			// Make buffer for 32bit BMP
			YCMemory<BYTE> buf2(file_info->sizeOrg - 54);

			// Refers to the alpha value
			LPBYTE pbufA = &buf[fHed->bfSize];

			// Skip the BMP header
			LPBYTE pbuf = &buf[54];

			LPBYTE pbuf2 = &buf2[0];

			DWORD color_size = iHed->biWidth * iHed->biHeight;

			for (DWORD i = 0; i < color_size; i++)
			{
				// Synthesize alpha value
				for (int j = 0; j < 3; j++)
				{
					*pbuf2++ = *pbuf++;
				}

				*pbuf2++ = *pbufA++;
			}

			CImage image;
			image.Init(pclArc, iHed->biWidth, iHed->biHeight, 32);
			image.Write(&buf2[0], file_info->sizeOrg - 54);
		}
		else // Below 24bit BMP
		{
			CImage image;
			image.Init(pclArc, &buf[0]);
			image.Write(file_info->sizeOrg);
		}
	}
	else if (file_info->format == _T("LZ"))
	{
		// LZ Compressed files other than BMP

		pclArc->OpenFile();
		pclArc->WriteFile(&buf[0], file_info->sizeOrg);
	}
	else // Other file
	{
		pclArc->OpenFile();
		pclArc->WriteFile(&buf[0], file_info->sizeCmp);
	}

	return true;
}

void CAxl::InitMountKey(LPVOID deckey)
{
	m_len = strlen((char*)deckey);
	memcpy(m_deckey, deckey, m_len);
}

bool CAxl::CreateKey(LPBYTE key, LPINT key_len, LPBYTE pIndex, DWORD index_size)
{
	for (int i = 0; i < (int)index_size; i += 44)
	{
		// Copy the portion of the file name from the index
		BYTE fname[32];
		memcpy(fname, pIndex, 32);

		LPBYTE key_end = &fname[31];

		LPBYTE pkey1 = key_end;
		LPBYTE pkey2 = key_end - 1;

		*key_len = 1;

		while (pkey2 > fname)
		{
			while (*pkey1 != *pkey2)
			{
				// Moved if the same data
				pkey2--;

				// Keep counting the length of the key
				(*key_len)++;

				// After exiting the loop, go back to the beginning of the file name
				if (pkey2 == fname)
					break;
			}

			// After exiting the loop, go back to the beginning of the file name
			if (pkey2 == fname)
				break;

			int key_len_cpy = *key_len;

			// Make a comparison with previous data
			while (*pkey1 == *pkey2)
			{
				pkey1--;
				pkey2--;
				key_len_cpy--;

				// Once the key matches exactly
				if (key_len_cpy == 0)
				{
					// Length of the key to be copied to the first
					int key_len_hed = (i + 32) % *key_len;
					LPBYTE pkey3 = key_end + 1 - key_len_hed;

					for (int j = 0; j < key_len_hed; j++)
						key[j] = *pkey3++;

					// Followed by the actual key
					pkey3 = key_end + 1 - *key_len;

					for (int j = key_len_hed; j < *key_len; j++)
						key[j] = *pkey3++;

					return true;
				}

				// After exiting the loop, go back to the beginning of the file name
				if (pkey2 == fname)
					break;
			}

			// After exiting the loop, go back to the beginning of the file name
			if (pkey2 == fname)
				break;
		}

		pIndex += 44;
	}

	return false;
}

bool CAxl::DecryptIndex(LPBYTE pIndex, DWORD index_size, QWORD arcSize)
{
	// Key generation from the file name portion of the index
	BYTE key[32];
	int key_len;
	if (!CreateKey(key, &key_len, pIndex, index_size))
		return false;

	// Copy the index
	YCMemory<BYTE> pIndex_copy(index_size);
	memcpy(&pIndex_copy[0], pIndex, index_size);

	// Decoding the index that was copied
	for (DWORD i = 0, j = 0; i < index_size; i++)
	{
		pIndex_copy[i] -= key[j++];

		if (j == key_len)
		{
			j = 0;
		}
	}

	// Check whether the index matches the beginning and end of the first file
	if (*(LPDWORD)&pIndex_copy[40] != 8 + index_size)
		return false;

	// Check whether the match is the end of the last file and archive files
	if (*(LPDWORD)&pIndex_copy[index_size-4] + *(LPDWORD)&pIndex_copy[index_size-8] != arcSize)
		return false;

	// The copied index was the result of the check if there is no problem decoding
	memcpy(pIndex, &pIndex_copy[0], index_size);

	return true;

/*
	static char* key[] = {
		"SUMMER",     // チュートリアルサマー
		"HIDAMARI",   // ひだまり
		"KIMIKOE",    // キミの声がきこえる
		"CFTRIAL",    // CROSS FIRE Trial Version v1
		"TEST",       // CROSS FIRE, CROSS FIRE Trial Version v2
		"SAKURA-R",   // さくらリラクゼーション
		"KOICHU",     // こいちゅ！ ～恋に恋するかたおもい～
		"LAVENDURA",  // 黎明のラヴェンデュラ
		"HAMA",       // 真章 幻夢館, そらのいろ、みずのいろ
		"OBA",        // 叔母の寝室
		"KANSEN"      // 姦染 ～淫欲の連鎖～
	};

	YCMemory<BYTE> pIndex_copy(index_size);

	for (int i = 0; i < _countof(key); i++)
	{
		// Copy the index
		memcpy(&pIndex_copy[0], pIndex, index_size);
		// Determine length of the key
		int key_len = strlen((char*)key[i]);
		// Decryption
		for (int j = 0, k = 0; j < (int)index_size; j++)
		{
			pIndex_copy[j] += (BYTE)key[i][k++];
			if (k == key_len)
				k = 0;
		}
		// Check for a match between the beginning and end of the index of the first file
		// Decrypt another key if they do not match
		if (*(LPDWORD)&pIndex_copy[40] != 8 + index_size)
			continue;
		// Check whether the match is the end of the last file and the end of the archive files
		// If there is a match
		if (*(LPDWORD)&pIndex_copy[index_size-4] + *(LPDWORD)&pIndex_copy[index_size-8] == arcSize)
		{
			memcpy(pIndex, &pIndex_copy[0], index_size);
			return TRUE;
		}
	}

	return FALSE;
*/


	//LPBYTE deckey = m_deckey;
	//DWORD deckey_len = m_len;

	//for (int i = 0, j = 0; i < (int)index_size; i++)
	//{
	//	pIndex[i] += deckey[j++];
	//	if (j == deckey_len)
	//      j = 0;
	//}
}
