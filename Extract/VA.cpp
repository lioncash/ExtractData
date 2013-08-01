#include "stdafx.h"
#include "../ExtractBase.h"
#include "../Sound/Wav.h"
#include "VA.h"

BOOL CVA::Mount(CArcFile* pclArc)
{
	if (MountNwa(pclArc) == TRUE)
		return TRUE;
	if (MountNwk(pclArc) == TRUE)
		return TRUE;
	if (MountOvk(pclArc) == TRUE)
		return TRUE;
	return FALSE;
}

// Function that gets the information of a *.nwa file.
BOOL CVA::MountNwa(CArcFile* pclArc)
{
	if (pclArc->GetArcExten() != _T(".nwa"))
		return FALSE;

	// Read nwa header
	NWAHed nwaHed;
	pclArc->Read(&nwaHed, sizeof(NWAHed));

	// Add to listview
	SFileInfo infFile;
	infFile.name = pclArc->GetArcName();
	infFile.sizeOrg = nwaHed.DataSize + 44;
	infFile.sizeCmp = (nwaHed.CompFileSize == 0) ? infFile.sizeOrg : nwaHed.CompFileSize;
	infFile.start = 0x00;
	infFile.end = infFile.start + infFile.sizeCmp;
	pclArc->AddFileInfo(infFile);

	return TRUE;
}

