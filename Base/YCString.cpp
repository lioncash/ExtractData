
#include	"stdafx.h"
#include	"YCString.h"

//////////////////////////////////////////////////////////////////////////////////////////
// Gets the length of the string after conversion.
//
// Returns: Length of the wide-character string equivalent of the given multi-byte string.
//
// Remarks: Multi-byte character -> Wide character

int		YCBaseString::GetBaseTypeLength(
	const char*			pszSrc					// Multi-byte string
	) const
{
	return (GetBaseTypeLength( pszSrc, -1 ) - 1);
}

//////////////////////////////////////////////////////////////////////////////////////////
// Gets the length of the string after conversion.
//
// Returns: The length of the resulting wide-character string.
//
// Remarks: Multi-byte character -> Wide character

int		YCBaseString::GetBaseTypeLength(
	const char*			pszSrc,					// Multi-byte string
	int					nLength					// Length
	) const
{
	return ::MultiByteToWideChar( CP_ACP, 0, pszSrc, nLength, NULL, 0 );
}

//////////////////////////////////////////////////////////////////////////////////////////
// Gets the length of the string after conversion.
//
// Returns: Length of the multi-byte string equivalent of the given wide-character string.
//
// Remarks: Wide character -> Multi-byte character

int		YCBaseString::GetBaseTypeLength(
	const wchar_t*		pwszSrc					// Wide-character string
	) const
{
	return (GetBaseTypeLength( pwszSrc, -1 ) - 1);
}

//////////////////////////////////////////////////////////////////////////////////////////
// Gets the length of the string after conversion.
//
// Returns: The length of the multi-byte character.
//
// Remarks: Wide character -> Multi-byte character

int		YCBaseString::GetBaseTypeLength(
	const wchar_t*		pwszSrc,				// Wide-character string
	int					nLength					// Length
	) const
{
	return ::WideCharToMultiByte( CP_ACP, 0, pwszSrc, nLength, NULL, 0, NULL, NULL );
}

//////////////////////////////////////////////////////////////////////////////////////////
// Character code conversion
//
// Return: The length of the wide-character string.
//
// Remarks: Multi-byte string -> Wide-character string

int		YCBaseString::ConvertToBaseType(
	wchar_t*			pwszDst,				// Wide-character string destination
	int					nDstLength,				// Destination size
	const char*			pszSrc,					// Multi-byte string
	int					nSrcLength				// Multi-byte string length
	) const
{
	int					nLength;

	nLength = ::MultiByteToWideChar( CP_ACP, 0, pszSrc, nSrcLength, pwszDst, nDstLength );

	if( nSrcLength == -1 )
	{
		nLength -= 1;
	}
	else
	{
		pwszDst[nLength] = L'\0';
	}

	return nLength;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Character code conversion
//
// Returns: The length of the multi-byte string.
//
// Remarks: Wide-character string -> Multi-byte string

int		YCBaseString::ConvertToBaseType(
	char*				pszDst,					// Multi-byte string destination.
	int					nDstLength,				// Destination size
	const wchar_t*		pwszSrc,				// Wide-character string
	int					nSrcLength				// Wide-character string length
	) const
{
	int					nLength;

	nLength = ::WideCharToMultiByte( CP_ACP, 0, pwszSrc, nSrcLength, pszDst, nDstLength, NULL, NULL );

	if( nSrcLength == -1 )
	{
		nLength -= 1;
	}
	else
	{
		pszDst[nLength] = '\0';
	}

	return nLength;
}
