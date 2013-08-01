#include "stdafx.h"
#include "../../ExtractBase.h"
#include "../../Arc/Zlib.h"
#include "../../Image.h"
#include "../../Sound/Ogg.h"
#include "../../FindFile.h"
#include "Tlg.h"
#include "Krkr.h"

//////////////////////////////////////////////////////////////////////////////////////////
//  Mount

BOOL	CKrkr::Mount(
	CArcFile*			pclArc							// Archive
	)
{
	DWORD				dwOffset;

	if( memcmp( pclArc->GetHed(), "XP3\r\n \n\x1A\x8B\x67\x01", 11 ) == 0 )
	{
		// XP3

		dwOffset = 0;
	}
	else if( memcmp( pclArc->GetHed(), "MZ", 2 ) == 0 )
	{
		// EXE Type

		if( !FindXP3FromExecuteFile( pclArc, &dwOffset ) )
		{
			return	FALSE;
		}
	}
	else
	{
		return	FALSE;
	}

	m_pclArc = pclArc;

	// Set MD5 value for tpm

	SetMD5ForTpm( pclArc );

	// Check if archive can be decoded

	if( !OnCheckDecrypt( pclArc ) )
	{
		// Archive can not be decoded

		return	FALSE;
	}

	// Get index position

	INT64				n64IndexPos;

	pclArc->SeekHed( 11 + dwOffset );
	pclArc->Read( &n64IndexPos, 8 );

	pclArc->SeekCur( n64IndexPos - 19 );

	BYTE				abtWork[256];

	pclArc->Read( abtWork, sizeof( abtWork ) );

	switch( abtWork[0] )
	{
	case	0x80:

		n64IndexPos = *(INT64*) &abtWork[9];
		break;
	}

	// Read the index header

	BYTE				btCmpIndex;

	pclArc->SeekHed( n64IndexPos + dwOffset );
	pclArc->Read( &btCmpIndex, 1 );

	UINT64				u64CompIndexSize;
	UINT64				u64IndexSize;

	if( btCmpIndex )
	{
		// Index has been compressed

		pclArc->Read( &u64CompIndexSize, 8 );
	}

	pclArc->Read( &u64IndexSize, 8 );

	// Ensure buffer

	YCMemory<BYTE>		clmbtIndex( u64IndexSize );
	DWORD				dwIndexPtr = 0;

	// If the index header is compressed, decompress it

	if( btCmpIndex )
	{
		// Index has been compressed

		CZlib				clZlib;

		// Ensure buffer

		YCMemory<BYTE>		clmbtCompIndex( u64CompIndexSize );

		// zlib Decompression

		pclArc->Read( &clmbtCompIndex[0], u64CompIndexSize );

		clZlib.Decompress( &clmbtIndex[0], u64IndexSize, &clmbtCompIndex[0], u64CompIndexSize );
	}
	else
	{
		// Index is not compressed

		pclArc->Read( &clmbtIndex[0], u64IndexSize );
	}

	// Get index file information

	for( UINT64 i = 0 ; i < u64IndexSize ; )
	{
		// "File" Chunk

		FileChunk			stFileChunk;

		memcpy( stFileChunk.name, &clmbtIndex[i], 4 );

		stFileChunk.size = *(UINT64*) &clmbtIndex[i + 4];

		if( memcmp(stFileChunk.name, "File", 4 ) != 0 )
		{
			break;
		}

		i += 12;

		// "info" Chunk

		InfoChunk			stInfoChunk;

		memcpy( stInfoChunk.name, &clmbtIndex[i], 4 );

		stInfoChunk.size = *(UINT64*) &clmbtIndex[i + 4];
		stInfoChunk.protect = *(DWORD*) &clmbtIndex[i + 12];
		stInfoChunk.orgSize = *(UINT64*) &clmbtIndex[i + 16];
		stInfoChunk.arcSize = *(UINT64*) &clmbtIndex[i + 24];
		stInfoChunk.nameLen = *(WORD*) &clmbtIndex[i + 32];
		stInfoChunk.filename = (wchar_t*) &clmbtIndex[i + 34];

		if( memcmp( stInfoChunk.name, "info", 4 ) != 0 )
		{
			break;
		}

		i += 12 + stInfoChunk.size;

		// "segm" Chunk

		SegmChunk			stSegmChunk;

		memcpy( stSegmChunk.name, &clmbtIndex[i], 4 );

		stSegmChunk.size = *(UINT64*) &clmbtIndex[i + 4];

		if( memcmp( stSegmChunk.name, "segm", 4 ) != 0 )
		{
			break;
		}

		i += 12;

		SFileInfo			stFileInfo;

		UINT64				u64SegmCount = (stSegmChunk.size / 28);

		for( UINT64 j = 0 ; j < u64SegmCount ; j++ )
		{
			stSegmChunk.comp = *(DWORD*) &clmbtIndex[i];
			stSegmChunk.start = *(UINT64*) &clmbtIndex[i + 4] + dwOffset;
			stSegmChunk.orgSize = *(UINT64*) &clmbtIndex[i + 12];
			stSegmChunk.arcSize = *(UINT64*) &clmbtIndex[i + 20];

			stFileInfo.bCmps.push_back( stSegmChunk.comp );
			stFileInfo.starts.push_back( stSegmChunk.start );
			stFileInfo.sizesOrg.push_back( stSegmChunk.orgSize );
			stFileInfo.sizesCmp.push_back( stSegmChunk.arcSize );

			i += 28;
		}

		// Check for any other chunks

		UINT64				u64Remainder = stFileChunk.size - 12 - stInfoChunk.size - 12 - stSegmChunk.size;

		if( u64Remainder > 0 )
		{
			// "adlr" Chunk

			if( memcmp( &clmbtIndex[i], "adlr", 4 ) == 0 )
			{
				AdlrChunk			stAdlrChunk;

				memcpy( stAdlrChunk.name, &clmbtIndex[i], 4 );

				stAdlrChunk.size = *(UINT64*) &clmbtIndex[i + 4];
				stAdlrChunk.key = *(DWORD*) &clmbtIndex[i + 12];

				stFileInfo.key = stAdlrChunk.key;
			}

			i += u64Remainder;
		}

		// Store and show the stucture in a listview

		stFileInfo.name.Copy( stInfoChunk.filename, stInfoChunk.nameLen );
		stFileInfo.sizeOrg = stInfoChunk.orgSize;
		stFileInfo.sizeCmp = stInfoChunk.arcSize;
		stFileInfo.start = stFileInfo.starts[0];
		stFileInfo.end = stFileInfo.start + stFileInfo.sizeCmp;

		if( stSegmChunk.comp )
		{
			stFileInfo.format = _T("zlib");
		}

		pclArc->AddFileInfo( stFileInfo );
	}

	return	TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////////
//  Decode

BOOL	CKrkr::Decode(
	CArcFile*			pclArc							// Archive
	)
{
	SFileInfo*			pstFileInfo = pclArc->GetOpenFileInfo();

	if( (pclArc->GetArcExten() != _T(".xp3")) && (pclArc->GetArcExten() != _T(".exe")) )
	{
		return	FALSE;
	}

	YCString			clsFileExt = PathFindExtension( pstFileInfo->name );

	clsFileExt.MakeLower();

	InitDecrypt( pclArc );

//char s[256];
//_stprintf(s, "%08X", pInfFile->key);
//MessageBox(NULL, s, "", 0);

	// Ensure buffer

	DWORD				dwBufferSize = pclArc->GetBufSize();

	YCMemory<BYTE>		clmbtBuffer;

	// Whether or not it's bound to memory

	BOOL				bComposeMemory = FALSE;

	if( (clsFileExt == _T(".tlg")) ||
		((clsFileExt == _T(".ogg")) && pclArc->GetOpt()->bFixOgg) ||
		(clsFileExt == _T(".bmp")) )
	{
		// TLG, OGG(Fix CRC), BMP

		clmbtBuffer.resize( pstFileInfo->sizeOrg + 3 );
		bComposeMemory = TRUE;
	}
	else if( (m_dwDecryptKey == 0) &&
		pclArc->GetOpt()->bEasyDecrypt && (
		(clsFileExt == _T(".tjs")) ||
		(clsFileExt == _T(".ks")) ||
		(clsFileExt == _T(".asd")) ||
		(clsFileExt == _T(".txt"))) )
	{
		// tjs, ks, asd, txt

		clmbtBuffer.resize( pstFileInfo->sizeOrg + 3 );
		bComposeMemory = TRUE;
	}
	else
	{
		// Other

		clmbtBuffer.resize( dwBufferSize + 3 );
	}

	// Create output file

	if( !bComposeMemory )
	{
		pclArc->OpenFile();
	}

	DWORD				dwBufferPtr = 0;
	DWORD				dwBufferSizeBase = dwBufferSize;
	DWORD				dwWroteSize = 0;

	for( size_t i = 0 ; i < pstFileInfo->starts.size() ; i++ )
	{
		dwBufferSize = dwBufferSizeBase;

		pclArc->SeekHed( pstFileInfo->starts[i] );

		if( pstFileInfo->bCmps[i] )
		{
			// Compressed data

			CZlib				clZlib;

			// Ensure buffer

			DWORD				dwSrcSize = pstFileInfo->sizesCmp[i];

			YCMemory<BYTE>		clmbtSrc( dwSrcSize );

			DWORD				dwDstSize = pstFileInfo->sizesOrg[i];

			YCMemory<BYTE>		clmbtDst( dwDstSize + 3 );

			// zlib Decompression

			pclArc->Read( &clmbtSrc[0], dwSrcSize );

			clZlib.Decompress( &clmbtDst[0], dwDstSize, &clmbtSrc[0], dwSrcSize );

			DWORD				dwDataSize = Decrypt( &clmbtDst[0], dwDstSize, dwWroteSize );

			if( bComposeMemory )
			{
				// Bound to the buffer

				memcpy( &clmbtBuffer[dwBufferPtr], &clmbtDst[0], dwDataSize );

				dwBufferPtr += dwDataSize;
			}
			else
			{
				// Output

				pclArc->WriteFile( &clmbtDst[0], dwDataSize, dwDstSize );
			}

			dwWroteSize += dwDstSize;
		}
		else
		{
			// Uncompressed data

			if( bComposeMemory )
			{
				// Bound to the buffer

				DWORD				dwDstSize = pstFileInfo->sizesOrg[i];

				pclArc->Read( &clmbtBuffer[dwBufferPtr], dwDstSize );

				DWORD				dwDataSize = Decrypt( &clmbtBuffer[dwBufferPtr], dwDstSize, dwWroteSize );

				dwBufferPtr += dwDataSize;
				dwWroteSize += dwDstSize;
			}
			else
			{
				DWORD				dwDstSize = pstFileInfo->sizesOrg[i];

				for( DWORD dwWroteSizes = 0 ; dwWroteSizes != dwDstSize ; dwWroteSizes += dwBufferSize )
				{
					// Adjust buffer size

					pclArc->SetBufSize( &dwBufferSize, dwWroteSizes, dwDstSize );

					pclArc->Read( &clmbtBuffer[0], dwBufferSize );

					DWORD				dwDataSize = Decrypt( &clmbtBuffer[0], dwBufferSize, dwWroteSize );

					pclArc->WriteFile( &clmbtBuffer[0], dwDataSize );

					dwWroteSize += dwBufferSize;
				}
			}
		}
	}

	if( clsFileExt == _T(".tlg") )
	{
		// Convert tlg to bmp

		CTlg				clTLG;

		clTLG.Decode( pclArc, &clmbtBuffer[0] );
	}
	else if( clsFileExt == _T(".ogg") && pclArc->GetOpt()->bFixOgg )
	{
		// Fix CRC of OGG files

		COgg				clOGG;

		clOGG.Decode( pclArc, &clmbtBuffer[0] );
	}
	else if( clsFileExt == _T(".bmp") )
	{
		// bmp output (png conversion)

		CImage				clImage;

		clImage.Init( pclArc, &clmbtBuffer[0] );
		clImage.Write( pstFileInfo->sizeOrg );
	}
	else if( m_dwDecryptKey == 0 &&
		pclArc->GetOpt()->bEasyDecrypt && (
		(clsFileExt == _T(".tjs")) ||
		(clsFileExt == _T(".ks")) ||
		(clsFileExt == _T(".asd")) ||
		(clsFileExt == _T(".txt"))) )
	{
		// Text file

		DWORD				dwDstSize = pstFileInfo->sizeOrg;

		SetDecryptRequirement( TRUE );

		m_dwDecryptKey = pclArc->InitDecryptForText( &clmbtBuffer[0], dwDstSize );

		DWORD				dwDataSize = Decrypt( &clmbtBuffer[0], dwDstSize, 0 );

		pclArc->OpenFile();
		pclArc->WriteFile( &clmbtBuffer[0], dwDataSize, dwDstSize );
	}

	return	TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////////
//  Extraction

BOOL	CKrkr::Extract(
	CArcFile*			pclArc							// Archive
	)
{
	SFileInfo*			pstFileInfo = pclArc->GetOpenFileInfo();

	DWORD				dwBufferSize = pclArc->GetBufSize();
	DWORD				dwBufferSizeBase = dwBufferSize;

	YCMemory<BYTE>		clmbtBuffer( dwBufferSize );

	pclArc->OpenFile();

	for( size_t i = 0 ; i < pstFileInfo->starts.size() ; i++ )
	{
		dwBufferSize = dwBufferSizeBase;

		pclArc->SeekHed( pstFileInfo->starts[i] );

		DWORD				dwDstSize = pstFileInfo->sizesOrg[i];

		for( DWORD dwWroteSizes = 0 ; dwWroteSizes != dwDstSize ; dwWroteSizes += dwBufferSize )
		{
			// Adjust buffer size

			pclArc->SetBufSize( &dwBufferSize, dwWroteSizes, dwDstSize );

			pclArc->Read( &clmbtBuffer[0], dwBufferSize );
			pclArc->WriteFile( &clmbtBuffer[0], dwBufferSize );
		}
	}

	pclArc->CloseFile();

	return	TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////////
//  Set MD5 value for tpm in the archive folder

void	CKrkr::SetMD5ForTpm(
	CArcFile*			pclArc							// Archive
	)
{
	if( pclArc->CheckMD5OfSet() )
	{
		// If it has already been set

		return;
	}

	// Get directory path to the archive

	TCHAR				szBasePathToTpm[MAX_PATH];

	lstrcpy( szBasePathToTpm, pclArc->GetArcPath() );
	PathRemoveFileSpec( szBasePathToTpm );

	// Get the tpm file path

	CFindFile				clFindFile;

	std::vector<YCString>&	vtsPathToTpm = clFindFile.DoFind( szBasePathToTpm, _T("*.tpm") );

	// Set the tpm MD5 value

	CMD5				clmd5Tpm;

	for( size_t i = 0 ; i < vtsPathToTpm.size() ; i++)
	{
		pclArc->SetMD5( clmd5Tpm.Calculate( vtsPathToTpm[i] ) );
	}

	pclArc->SetMD5OfFlag( TRUE );
}

//////////////////////////////////////////////////////////////////////////////////////////
//  Determine if the archive is decodable

BOOL	CKrkr::OnCheckDecrypt(
	CArcFile*			pclArc							// Archive
	)
{
	return	TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////////
//  Verifies that the MD5 value of tpm in the archive folder matches

BOOL	CKrkr::CheckTpm(
	const char*			pszMD5							// MD5
	)
{
	// Comparison

	for( size_t i = 0 ; i < m_pclArc->GetMD5().size() ; i++ )
	{
		if( memcmp( pszMD5, m_pclArc->GetMD5()[i].szABCD, 32 ) == 0 )
		{
			// Matches

			return	TRUE;
		}
	}

	return	FALSE;
}

//////////////////////////////////////////////////////////////////////////////////////////
//  Initialize decryption process

void	CKrkr::InitDecrypt(
	CArcFile*			pclArc							// Archive
	)
{
	m_pclArc = pclArc;

	// Enable decryption request

	SetDecryptRequirement( TRUE );

	// Set decryption size

	SetDecryptSize( 0 );

	// Call the initialization function that has been overwritten

	m_dwDecryptKey = OnInitDecrypt( pclArc );
}

//////////////////////////////////////////////////////////////////////////////////////////
//  By default, use simple decoding

DWORD	CKrkr::OnInitDecrypt(
	CArcFile*			pclArc							// Archive
	)
{
	DWORD				dwDecryptKey = pclArc->InitDecrypt();

	if( dwDecryptKey == 0 )
	{
		// Unencrypted

		SetDecryptRequirement( FALSE );
	}

	return	dwDecryptKey;
}

//////////////////////////////////////////////////////////////////////////////////////////
//  Decryption Process

DWORD	CKrkr::Decrypt(
	BYTE*				pbtTarget,						// Data to be decoded
	DWORD				dwTargetSize,					// Decoding Size
	DWORD				dwOffset						// Location of data to be decoded (offset)
	)
{
	// Don't decode files

	if( !m_bDecrypt )
	{
		// No decryption requests

		return	dwTargetSize;
	}

	DWORD				dwDecryptSize = m_dwDecryptSize;

	if( dwDecryptSize == 0 )
	{
		// Decoding size has not been set

		return	OnDecrypt( pbtTarget, dwTargetSize, dwOffset, m_dwDecryptKey );
	}
	else
	{
		// Decoding size has been set

		if( dwOffset >= dwDecryptSize )
		{
			// Don't decode anymore

			return	dwTargetSize;
		}

		if( dwDecryptSize > dwTargetSize )
		{
			// Size is larger than the predetermined decryption data size

			dwDecryptSize = dwTargetSize;
		}

		OnDecrypt( pbtTarget, dwDecryptSize, dwOffset, m_dwDecryptKey );

		return	dwTargetSize;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////
//  By default, use simple decoding
//
//  Remark: The dwDecryptKey returns the value from OnInitDecrypt

DWORD	CKrkr::OnDecrypt(
	BYTE*				pbtTarget,						// Data to be decoded
	DWORD				dwTargetSize,					// Decoding size
	DWORD				dwOffset,						// Location of data to be decoded (offset)
	DWORD				dwDecryptKey					// Decryption key
	)
{
	m_pclArc->Decrypt( pbtTarget, dwTargetSize );

	return	dwTargetSize;
}

//////////////////////////////////////////////////////////////////////////////////////////
//  Set decryption request

void	CKrkr::SetDecryptRequirement(
	BOOL				bDecrypt						// Decryption request
	)
{
	m_bDecrypt = bDecrypt;
}

//////////////////////////////////////////////////////////////////////////////////////////
//  Set decoding size

void	CKrkr::SetDecryptSize(
	DWORD				dwDecryptSize					// Decoding size
	)
{
	m_dwDecryptSize = dwDecryptSize;
}

//////////////////////////////////////////////////////////////////////////////////////////
//  Get the offset for the location of the archive within an EXE file.
//
//  Remark: KiriKiri allows its resources to be stored within an executable

BOOL	CKrkr::FindXP3FromExecuteFile(
	CArcFile*			pclArc,							// Archive
	DWORD*				pdwOffset						// Destination
	)
{
	if( pclArc->GetArcSize() <= 0x200000 )
	{
		// Is not a kirikiri executable

		return	FALSE;
	}

	*pdwOffset = 16;

	pclArc->SeekHed( 16 );

	BYTE				abtBuffer[4096];
	DWORD				dwReadSize;

	do
	{
		dwReadSize = pclArc->Read( abtBuffer, sizeof(abtBuffer) );

		for( DWORD i = 0, j = 0 ; i < (dwReadSize / 16) ; i++, j += 16 )
		{
			if( memcmp( &abtBuffer[j], "XP3\r\n \n\x1A\x8B\x67\x01", 11) == 0 )
			{
				// Found XP3 archive

				*pdwOffset += j;
				return	TRUE;
			}
		}

		*pdwOffset += dwReadSize;

		if( *pdwOffset >= 0x500000 )
		{
			// Truncate search
			
			break;
		}

		// If canceled

		if( pclArc->GetProg()->OnCancel() )
		{
			throw	-1;
		}
	}
	while( dwReadSize == sizeof(abtBuffer) );

	// No XP3 archive

	*pdwOffset = 0;

	pclArc->SeekHed();

	return	FALSE;
}
