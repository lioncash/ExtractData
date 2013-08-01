#include "stdafx.h"
#include "File.h"
#include "MD5.h"

//////////////////////////////////////////////////////////////////////////////////////////
//	Constructor

CMD5::CMD5()
{
	m_anTable[ 1] = 0xD76AA478;
	m_anTable[ 2] = 0xE8C7B756;
	m_anTable[ 3] = 0x242070DB;
	m_anTable[ 4] = 0xC1BDCEEE;
	m_anTable[ 5] = 0xF57C0FAF;
	m_anTable[ 6] = 0x4787C62A;
	m_anTable[ 7] = 0xA8304613;
	m_anTable[ 8] = 0xFD469501;
	m_anTable[ 9] = 0x698098D8;
	m_anTable[10] = 0x8B44F7AF;
	m_anTable[11] = 0xFFFF5BB1;
	m_anTable[12] = 0x895CD7BE;
	m_anTable[13] = 0x6B901122;
	m_anTable[14] = 0xFD987193;
	m_anTable[15] = 0xA679438E;
	m_anTable[16] = 0x49B40821;
	m_anTable[17] = 0xF61E2562;
	m_anTable[18] = 0xC040B340;
	m_anTable[19] = 0x265E5A51;
	m_anTable[20] = 0xE9B6C7AA;
	m_anTable[21] = 0xD62F105D;
	m_anTable[22] = 0x02441453;
	m_anTable[23] = 0xD8A1E681;
	m_anTable[24] = 0xE7D3FBC8;
	m_anTable[25] = 0x21E1CDE6;
	m_anTable[26] = 0xC33707D6;
	m_anTable[27] = 0xF4D50D87;
	m_anTable[28] = 0x455A14ED;
	m_anTable[29] = 0xA9E3E905;
	m_anTable[30] = 0xFCEFA3F8;
	m_anTable[31] = 0x676F02D9;
	m_anTable[32] = 0x8D2A4C8A;
	m_anTable[33] = 0xFFFA3942;
	m_anTable[34] = 0x8771F681;
	m_anTable[35] = 0x6D9D6122;
	m_anTable[36] = 0xFDE5380C;
	m_anTable[37] = 0xA4BEEA44;
	m_anTable[38] = 0x4BDECFA9;
	m_anTable[39] = 0xF6BB4B60;
	m_anTable[40] = 0xBEBFBC70;
	m_anTable[41] = 0x289B7EC6;
	m_anTable[42] = 0xEAA127FA;
	m_anTable[43] = 0xD4EF3085;
	m_anTable[44] = 0x04881D05;
	m_anTable[45] = 0xD9D4D039;
	m_anTable[46] = 0xE6DB99E5;
	m_anTable[47] = 0x1FA27CF8;
	m_anTable[48] = 0xC4AC5665;
	m_anTable[49] = 0xF4292244;
	m_anTable[50] = 0x432AFF97;
	m_anTable[51] = 0xAB9423A7;
	m_anTable[52] = 0xFC93A039;
	m_anTable[53] = 0x655B59C3;
	m_anTable[54] = 0x8F0CCC92;
	m_anTable[55] = 0xFFEFF47D;
	m_anTable[56] = 0x85845DD1;
	m_anTable[57] = 0x6FA87E4F;
	m_anTable[58] = 0xFE2CE6E0;
	m_anTable[59] = 0xA3014314;
	m_anTable[60] = 0x4E0811A1;
	m_anTable[61] = 0xF7537E82;
	m_anTable[62] = 0xBD3AF235;
	m_anTable[63] = 0x2AD7D2BB;
	m_anTable[64] = 0xEB86D391;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	

SMD5 CMD5::Calculate(
	LPCTSTR				pszPathToFile					// File path
	)
{
	// Open file

	CFile				clfWork;

	if( !clfWork.OpenForRead( pszPathToFile ) )
	{
		return	m_stmd5Value;
	}

	// Get file size

	DWORD				dwSrcSize = clfWork.GetFileSize();

	// Get padding

	DWORD				dwPadding = CalculatePadding( dwSrcSize );

	// Read buffer allocation

	YCMemory<BYTE>		clmbtSrc( dwSrcSize + dwPadding );

	// Read file

	clfWork.Read( &clmbtSrc[0], dwSrcSize );

	// Append padding

	AppendPadding( &clmbtSrc[0], dwSrcSize, dwPadding );

	// Calculate MD5

	return	Calculate( &clmbtSrc[0], (dwSrcSize + dwPadding) );
}

//////////////////////////////////////////////////////////////////////////////////////////
//	Calculate MD5 Value
//
// Remark: If bAlignment assumes that the input data is aligned on 64 bytes
//			then the efficiency is not good

SMD5	CMD5::Calculate(
	const void*			pvSrc,							// Input data
	DWORD				dwSrcSize,						// Input data size
	const DWORD*		pdwInitialize,					// Initialization
	BOOL				bAlignment						// Alignment request
	)
{
	const DWORD*		pdwSrc = (const DWORD*) pvSrc;

	// Alignment

	YCMemory<BYTE>		clmbtSrc;

	if( bAlignment )
	{
		// Alignment request

		DWORD				dwPadding = CalculatePadding( dwSrcSize );

		clmbtSrc.resize( dwSrcSize + dwPadding );

		memcpy( &clmbtSrc[0], pvSrc, dwSrcSize );

		// Append padding

		AppendPadding( &clmbtSrc[0], dwSrcSize, dwPadding );

		pdwSrc = (const DWORD*) &clmbtSrc[0];
	}

	// Set initial value

	if( pdwInitialize == NULL )
	{
		// Using the default initialization values

		m_stmd5Value.adwABCD[0] = 0x67452301;
		m_stmd5Value.adwABCD[1] = 0xEFCDAB89;
		m_stmd5Value.adwABCD[2] = 0x98BADCFE;
		m_stmd5Value.adwABCD[3] = 0x10325476;
	}
	else
	{
		m_stmd5Value.adwABCD[0] = pdwInitialize[0];
		m_stmd5Value.adwABCD[1] = pdwInitialize[1];
		m_stmd5Value.adwABCD[2] = pdwInitialize[2];
		m_stmd5Value.adwABCD[3] = pdwInitialize[3];
	}

	for( DWORD i = 0, dwSrcPtr = 0 ; i < dwSrcSize ; i += 64, dwSrcPtr += 16 )
	{
		for( int j = 0 ; j < 16 ; j++ )
		{
			// Copy

			m_adwX[j] = pdwSrc[dwSrcPtr + j];
		}

		// Save

		DWORD				dwA = m_stmd5Value.adwABCD[0];
		DWORD				dwB = m_stmd5Value.adwABCD[1];
		DWORD				dwC = m_stmd5Value.adwABCD[2];
		DWORD				dwD = m_stmd5Value.adwABCD[3];

		CalculateSub5( dwA, dwB, dwC, dwD,  0,  7,  1 );
		CalculateSub5( dwD, dwA, dwB, dwC,  1, 12,  2 );
		CalculateSub5( dwC, dwD, dwA, dwB,  2, 17,  3 );
		CalculateSub5( dwB, dwC, dwD, dwA,  3, 22,  4 );
		CalculateSub5( dwA, dwB, dwC, dwD,  4,  7,  5 );
		CalculateSub5( dwD, dwA, dwB, dwC,  5, 12,  6 );
		CalculateSub5( dwC, dwD, dwA, dwB,  6, 17,  7 );
		CalculateSub5( dwB, dwC, dwD, dwA,  7, 22,  8 );
		CalculateSub5( dwA, dwB, dwC, dwD,  8,  7,  9 );
		CalculateSub5( dwD, dwA, dwB, dwC,  9, 12, 10 );
		CalculateSub5( dwC, dwD, dwA, dwB, 10, 17, 11 );
		CalculateSub5( dwB, dwC, dwD, dwA, 11, 22, 12 );
		CalculateSub5( dwA, dwB, dwC, dwD, 12,  7, 13 );
		CalculateSub5( dwD, dwA, dwB, dwC, 13, 12, 14 );
		CalculateSub5( dwC, dwD, dwA, dwB, 14, 17, 15 );
		CalculateSub5( dwB, dwC, dwD, dwA, 15, 22, 16 );

		CalculateSub6( dwA, dwB, dwC, dwD,  1,  5, 17 );
		CalculateSub6( dwD, dwA, dwB, dwC,  6,  9, 18 );
		CalculateSub6( dwC, dwD, dwA, dwB, 11, 14, 19 );
		CalculateSub6( dwB, dwC, dwD, dwA,  0, 20, 20 );
		CalculateSub6( dwA, dwB, dwC, dwD,  5,  5, 21 );
		CalculateSub6( dwD, dwA, dwB, dwC, 10,  9, 22 );
		CalculateSub6( dwC, dwD, dwA, dwB, 15, 14, 23 );
		CalculateSub6( dwB, dwC, dwD, dwA,  4, 20, 24 );
		CalculateSub6( dwA, dwB, dwC, dwD,  9,  5, 25 );
		CalculateSub6( dwD, dwA, dwB, dwC, 14,  9, 26 );
		CalculateSub6( dwC, dwD, dwA, dwB,  3, 14, 27 );
		CalculateSub6( dwB, dwC, dwD, dwA,  8, 20, 28 );
		CalculateSub6( dwA, dwB, dwC, dwD, 13,  5, 29 );
		CalculateSub6( dwD, dwA, dwB, dwC,  2,  9, 30 );
		CalculateSub6( dwC, dwD, dwA, dwB,  7, 14, 31 );
		CalculateSub6( dwB, dwC, dwD, dwA, 12, 20, 32 );

		CalculateSub7( dwA, dwB, dwC, dwD,  5,  4, 33 );
		CalculateSub7( dwD, dwA, dwB, dwC,  8, 11, 34 );
		CalculateSub7( dwC, dwD, dwA, dwB, 11, 16, 35 );
		CalculateSub7( dwB, dwC, dwD, dwA, 14, 23, 36 );
		CalculateSub7( dwA, dwB, dwC, dwD,  1,  4, 37 );
		CalculateSub7( dwD, dwA, dwB, dwC,  4, 11, 38 );
		CalculateSub7( dwC, dwD, dwA, dwB,  7, 16, 39 );
		CalculateSub7( dwB, dwC, dwD, dwA, 10, 23, 40 );
		CalculateSub7( dwA, dwB, dwC, dwD, 13,  4, 41 );
		CalculateSub7( dwD, dwA, dwB, dwC,  0, 11, 42 );
		CalculateSub7( dwC, dwD, dwA, dwB,  3, 16, 43 );
		CalculateSub7( dwB, dwC, dwD, dwA,  6, 23, 44 );
		CalculateSub7( dwA, dwB, dwC, dwD,  9,  4, 45 );
		CalculateSub7( dwD, dwA, dwB, dwC, 12, 11, 46 );
		CalculateSub7( dwC, dwD, dwA, dwB, 15, 16, 47 );
		CalculateSub7( dwB, dwC, dwD, dwA,  2, 23, 48 );

		CalculateSub8( dwA, dwB, dwC, dwD,  0,  6, 49 );
		CalculateSub8( dwD, dwA, dwB, dwC,  7, 10, 50 );
		CalculateSub8( dwC, dwD, dwA, dwB, 14, 15, 51 );
		CalculateSub8( dwB, dwC, dwD, dwA,  5, 21, 52 );
		CalculateSub8( dwA, dwB, dwC, dwD, 12,  6, 53 );
		CalculateSub8( dwD, dwA, dwB, dwC,  3, 10, 54 );
		CalculateSub8( dwC, dwD, dwA, dwB, 10, 15, 55 );
		CalculateSub8( dwB, dwC, dwD, dwA,  1, 21, 56 );
		CalculateSub8( dwA, dwB, dwC, dwD,  8,  6, 57 );
		CalculateSub8( dwD, dwA, dwB, dwC, 15, 10, 58 );
		CalculateSub8( dwC, dwD, dwA, dwB,  6, 15, 59 );
		CalculateSub8( dwB, dwC, dwD, dwA, 13, 21, 60 );
		CalculateSub8( dwA, dwB, dwC, dwD,  4,  6, 61 );
		CalculateSub8( dwD, dwA, dwB, dwC, 11, 10, 62 );
		CalculateSub8( dwC, dwD, dwA, dwB,  2, 15, 63 );
		CalculateSub8( dwB, dwC, dwD, dwA,  9, 21, 64 );

		m_stmd5Value.adwABCD[0] += dwA;
		m_stmd5Value.adwABCD[1] += dwB;
		m_stmd5Value.adwABCD[2] += dwC;
		m_stmd5Value.adwABCD[3] += dwD;
	}

	// Convert to a string

	MD5ToStrings( m_stmd5Value.szABCD, m_stmd5Value.adwABCD );

//	ValueToStr( &m_stmd5Value.szABCD[ 0], m_stmd5Value.adwABCD[0] );
//	ValueToStr( &m_stmd5Value.szABCD[ 8], m_stmd5Value.adwABCD[1] );
//	ValueToStr( &m_stmd5Value.szABCD[16], m_stmd5Value.adwABCD[2] );
//	ValueToStr( &m_stmd5Value.szABCD[24], m_stmd5Value.adwABCD[3] );

	return	m_stmd5Value;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	Calculation Processing

DWORD	CMD5::CalculateSub1(
	DWORD				dwX,
	DWORD				dwY,
	DWORD				dwZ
	)
{
	return	((dwX & dwY) | (~dwX & dwZ));
}

//////////////////////////////////////////////////////////////////////////////////////////
//	Calculation Processing

DWORD	CMD5::CalculateSub2(
	DWORD				dwX,
	DWORD				dwY,
	DWORD				dwZ
	)
{
	return	((dwX & dwZ) | (dwY & ~dwZ));
}

//////////////////////////////////////////////////////////////////////////////////////////
//	Calculation Processing

DWORD	CMD5::CalculateSub3(
	DWORD				dwX,
	DWORD				dwY,
	DWORD				dwZ
	)
{
	return	(dwX ^ dwY ^ dwZ);
}

//////////////////////////////////////////////////////////////////////////////////////////
//	Calculation Processing

DWORD	CMD5::CalculateSub4(
	DWORD				dwX,
	DWORD				dwY,
	DWORD				dwZ
	)
{
	return	(dwY ^ (dwX | ~dwZ));
}

//////////////////////////////////////////////////////////////////////////////////////////
//	Calculation Processing

void	CMD5::CalculateSub5(
	DWORD&				dwA,
	DWORD				dwB,
	DWORD				dwC,
	DWORD				dwD,
	DWORD				dwK,
	DWORD				dwS,
	DWORD				dwI
	)
{
	dwA = dwB + RotateLeft( dwA + CalculateSub1( dwB, dwC, dwD ) + m_adwX[dwK] + m_anTable[dwI], dwS );
}

//////////////////////////////////////////////////////////////////////////////////////////
//	Calculation Processing

void	CMD5::CalculateSub6(
	DWORD&				dwA,
	DWORD				dwB,
	DWORD				dwC,
	DWORD				dwD,
	DWORD				dwK,
	DWORD				dwS,
	DWORD				dwI
	)
{
	dwA = dwB + RotateLeft( dwA + CalculateSub2( dwB, dwC, dwD ) + m_adwX[dwK] + m_anTable[dwI], dwS );
}

//////////////////////////////////////////////////////////////////////////////////////////
//	Calculation Processing

void	CMD5::CalculateSub7(
	DWORD&				dwA,
	DWORD				dwB,
	DWORD				dwC,
	DWORD				dwD,
	DWORD				dwK,
	DWORD				dwS,
	DWORD				dwI
	)
{
	dwA = dwB + RotateLeft( dwA + CalculateSub3( dwB, dwC, dwD ) + m_adwX[dwK] + m_anTable[dwI], dwS );
}

//////////////////////////////////////////////////////////////////////////////////////////
//	Calculation Processing

void	CMD5::CalculateSub8(
	DWORD&				dwA,
	DWORD				dwB,
	DWORD				dwC,
	DWORD				dwD,
	DWORD				dwK,
	DWORD				dwS,
	DWORD				dwI
	)
{
	dwA = dwB + RotateLeft( dwA + CalculateSub4( dwB, dwC, dwD ) + m_adwX[dwK] + m_anTable[dwI], dwS );
}

//////////////////////////////////////////////////////////////////////////////////////////
//	Calculate Padding

DWORD	CMD5::CalculatePadding(
	DWORD				dwSize							// Size
	)
{
	// Get padding

	DWORD				dwWork = (dwSize % 64);
	DWORD				dwPadding = (64 - dwWork);

	if( dwPadding < 9 )
	{
		// 1バイト以上のパディングと8バイトのデータサイズが入らない

		dwPadding += 64;
	}

	return	dwPadding;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	Append Padding

void	CMD5::AppendPadding(
	void*				pvSrc,							// Input data
	DWORD				dwSrcSize,						// Input data size
	DWORD				dwPadding						// Padding
	)
{
	BYTE*				pbtSrc = (BYTE*) pvSrc;

	// Append padding

	pbtSrc[dwSrcSize] = 0x80;

	ZeroMemory( &pbtSrc[dwSrcSize + 1], (dwPadding - 9) );

	// Additional data size (in bits)

	*(UINT64*) &pbtSrc[dwSrcSize + dwPadding - 8] = (UINT64) dwSrcSize * 8;
}

//////////////////////////////////////////////////////////////////////////////////////////
//	Circular Rotate Left

DWORD	CMD5::RotateLeft(
	DWORD				dwA,
	DWORD				dwS
	)
{
	return	((dwA << dwS) | (dwA >> (32 - dwS)));
}

//////////////////////////////////////////////////////////////////////////////////////////
//	MD5 value converted to a string of decimal digits

void	CMD5::ValueToStr(
	LPSTR				pszDstOfMD5,					// Storage location of the string
	DWORD				dwMD5							// MD5 value
	)
{
	static const char	acHex[] =
	{
		'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'
	};

	for( int i = 0 ; i < 4 ; i++ )
	{
		pszDstOfMD5[i * 2 + 0] = acHex[((dwMD5 >> 4) & 0x0F)];
		pszDstOfMD5[i * 2 + 1] = acHex[(dwMD5 & 0x0F)];

		dwMD5 >>= 8;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////
//	MD5 value converted to a string of decimal digits

void	CMD5::MD5ToStrings(
	LPSTR				pszDstOfMD5,					// Storage location of the string
	const DWORD*		pdwMD5							// MD5 value
	)
{
	static const char	acHex[] =
	{
		'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'
	};

	for( int i = 0 ; i < 4 ; i++ )
	{
		DWORD				dwMD5 = pdwMD5[i];

		for( int j = 0 ; j < 4 ; j++ )
		{
			pszDstOfMD5[i * 8 + j * 2 + 0] = acHex[((dwMD5 >> 4) & 0x0F)];
			pszDstOfMD5[i * 8 + j * 2 + 1] = acHex[(dwMD5 & 0x0F)];

			dwMD5 >>= 8;
		}
	}
}
