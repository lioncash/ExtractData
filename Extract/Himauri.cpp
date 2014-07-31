#include "stdafx.h"
#include "../ExtractBase.h"
#include "../Image.h"
#include "../Image/Tga.h"
#include "Himauri.h"

//////////////////////////////////////////////////////////////////////////////////////////
// Mount

BOOL CHimauri::Mount(
	CArcFile*			pclArc							// Archive
	)
{
	if( pclArc->GetArcExten() != _T(".hxp") )
	{
		return FALSE;
	}

	if( MountHim4( pclArc) )
	{
		return TRUE;
	}

	if( MountHim5( pclArc ) )
	{
		return TRUE;
	}

	return FALSE;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Mount Him4

BOOL CHimauri::MountHim4(
	CArcFile*			pclArc							// Archive
	)
{
	if( memcmp( pclArc->GetHed(), "Him4", 4 ) != 0 )
	{
		return FALSE;
	}

	// Get file count
	DWORD dwFiles;
	pclArc->SeekHed( 4 );
	pclArc->Read( &dwFiles, 4 );

	// Get index size from file count
	DWORD dwIndexSize = dwFiles * 4;

	// Get index
	YCMemory<BYTE> clmbtIndex( dwIndexSize );
	DWORD dwIndexPtr = 0;
	pclArc->Read( &clmbtIndex[0], dwIndexSize );
	TCHAR szArcName[_MAX_FNAME];

	lstrcpy( szArcName, pclArc->GetArcName() );
	PathRemoveExtension( szArcName );

	for( DWORD i = 0 ; i < dwFiles ; i++ )
	{
		TCHAR szFileName[_MAX_FNAME];
		_stprintf( szFileName, _T("%s_%06u"), szArcName, i );

		// Add to list view
		SFileInfo stFileInfo;
		stFileInfo.name = szFileName;
		stFileInfo.start = *(LPDWORD) &clmbtIndex[dwIndexPtr + 0];
		stFileInfo.end = ((i + 1) < dwFiles) ? *(DWORD*) &clmbtIndex[dwIndexPtr + 4] : pclArc->GetArcSize();
		stFileInfo.sizeCmp = stFileInfo.end - stFileInfo.start;
		stFileInfo.sizeOrg = stFileInfo.sizeCmp;

		if( stFileInfo.sizeCmp != 10 )
		{
			// Is not a dummy file

			pclArc->AddFileInfo( stFileInfo );
		}

		dwIndexPtr += 4;
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Mount Him5

BOOL CHimauri::MountHim5(
	CArcFile*			pclArc							// Archive
	)
{
	if( memcmp( pclArc->GetHed(), "Him5", 4 ) != 0 )
	{
		return FALSE;
	}

	// Get file count
	DWORD dwFiles;
	pclArc->SeekHed( 4 );
	pclArc->Read( &dwFiles, 4 );

	// Get index size from file count
	DWORD dwIndexSize = dwFiles * 8;

	// Get index
	YCMemory<BYTE> clmbtIndex( dwIndexSize );
	DWORD         dwIndexPtr = 0;
	pclArc->Read( &clmbtIndex[0], dwIndexSize );

	// Gets index 2's size from the first index
	DWORD dwIndexSize2 = *(DWORD*) &clmbtIndex[dwFiles * 8 - 4] - *(DWORD*) &clmbtIndex[4] + *(DWORD*) &clmbtIndex[dwFiles * 8 - 8];

	// Get index 2
	YCMemory<BYTE> clmbtIndex2( dwIndexSize2 );
	DWORD          dwIndexPtr2 = 0;
	pclArc->Read( &clmbtIndex2[0], dwIndexSize2 );

	// Remove dummy
	for( DWORD i = dwFiles ; i > 0 ; i-- )
	{
		if( *(DWORD*) &clmbtIndex[dwIndexPtr + 0] == 0 )
		{
			dwFiles--;
		}
	}

	// Gets difference increment
	BOOL bDiffCompose = (pclArc->GetArcName() == _T("natsucha.hxp"));

	// Gets file info
	std::vector<SFileInfo> vcFileInfo;
	std::vector<SFileInfo> vcDiffFileInfo;

	for( DWORD i = 0 ; i < dwFiles ; i++ )
	{
		while( 1 )
		{
			// Gets the length of a segment
			DWORD dwSegLength = clmbtIndex2[dwIndexPtr2 + 0];

			// Get the file name from index 2
			DWORD   dwFileNameLength = (dwSegLength - 5);
			char    szFileName[_MAX_FNAME];
			memcpy( szFileName, &clmbtIndex2[dwIndexPtr2 + 5], dwFileNameLength );
			SFileInfo stFileInfo;

			stFileInfo.name = szFileName;
			stFileInfo.start = pclArc->ConvEndian( *(DWORD*) &clmbtIndex2[dwIndexPtr2 + 1] );

			// Get the exit address

			if( clmbtIndex2[dwIndexPtr2 + dwSegLength] == 0 )
			{
				if( (i + 1) == dwFiles )
				{
					stFileInfo.end = pclArc->GetArcSize();
				}
				else
				{
					stFileInfo.end = pclArc->ConvEndian( *(DWORD*) &clmbtIndex2[dwIndexPtr2 + dwSegLength + 2] );
				}
			}
			else
			{
				stFileInfo.end = pclArc->ConvEndian( *(DWORD*) &clmbtIndex2[dwIndexPtr2 + dwSegLength + 1] );
			}

			stFileInfo.sizeCmp = stFileInfo.end - stFileInfo.start;
			stFileInfo.sizeOrg = stFileInfo.sizeCmp;

			if( stFileInfo.sizeCmp != 10 )
			{
				// Is not a dummy file
				if( bDiffCompose )
				{
					// Difference composition is enabled
					char* pszDiffMark = strrchr( szFileName, _T('_') );

					if( (pszDiffMark != NULL) && (strlen( pszDiffMark ) >= 2) && isdigit( pszDiffMark[1] ) )
					{
						// Difference file

						int nPos = stFileInfo.name.ReverseFind( _T('_') );
						stFileInfo.name.Delete( (nPos + 1), stFileInfo.name.GetLength() );
						vcDiffFileInfo.push_back( stFileInfo );
					}
					else
					{
						vcFileInfo.push_back( stFileInfo );
					}
				}
				else
				{
					pclArc->AddFileInfo( stFileInfo );
				}
			}

			// 1セグメントの最後が0なら1ファイル終了と判断

			if( clmbtIndex2[dwIndexPtr2 + dwSegLength] == 0 )
			{
				dwIndexPtr2 += dwSegLength + 1;
				break;
			}

			dwIndexPtr2 += dwSegLength;
		}
	}

	if( bDiffCompose )
	{
		// Difference composition is enabled
		for( size_t i = 0 ; i < vcFileInfo.size() ; i++ )
		{
			int nPos = vcFileInfo[i].name.ReverseFind( _T('_') );
			if( nPos >= 0 )
			{
				YCString clsBaseFileName = vcFileInfo[i].name.Left( (nPos + 1) );

				for( size_t j = 0 ; j < vcDiffFileInfo.size() ; j++ )
				{
					if( clsBaseFileName == vcDiffFileInfo[j].name )
					{
						// Within the difference range

						vcFileInfo[i].starts.push_back( vcDiffFileInfo[j].start );
						vcFileInfo[i].sizesCmp.push_back( vcDiffFileInfo[j].sizeCmp );
						vcFileInfo[i].sizesOrg.push_back( vcDiffFileInfo[j].sizeOrg );
					}
				}
			}

			pclArc->AddFileInfo( vcFileInfo[i] );
		}
	}

	return	TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Decode

BOOL CHimauri::Decode(
	CArcFile*			pclArc							// Archive
	)
{
	if( pclArc->GetArcExten() != _T(".hxp") )
	{
		return FALSE;
	}

	if( memcmp( pclArc->GetHed(), "Him", 3 ) != 0 )
	{
		return FALSE;
	}

	// Get input size
	DWORD dwSrcSize;
	pclArc->Read( &dwSrcSize, 4 );

	// Get output size
	DWORD dwDstSize;
	pclArc->Read( &dwDstSize, 4 );

	// Ensure buffer
	YCMemory<BYTE> clmbtSrc( dwSrcSize );
	YCMemory<BYTE> clmbtDst( dwDstSize );

	if( dwSrcSize == 0 )
	{
		// Not a compressed file

		pclArc->Read( &clmbtDst[0], dwDstSize );
	}
	else
	{
		// Compressed file

		pclArc->Read( &clmbtSrc[0], dwSrcSize );
		Decomp( &clmbtDst[0], dwDstSize, &clmbtSrc[0], dwSrcSize );
	}

	YCString sFileExt;
	if( memcmp( &clmbtDst[0], "OggS", 4 ) == 0 )
	{
		// Ogg Vorbis

		sFileExt = _T(".ogg");
	}
	else if( memcmp( &clmbtDst[0], "RIFF", 4 ) == 0 )
	{
		// WAVE

		sFileExt = _T(".wav");
	}
	else if( memcmp( &clmbtDst[0], "Himauri", 7 ) == 0 )
	{
		// hst

		sFileExt = _T(".txt");
	}
	else if( memcmp( &clmbtDst[0], "BM", 2 ) == 0 )
	{
		// BMP

		CImage clImage;
		clImage.Init( pclArc, &clmbtDst[0] );
		clImage.Write( dwDstSize );

		return TRUE;
	}
	else if( (clmbtDst[0] == 0) && (clmbtDst[1] <= 1) && (clmbtDst[2] > 0) && (memcmp( &clmbtDst[3], "\x00\x00\x00\x00\x00", 5 ) == 0) )
	{
		// TGA

		SFileInfo* pstFileInfo = pclArc->GetOpenFileInfo();

		if( pstFileInfo->starts.empty() )
		{
			// Difference does not exist

			CTga clTga;
			clTga.Decode( pclArc, &clmbtDst[0], dwDstSize );
		}
		else
		{
			// There is a difference

			// Get TGA image-based header
			CTga::STGAHeader* psttgahBase = (CTga::STGAHeader*) &clmbtDst[0];

			// TGA Decompression
			DWORD          dwDstSize2 = ((psttgahBase->wWidth * (psttgahBase->btDepth >> 3) + 3) & 0xFFFFFFFC) * psttgahBase->wHeight;
			YCMemory<BYTE> clmbtDst2( dwDstSize2 );

			CTga clTga;
			clTga.Decomp( &clmbtDst2[0], dwDstSize2, &clmbtDst[0], dwDstSize );

			// Outputs difference
			for( size_t i = 0 ; i < pstFileInfo->starts.size() ; i++ )
			{
				pclArc->SeekHed( pstFileInfo->starts[i] );

				// Get image input size difference
				DWORD dwSrcSizeForDiff;
				pclArc->Read( &dwSrcSizeForDiff, 4 );

				// Get image output size difference
				DWORD dwDstSizeForDiff;
				pclArc->Read( &dwDstSizeForDiff, 4 );

				// Ensure image difference buffer
				YCMemory<BYTE> clmbtSrcForDiff( dwSrcSizeForDiff );
				YCMemory<BYTE> clmbtDstForDiff( dwDstSizeForDiff );

				// Get image difference
				if( dwSrcSizeForDiff == 0 )
				{
					// Uncompressed file

					pclArc->Read( &clmbtDstForDiff[0], dwDstSizeForDiff );
				}
				else
				{
					// Compressed file

					pclArc->Read( &clmbtSrcForDiff[0], dwSrcSizeForDiff );
					Decomp( &clmbtDstForDiff[0], dwDstSizeForDiff, &clmbtSrcForDiff[0], dwSrcSizeForDiff );
				}

				// Get TGA image header difference
				CTga::STGAHeader* psttgahDiff = (CTga::STGAHeader*) &clmbtDstForDiff[0];

				// TGA Decompression
				DWORD          dwDstSizeForDiff2 = ((psttgahDiff->wWidth * (psttgahDiff->btDepth >> 3) + 3) & 0xFFFFFFFC) * psttgahDiff->wHeight;
				YCMemory<BYTE> clmbtDstForDiff2( dwDstSizeForDiff );
				clTga.Decomp( &clmbtDstForDiff2[0], dwDstSizeForDiff, &clmbtDstForDiff[0], dwDstSizeForDiff );

				// Difference Composition
				DWORD          dwDstSizeForCompose = dwDstSize2;
				YCMemory<BYTE> clmbtDstForCompose( dwDstSizeForCompose );
				Compose( &clmbtDstForCompose[0], dwDstSizeForCompose, &clmbtDst2[0], dwDstSize2, &clmbtDstForDiff2[0], dwDstSizeForDiff2 );

				// End of filename changes
				TCHAR      szLastName[_MAX_FNAME];
				_stprintf( szLastName, _T("_%03u.bmp"), i );

				// Regrest progressbar progress
				BOOL bProgress = (i == 0);

				// Output
				CImage clImage;
				clImage.Init( pclArc, psttgahBase->wWidth, psttgahBase->wHeight, psttgahBase->btDepth, NULL, 0, szLastName );
				clImage.Write( &clmbtDstForCompose[0], dwDstSizeForCompose, bProgress );
				clImage.Close();
			}
		}

		return TRUE;
	}
	else if( memcmp( &clmbtDst[0], "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 16 ) == 0 )
	{
		// Mask image
		CImage clImage;
		clImage.Init( pclArc, 800, 600, 8 );
		clImage.WriteReverse( &clmbtDst[0], dwDstSize );

		return TRUE;
	}
	else
	{
		// Other

		sFileExt = _T(".txt");
	}

	pclArc->OpenFile( sFileExt );
	pclArc->WriteFile( &clmbtDst[0], dwDstSize, pclArc->GetOpenFileInfo()->sizeCmp );

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Decompression

void CHimauri::Decomp(
	BYTE*				pbtDst,							// Destination
	DWORD				dwDstSize,						// Destination Size
	const BYTE*			pbtSrc,							// Compressed Data
	DWORD				dwSrcSize						// Compressed Data Size
	)
{
	DWORD dwSrcPtr = 0;
	DWORD dwDstPtr = 0;
	DWORD dwCode = 0;
	DWORD dwBack = 0;

	while( (dwSrcPtr < dwSrcSize) && (dwDstPtr < dwDstSize) )
	{
		if( dwCode == 0 )
		{
			dwCode = pbtSrc[dwSrcPtr++];

			if( dwCode < 0x20 )
			{
				dwBack = 0;

				if( dwCode < 0x1D )
				{
					dwCode++;
				}
				else if( dwCode == 0x1D )
				{
					dwCode = pbtSrc[dwSrcPtr++] + 0x1E;
				}
				else if( dwCode == 0x1E )
				{
					dwCode = pbtSrc[dwSrcPtr++];
					dwCode <<= 8;
					dwCode |= pbtSrc[dwSrcPtr++];
					dwCode += 0x11E;
				}
				else if (dwCode == 0x1F)
				{
					dwCode = pbtSrc[dwSrcPtr++];
					dwCode <<= 8;
					dwCode |= pbtSrc[dwSrcPtr++];
					dwCode <<= 8;
					dwCode |= pbtSrc[dwSrcPtr++];
					dwCode <<= 8;
					dwCode |= pbtSrc[dwSrcPtr++];
				}
			}
			else
			{
				if( dwCode >= 0x80 )
				{
					dwBack = ((dwCode & 0x1F) << 8) | pbtSrc[dwSrcPtr++];
					dwCode = (dwCode >> 5) & 3;
				}
				else
				{
					DWORD dwCode2 = (dwCode & 0x60);

					if( dwCode2 == 0x20 )
					{
						dwBack = (dwCode >> 2) & 7;
						dwCode &= 3;
					}
					else
					{
						dwCode &= 0x1F;

						if( dwCode2 == 0x40 )
						{
							dwBack = pbtSrc[dwSrcPtr++];
							dwCode += 4;
						}
						else
						{
							dwBack = (dwCode << 8) | pbtSrc[dwSrcPtr++];
							dwCode = pbtSrc[dwSrcPtr++];

							if( dwCode == 0xFE )
							{
								dwCode = pbtSrc[dwSrcPtr++];
								dwCode <<= 8;
								dwCode |= pbtSrc[dwSrcPtr++];
								dwCode += 0x102;
							}
							else if( dwCode == 0xFF )
							{
								dwCode = pbtSrc[dwSrcPtr++];
								dwCode <<= 8;
								dwCode |= pbtSrc[dwSrcPtr++];
								dwCode <<= 8;
								dwCode |= pbtSrc[dwSrcPtr++];
								dwCode <<= 8;
								dwCode |= pbtSrc[dwSrcPtr++];
							}
							else
							{
								dwCode += 4;
							}
						}
					}
				}

				dwBack++;
				dwCode += 3;
			}
		}

		// Get output length
		DWORD dwLength = dwCode;
		if( (dwDstPtr + dwLength) > dwDstSize )
		{
			dwLength = (dwDstSize - dwDstPtr);
		}

		dwCode -= dwLength;

		// Output
		if( dwBack > 0 )
		{
			// Output previous data

			for( DWORD i = 0 ; i < dwLength ; i++ )
			{
				pbtDst[dwDstPtr + i] = pbtDst[dwDstPtr - dwBack + i];
			}

			dwDstPtr += dwLength;
		}
		else
		{
			// Output input data

			memcpy( &pbtDst[dwDstPtr], &pbtSrc[dwSrcPtr], dwLength );

			dwSrcPtr += dwLength;
			dwDstPtr += dwLength;
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////
// Difference Composition

BOOL CHimauri::Compose(
	BYTE*				pbtDst,							// Destination
	DWORD				dwDstSize,						// Destination Size
	const BYTE*			pbtBase,						// Image base
	DWORD				dwBaseSize,						// Image base size
	const BYTE*			pbtDiff,						// Image difference
	DWORD				dwDiffSize						// Image difference size
	)
{
	// Synthesize base image and difference image

	memcpy( pbtDst, pbtBase, dwDstSize );

	for( DWORD i = 0 ; i < dwDstSize ; i += 4 )
	{
		// 32bit -> 24bit

		if( pbtDiff[i + 3] > 0 )
		{
			for( int j = 0 ; j < 3 ; j++ )
			{
				pbtDst[i + j] = (pbtDiff[i + j] - pbtBase[i + j]) * pbtDiff[i + 3] / 255 + pbtBase[i + j];
			}

			pbtDst[i + 3] = 0xFF;
		}
	}

	return TRUE;
}
