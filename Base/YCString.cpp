#include "StdAfx.h"
#include "YCString.h"

/// Gets the length of the string after conversion.
///
/// @param pszSrc Multi-byte string
///
/// @remark Multi-byte character -> Wide character
///
/// @return Length of the wide-character string equivalent of the given multi-byte string.
///
int YCBaseString::GetBaseTypeLength(const char* pszSrc) const
{
	return (GetBaseTypeLength(pszSrc, -1) - 1);
}

/// Gets the length of the string after conversion.
///
/// @param pszSrc  Multi-byte string
/// @param nLength Length
///
/// @remark Multi-byte character -> Wide character
///
/// @return The length of the resulting wide-character string.
///
int YCBaseString::GetBaseTypeLength(const char* pszSrc, int nLength) const
{
	return ::MultiByteToWideChar(CP_ACP, 0, pszSrc, nLength, nullptr, 0);
}

/// Gets the length of the string after conversion.
///
/// @param pwszSrc Wide-character string
///
/// @remark Wide character -> Multi-byte character
///
/// @return Length of the multi-byte string equivalent of the given wide-character string.
///
int YCBaseString::GetBaseTypeLength(const wchar_t* pwszSrc) const
{
	return (GetBaseTypeLength( pwszSrc, -1 ) - 1);
}

/// Gets the length of the string after conversion.
///
/// @param pwszSrc Wide-character string
/// @param nLength Length
///
/// @remark Wide character -> Multi-byte character
///
/// @return The length of the multi-byte character.
///
int YCBaseString::GetBaseTypeLength(const wchar_t* pwszSrc, int nLength) const
{
	return ::WideCharToMultiByte(CP_ACP, 0, pwszSrc, nLength, nullptr, 0, nullptr, nullptr);
}

/// Character code conversion
///
/// @param pwszDst    Wide-character string destination
/// @param nDstLength Destination size
/// @param pszSrc     Multi-byte string
/// @param nSrcLength Multi-byte string length
///
/// @remark Multi-byte string -> Wide-character string
///
/// @return The length of the wide-character string.
///
int YCBaseString::ConvertToBaseType(wchar_t* pwszDst, int nDstLength, const char* pszSrc, int nSrcLength) const
{
	int nLength = ::MultiByteToWideChar(CP_ACP, 0, pszSrc, nSrcLength, pwszDst, nDstLength);

	if (nSrcLength == -1)
	{
		nLength -= 1;
	}
	else
	{
		pwszDst[nLength] = L'\0';
	}

	return nLength;
}

/// Character code conversion
///
/// @param pszDst     Multi-byte string destination
/// @param nDstLength Destination size
/// @param pwszSrc    Wide-character string
/// @param nSrcLength Wide-character string length
///
/// @remark Wide-character string -> Multi-byte string
///
/// @return The length of the multi-byte string.
///
int YCBaseString::ConvertToBaseType(char* pszDst, int nDstLength, const wchar_t* pwszSrc, int nSrcLength) const
{
	int nLength = ::WideCharToMultiByte(CP_ACP, 0, pwszSrc, nSrcLength, pszDst, nDstLength, nullptr, nullptr);

	if (nSrcLength == -1)
	{
		nLength -= 1;
	}
	else
	{
		pszDst[nLength] = '\0';
	}

	return nLength;
}
