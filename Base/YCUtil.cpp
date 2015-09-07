#include "stdafx.h"
#include "YCUtil.h"
/*
//////////////////////////////////////////////////////////////////////////////////////////
// Set File Format

YCString YCUtil::SetFileFormat(
	const YCString&		rfcstFileName					// Filename
	)
{
	// Retrieves the extension

	LPCTSTR pszFileExt = PathFindExtension( rfcstFileName );

	if( pszFileExt == NULL )
	{
		// Extension does not exist

		return _T("");
	}

	if( lstrlen( pszFileExt ) < 2 )
	{
		// Extension is not only "."

		return _T("");
	}

	// Converted to uppercase extension

	TCHAR szFileFormat[256];

	lstrcpy( szFileFormat, &pszFileExt[1] );
	CharUpper( szFileFormat );

	return szFileFormat;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Separates digits by a comma 

YCString YCUtil::SetCommaFormat(
	DWORD				dwSize,							// Number
	DWORD				dwDigit							// Separate digit
	)
{
	YCString cstSize;

	cstSize.Format( _T("%d"), dwSize );

	int nLength = cstSize.length();

	int comma_num = (nLength - 1) / dwDigit;
	int comma_pos = nLength % dwDigit;

	if( comma_pos == 0 )
	{
		comma_pos = dwDigit;
	}
	if( comma_num == 0 )
	{
		comma_pos = 0;
	}

	for( int i = 0 ; i < comma_num ; i++ )
	{
		cstSize.insert( comma_pos + dwDigit * i + i, _T(",") );
	}

	return cstSize;
}
*/
//////////////////////////////////////////////////////////////////////////////////////////
// Creates the lowest level directory you want to create
//
// Parameters:
//   - pszPathToFile - File path
//   - bFileName     - Contains a file name or file path

void YCUtil::CreateDirectory(LPCTSTR pszPathToFile, BOOL bFileName)
{
	std::vector<YCString> vcDirPathList;
	LPCTSTR               pszFilePathBase = pszPathToFile;

	while ((pszPathToFile = PathFindNextComponent(pszPathToFile)) != NULL)
	{
		YCString clsDirPath(pszFilePathBase, pszPathToFile - pszFilePathBase - 1); // Do not put a '\' at the end just to be sure to -1

		vcDirPathList.push_back(clsDirPath);
	}

	// Create a directory in the order from the root

	size_t uMax = vcDirPathList.size();

	if (bFileName)
	{
		// To -1 so as not to create a directory of the file name

		uMax--;
	}

	for (size_t uCnt = 0; uCnt < uMax; uCnt++)
	{
		::CreateDirectory(vcDirPathList[uCnt], NULL);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////
// Replace forward slashes with backward slashes

void YCUtil::ReplaceSlashToBackslash(LPSTR pszFileName)
{
	while (*pszFileName != '\0')
	{
		if (!::IsDBCSLeadByte(*pszFileName))
		{
			// 1 byte character

			// Replace forward slashes with backward slashes

			if (*pszFileName == '/')
			{
				*pszFileName = '\\';
			}
		}

		// Advance to next character

		pszFileName = ::CharNextA(pszFileName);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////
// Replace forward slashes with backward slashes

void YCUtil::ReplaceSlashToBackslash(LPWSTR pwszFileName)
{
	for (; *pwszFileName != L'\0'; pwszFileName++)
	{
		if (*pwszFileName == L'/')
		{
			// Slash

			*pwszFileName = L'\\';
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////
// Convert Endian (4byte)

DWORD YCUtil::ConvEndian(DWORD dwSrc)
{
#if defined(_MSC_VER)
	return _byteswap_ulong(dwSrc);
#else
	return ((dwSrc & 0x000000FF) << 24) | (dwSrc & 0x0000FF00) << 8) | ((dwSrc & 0x00FF0000) >> 8) | ((dwSrc & 0xFF000000) >> 24);
#endif
}

//////////////////////////////////////////////////////////////////////////////////////////
// Convert Endian (4byte)

void YCUtil::ConvEndian(LPDWORD pdwDst)
{
	*pdwDst = ConvEndian(*pdwDst);
}

//////////////////////////////////////////////////////////////////////////////////////////
// Convert Endian (2byte)

WORD YCUtil::ConvEndian(WORD wSrc)
{
#if defined(_MSC_VER)
	return _byteswap_ushort(wSrc);
#else
	return (value << 8) | (value >> 8);
#endif
}

//////////////////////////////////////////////////////////////////////////////////////////
// Convert Endian (2byte)

void YCUtil::ConvEndian(LPWORD pwDst)
{
	*pwDst = ConvEndian(*pwDst);
}

//////////////////////////////////////////////////////////////////////////////////////////
// Compares the contents of memory
//
// Remark: Wildcard compatible version of the standard memcmp() function
//
// Parameters:
//   - pvData1 - Comparison data 1
//   - pvData2 - Comparison data 2
//   - dwSize  - Number of elements to compare

BOOL YCUtil::CompareMemory(const void* pvData1, const void* pvData2, DWORD dwSize)
{
	BYTE* pbtData1 = (BYTE*)pvData1;
	BYTE* pbtData2 = (BYTE*)pvData2;

	for (DWORD i = 0; i < dwSize; i++)
	{
		if ((pbtData1[i] != pbtData2[i]) && (pbtData2[i] != '*'))
		{
			// Mismatch
			return FALSE;
		}
	}

	// Match
	return TRUE;
}
