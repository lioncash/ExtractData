#pragma once

//----------------------------------------------------------------------------------------
//-- Utility Functions -------------------------------------------------------------------
//----------------------------------------------------------------------------------------

namespace YCUtil
{
	// String functions

//	YCString					SetFileFormat( const YCString& rfclsFileName );
//	YCString					SetCommaFormat( DWORD dwSize, DWORD dwDigit = 3 );

	// Path functions

	void						CreateDirectory( LPCTSTR pszPathToFile, BOOL bFileName );
	void						ReplaceSlashToBackslash( LPSTR pszFileName );
	void						ReplaceSlashToBackslash( LPWSTR pwszFileName );

	// Endian Conversion

	DWORD						ConvEndian( DWORD dwSrc );
	void						ConvEndian( LPDWORD pdwDst );

	WORD						ConvEndian( WORD wSrc );
	void						ConvEndian( LPWORD pwDst );

	// Memory Functions

	inline BOOL					CompareMemory( const void* pvData1, const void* pvData2, DWORD dwSize );
};