// Function that gets the information of a *.nwk file
BOOL CVA::MountNwk(CArcFile* pclArc)
{
	if (pclArc->GetArcExten() != _T(".nwk"))
		return FALSE;

	// Get file count
	DWORD ctFile;
	pclArc->Read(&ctFile, 4);

	// Number of files retrieved from the index
	DWORD index_size = ctFile * 12;

	// Get index
	YCMemory<BYTE> index(index_size);
	LPBYTE pIndex = &index[0];
	pclArc->Read(pIndex, index_size);

	// Get the base filename
	TCHAR szBaseFileName[_MAX_FNAME];
	lstrcpy(szBaseFileName, pclArc->GetArcName());
	PathRemoveExtension(szBaseFileName);

	for (int i = 0; i < (int)ctFile; i++)
	{
		// Get filename
		TCHAR szFileName[_MAX_FNAME];
		_stprintf(szFileName, _T("%s_%06d.nwa"), szBaseFileName, *(LPDWORD) &pIndex[8]);

		SFileInfo infFile;
		infFile.name = szFileName;
		infFile.sizeCmp = *(LPDWORD)&pIndex[0];
		//infFile.sizeOrg = infFile.sizeCmp;
		infFile.start = *(LPDWORD)&pIndex[4];
		infFile.end = infFile.start + infFile.sizeCmp;

		// Get filesize
		pclArc->Seek(infFile.start + 20, FILE_BEGIN);
		pclArc->Read(&infFile.sizeOrg, 4);
		infFile.sizeOrg += 44;

		pclArc->AddFileInfo(infFile);

		pIndex += 12;
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	Gets information from .ovk files

BOOL CVA::MountOvk(CArcFile* pclArc)
{
	if (pclArc->GetArcExten() != _T(".ovk"))
		return FALSE;

	// Get file count

	DWORD				dwFiles;

	pclArc->Read(&dwFiles, 4);

	// Get index size

	DWORD				dwIndexSize;

	dwIndexSize = dwFiles * 16;

	// Read index

	YCMemory<BYTE>		clmbtIndex(dwIndexSize);

	pclArc->Read(&clmbtIndex[0], dwIndexSize);

	// Get base filename

	TCHAR szBaseFileName[_MAX_FNAME];

	lstrcpy(szBaseFileName, pclArc->GetArcName());
	PathRemoveExtension(szBaseFileName);

	// Get file information

	for (DWORD i = 0; i < dwIndexSize; i += 16)
	{
		// Get filename

		TCHAR				szFileName[_MAX_FNAME];

		_stprintf(szFileName, _T("%s_%06d.ogg"), szBaseFileName, *(LPDWORD) &clmbtIndex[i + 8]);

		// Get file information

		SFileInfo			stfiWork;

		stfiWork.name = szFileName;
		stfiWork.sizeCmp = *(LPDWORD)&clmbtIndex[i];
		stfiWork.sizeOrg = stfiWork.sizeCmp;
		stfiWork.start = *(LPDWORD)&clmbtIndex[i + 4];
		stfiWork.end = stfiWork.start + stfiWork.sizeCmp;

		pclArc->AddFileInfo(stfiWork);
	}

	return TRUE;
}

inline int CVA::getbits(LPBYTE& data, int& shift, int bits)
{
	if (shift > 8)
	{
		data++;
		shift -= 8;
	}
	
	int ret = *(LPWORD)data >> shift;
	shift += bits;
	
	return ret & ((1 << bits) - 1); // mask
}

BOOL CVA::Decode(CArcFile* pclArc)
{
	if (DecodeNwa(pclArc) == TRUE)
		return TRUE;
	return FALSE;
}

// Function to convert to WAV for extraction
BOOL CVA::DecodeNwa(CArcFile* pclArc)
{
	SFileInfo* pInfFile = pclArc->GetOpenFileInfo();

	if (pInfFile->format != _T("NWA"))
		return FALSE;

	// Read nwa header
	NWAHed nwaHed;
	pclArc->Read(&nwaHed, sizeof(NWAHed));

	CWav wav;
	wav.Init(pclArc, nwaHed.DataSize, nwaHed.freq, nwaHed.channels, nwaHed.bits);

	// If no compression, just change the WAV header
	if (nwaHed.CmpLevel == 0xFFFFFFFF)
	{
		wav.Write();
	}
	else
	{
		// RLE compression

		BOOL				bRLE = FALSE;

		if( (nwaHed.CmpLevel == 5) && (nwaHed.channels != 2) )
		{
			// Uses RLE 

			bRLE = TRUE;
		}

		// Memory allocation of the offset
		DWORD offset_size = nwaHed.blocks * 4;
		YCMemory<DWORD> offsets(offset_size);
		// Allocate memory for writing data
		DWORD buf_len = nwaHed.BlockSize * (nwaHed.bits >> 3);
		YCMemory<BYTE> buf(buf_len);
		// Allocate memory for data loading
		DWORD z_buf_len = buf_len * 2;
		YCMemory<BYTE> z_buf(z_buf_len);

		// Read offset
		pclArc->Read(&offsets[0], offset_size);

		for (int i = 0; i < (int)nwaHed.blocks; i++)
		{
			LPBYTE z_pbuf = &z_buf[0];
			LPBYTE pbuf = &buf[0];

			// Get the size of the data to be read/decoded
			DWORD curblocksize, curcompsize;
			if (i < (int)nwaHed.blocks - 1)
			{
				curblocksize = buf_len;
				curcompsize = offsets[i + 1] - offsets[i];
			}
			else
			{
				curblocksize = nwaHed.RestSize * (nwaHed.bits >> 3);
				curcompsize = z_buf_len;
			}

			// Ensure buffers exist

			BYTE*				pbtSrcEnd = z_pbuf + curcompsize;
			BYTE*				pbtDstEnd = pbuf + curblocksize;

			// Read data
			pclArc->Read(z_pbuf, curcompsize);

			int d[2];
			int shift = 0;

			// Read first data
			if (nwaHed.bits == 8)
			{
				d[0] = *z_pbuf++;
			}
			else // fNWA.bits == 16
			{
				d[0] = *(LPWORD)z_pbuf;
				z_pbuf += 2;
			}

			if (nwaHed.channels == 2)
			{
				if (nwaHed.bits == 8)
				{
					d[1] = *z_pbuf++;
				}
				else  // fNWA.bits == 16
				{
					d[1] = *(LPWORD)z_pbuf;
					z_pbuf += 2;
				}
			}

			int dsize = curblocksize / (nwaHed.bits >> 3);
			BOOL bFlip = 0; // Stereo

			for (int j = 0; j < dsize; j++)
			{
				if( (z_pbuf >= pbtSrcEnd) || (pbuf >= pbtDstEnd) )
				{
					// Exit

					break;
				}

				int type = getbits(z_pbuf, shift, 3);

				// 7 : Large difference
				if (type == 7)
				{
					if (getbits(z_pbuf, shift, 1) == 1)
					{
						d[bFlip] = 0; // Unused
					}
					else
					{
						int BITS, SHIFT;
						if (nwaHed.CmpLevel >= 3)
						{
							BITS = 8;
							SHIFT = 9;
						}
						else
						{
							BITS = 8 - nwaHed.CmpLevel;
							SHIFT = 2 + type + nwaHed.CmpLevel;
						}
						
						const int MASK1 = (1 << (BITS - 1));
						const int MASK2 = (1 << (BITS - 1)) - 1;
						int b = getbits(z_pbuf, shift, BITS);
						if (b & MASK1)
							d[bFlip] -= (b & MASK2) << SHIFT;
						else
							d[bFlip] += (b & MASK2) << SHIFT;
					}
				}

				// 1-6 : Ordinary differential
				else if (type != 0)
				{
					int BITS, SHIFT;
					if (nwaHed.CmpLevel >= 3)
					{
						BITS = 3 + nwaHed.CmpLevel;
						SHIFT = 1 + type;
					}
					else
					{
						BITS = 5 - nwaHed.CmpLevel;
						SHIFT = 2 + type + nwaHed.CmpLevel;
					}
					
					const int MASK1 = (1 << (BITS - 1));
					const int MASK2 = (1 << (BITS - 1)) - 1;
					int b = getbits(z_pbuf, shift, BITS);
					if (b & MASK1)
						d[bFlip] -= (b & MASK2) << SHIFT;
					else
						d[bFlip] += (b & MASK2) << SHIFT;
				}

				// type == 0
				else
				{
					if( bRLE )
					{
						// Run-length compression 

						int					nRunLength = getbits(z_pbuf, shift, 1);

						if( nRunLength == 1 )
						{
							nRunLength = getbits(z_pbuf, shift, 2);

							if( nRunLength == 3 )
							{
								nRunLength = getbits(z_pbuf, shift, 8);
							}
						}

						// Going to write the same data as the previous data

						for( int k = 0 ; k <= nRunLength ; k++ )
						{
							if( nwaHed.bits == 8 )
							{
								*pbuf++ = (BYTE)d[bFlip];
							}
							else
							{
								*(LPWORD)pbuf = d[bFlip];
								pbuf += 2;
							}

							if (nwaHed.channels == 2)
							{
								bFlip ^= 1;	// channel switching
							}
						}

						j += nRunLength;
						continue;
					}
				}

				if( nwaHed.bits == 8 )
				{
					*pbuf++ = (BYTE)d[bFlip];
				}
				else
				{
					*(LPWORD)pbuf = d[bFlip];
					pbuf += 2;
				}

				if( nwaHed.channels == 2 )
				{
					bFlip ^= 1;	// channel switching
				}
			}

			wav.Write(&buf[0], curblocksize);
		}
	}

	return TRUE;
}