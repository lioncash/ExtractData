#include "StdAfx.h"
#include "YCString.h"

/// Gets the length of the string after conversion.
///
/// @param src Multi-byte string
///
/// @remark Multi-byte character -> Wide character
///
/// @return Length of the wide-character string equivalent of the given multi-byte string.
///
int YCBaseString::GetBaseTypeLength(const char* src) const
{
	return GetBaseTypeLength(src, -1) - 1;
}

/// Gets the length of the string after conversion.
///
/// @param src    Multi-byte string
/// @param length Length
///
/// @remark Multi-byte character -> Wide character
///
/// @return The length of the resulting wide-character string.
///
int YCBaseString::GetBaseTypeLength(const char* src, int length) const
{
	return ::MultiByteToWideChar(CP_ACP, 0, src, length, nullptr, 0);
}

/// Gets the length of the string after conversion.
///
/// @param src Wide-character string
///
/// @remark Wide character -> Multi-byte character
///
/// @return Length of the multi-byte string equivalent of the given wide-character string.
///
int YCBaseString::GetBaseTypeLength(const wchar_t* src) const
{
	return GetBaseTypeLength(src, -1) - 1;
}

/// Gets the length of the string after conversion.
///
/// @param src    Wide-character string
/// @param length Length
///
/// @remark Wide character -> Multi-byte character
///
/// @return The length of the multi-byte character.
///
int YCBaseString::GetBaseTypeLength(const wchar_t* src, int length) const
{
	return ::WideCharToMultiByte(CP_ACP, 0, src, length, nullptr, 0, nullptr, nullptr);
}

/// Character code conversion
///
/// @param dst        Wide-character string destination
/// @param dst_length Destination size
/// @param src        Multi-byte string
/// @param src_length Multi-byte string length
///
/// @remark Multi-byte string -> Wide-character string
///
/// @return The length of the wide-character string.
///
int YCBaseString::ConvertToBaseType(wchar_t* dst, int dst_length, const char* src, int src_length) const
{
	int length = ::MultiByteToWideChar(CP_ACP, 0, src, src_length, dst, dst_length);

	if (src_length == -1)
	{
		length -= 1;
	}
	else
	{
		dst[length] = L'\0';
	}

	return length;
}

/// Character code conversion
///
/// @param dst        Multi-byte string destination
/// @param dst_length Destination size
/// @param src        Wide-character string
/// @param src_length Wide-character string length
///
/// @remark Wide-character string -> Multi-byte string
///
/// @return The length of the multi-byte string.
///
int YCBaseString::ConvertToBaseType(char* dst, int dst_length, const wchar_t* src, int src_length) const
{
	int length = ::WideCharToMultiByte(CP_ACP, 0, src, src_length, dst, dst_length, nullptr, nullptr);

	if (src_length == -1)
	{
		length -= 1;
	}
	else
	{
		dst[length] = '\0';
	}

	return length;
}
