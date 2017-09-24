#include "StdAfx.h"
#include "File.h"
#include "MD5.h"

#include <array>
#include <vector>

namespace
{
constexpr std::array<int, 65> md5_table{{
	0x00000000, 0xD76AA478, 0xE8C7B756, 0x242070DB,
	0xC1BDCEEE, 0xF57C0FAF, 0x4787C62A, 0xA8304613,
	0xFD469501, 0x698098D8, 0x8B44F7AF, 0xFFFF5BB1,
	0x895CD7BE, 0x6B901122, 0xFD987193, 0xA679438E,
	0x49B40821, 0xF61E2562, 0xC040B340, 0x265E5A51,
	0xE9B6C7AA, 0xD62F105D, 0x02441453, 0xD8A1E681,
	0xE7D3FBC8, 0x21E1CDE6, 0xC33707D6, 0xF4D50D87,
	0x455A14ED, 0xA9E3E905, 0xFCEFA3F8, 0x676F02D9,
	0x8D2A4C8A, 0xFFFA3942, 0x8771F681, 0x6D9D6122,
	0xFDE5380C, 0xA4BEEA44, 0x4BDECFA9, 0xF6BB4B60,
	0xBEBFBC70, 0x289B7EC6, 0xEAA127FA, 0xD4EF3085,
	0x04881D05, 0xD9D4D039, 0xE6DB99E5, 0x1FA27CF8,
	0xC4AC5665, 0xF4292244, 0x432AFF97, 0xAB9423A7,
	0xFC93A039, 0x655B59C3, 0x8F0CCC92, 0xFFEFF47D,
	0x85845DD1, 0x6FA87E4F, 0xFE2CE6E0, 0xA3014314,
	0x4E0811A1, 0xF7537E82, 0xBD3AF235, 0x2AD7D2BB,
	0xEB86D391,
}};

constexpr std::array<char, 16> hex_chars{{
	'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'
}};
} // Anonymous namespace

/// Constructor
CMD5::CMD5()
{
}

/// Calculate MD5 Value
SMD5 CMD5::Calculate(LPCTSTR file_path)
{
	// Open file
	CFile file;
	if (!file.OpenForRead(file_path))
		return m_md5_value;

	// Get file size
	const u32 src_size = file.GetFileSize();

	// Get padding
	const u32 padding = CalculatePadding(src_size);

	// Read buffer allocation
	std::vector<u8> src(src_size + padding);

	// Read file
	file.Read(src.data(), src_size);

	// Append padding
	AppendPadding(src.data(), src_size, padding);

	// Calculate MD5
	return Calculate(src.data(), src_size + padding);
}

/// Calculate MD5 Value
///
/// @param src        Input Data
/// @param src_size   Input data size
/// @param initialize Initialization
/// @param alignment  Request alignment
///
/// Remark: If alignment assumes that the input data is aligned on 64 bytes
///         then the efficiency is not good
///
SMD5 CMD5::Calculate(const void* src, size_t src_size, const u32* initialize, bool alignment)
{
	const auto* pdwSrc = static_cast<const u32*>(src);

	// Alignment
	std::vector<u8> clmbtSrc;

	if (alignment)
	{
		// Alignment request
		const size_t padding = CalculatePadding(src_size);
		clmbtSrc.resize(src_size + padding);
		memcpy(&clmbtSrc[0], src, src_size);

		// Append padding
		AppendPadding(&clmbtSrc[0], src_size, padding);

		pdwSrc = reinterpret_cast<const u32*>(&clmbtSrc[0]);
	}

	// Set initial value
	if (initialize == nullptr)
	{
		// Using the default initialization values

		m_md5_value.adwABCD[0] = 0x67452301;
		m_md5_value.adwABCD[1] = 0xEFCDAB89;
		m_md5_value.adwABCD[2] = 0x98BADCFE;
		m_md5_value.adwABCD[3] = 0x10325476;
	}
	else
	{
		m_md5_value.adwABCD[0] = initialize[0];
		m_md5_value.adwABCD[1] = initialize[1];
		m_md5_value.adwABCD[2] = initialize[2];
		m_md5_value.adwABCD[3] = initialize[3];
	}

	for (size_t i = 0, src_ptr = 0; i < src_size; i += 64, src_ptr += 16)
	{
		for (size_t j = 0; j < m_x.size(); j++)
		{
			// Copy
			m_x[j] = pdwSrc[src_ptr + j];
		}

		// Save
		u32 a = m_md5_value.adwABCD[0];
		u32 b = m_md5_value.adwABCD[1];
		u32 c = m_md5_value.adwABCD[2];
		u32 d = m_md5_value.adwABCD[3];

		CalculateSub5(a, b, c, d,  0,  7,  1);
		CalculateSub5(d, a, b, c,  1, 12,  2);
		CalculateSub5(c, d, a, b,  2, 17,  3);
		CalculateSub5(b, c, d, a,  3, 22,  4);
		CalculateSub5(a, b, c, d,  4,  7,  5);
		CalculateSub5(d, a, b, c,  5, 12,  6);
		CalculateSub5(c, d, a, b,  6, 17,  7);
		CalculateSub5(b, c, d, a,  7, 22,  8);
		CalculateSub5(a, b, c, d,  8,  7,  9);
		CalculateSub5(d, a, b, c,  9, 12, 10);
		CalculateSub5(c, d, a, b, 10, 17, 11);
		CalculateSub5(b, c, d, a, 11, 22, 12);
		CalculateSub5(a, b, c, d, 12,  7, 13);
		CalculateSub5(d, a, b, c, 13, 12, 14);
		CalculateSub5(c, d, a, b, 14, 17, 15);
		CalculateSub5(b, c, d, a, 15, 22, 16);

		CalculateSub6(a, b, c, d,  1,  5, 17);
		CalculateSub6(d, a, b, c,  6,  9, 18);
		CalculateSub6(c, d, a, b, 11, 14, 19);
		CalculateSub6(b, c, d, a,  0, 20, 20);
		CalculateSub6(a, b, c, d,  5,  5, 21);
		CalculateSub6(d, a, b, c, 10,  9, 22);
		CalculateSub6(c, d, a, b, 15, 14, 23);
		CalculateSub6(b, c, d, a,  4, 20, 24);
		CalculateSub6(a, b, c, d,  9,  5, 25);
		CalculateSub6(d, a, b, c, 14,  9, 26);
		CalculateSub6(c, d, a, b,  3, 14, 27);
		CalculateSub6(b, c, d, a,  8, 20, 28);
		CalculateSub6(a, b, c, d, 13,  5, 29);
		CalculateSub6(d, a, b, c,  2,  9, 30);
		CalculateSub6(c, d, a, b,  7, 14, 31);
		CalculateSub6(b, c, d, a, 12, 20, 32);

		CalculateSub7(a, b, c, d,  5,  4, 33);
		CalculateSub7(d, a, b, c,  8, 11, 34);
		CalculateSub7(c, d, a, b, 11, 16, 35);
		CalculateSub7(b, c, d, a, 14, 23, 36);
		CalculateSub7(a, b, c, d,  1,  4, 37);
		CalculateSub7(d, a, b, c,  4, 11, 38);
		CalculateSub7(c, d, a, b,  7, 16, 39);
		CalculateSub7(b, c, d, a, 10, 23, 40);
		CalculateSub7(a, b, c, d, 13,  4, 41);
		CalculateSub7(d, a, b, c,  0, 11, 42);
		CalculateSub7(c, d, a, b,  3, 16, 43);
		CalculateSub7(b, c, d, a,  6, 23, 44);
		CalculateSub7(a, b, c, d,  9,  4, 45);
		CalculateSub7(d, a, b, c, 12, 11, 46);
		CalculateSub7(c, d, a, b, 15, 16, 47);
		CalculateSub7(b, c, d, a,  2, 23, 48);

		CalculateSub8(a, b, c, d,  0,  6, 49);
		CalculateSub8(d, a, b, c,  7, 10, 50);
		CalculateSub8(c, d, a, b, 14, 15, 51);
		CalculateSub8(b, c, d, a,  5, 21, 52);
		CalculateSub8(a, b, c, d, 12,  6, 53);
		CalculateSub8(d, a, b, c,  3, 10, 54);
		CalculateSub8(c, d, a, b, 10, 15, 55);
		CalculateSub8(b, c, d, a,  1, 21, 56);
		CalculateSub8(a, b, c, d,  8,  6, 57);
		CalculateSub8(d, a, b, c, 15, 10, 58);
		CalculateSub8(c, d, a, b,  6, 15, 59);
		CalculateSub8(b, c, d, a, 13, 21, 60);
		CalculateSub8(a, b, c, d,  4,  6, 61);
		CalculateSub8(d, a, b, c, 11, 10, 62);
		CalculateSub8(c, d, a, b,  2, 15, 63);
		CalculateSub8(b, c, d, a,  9, 21, 64);

		m_md5_value.adwABCD[0] += a;
		m_md5_value.adwABCD[1] += b;
		m_md5_value.adwABCD[2] += c;
		m_md5_value.adwABCD[3] += d;
	}

	// Convert to a string
	MD5ToStrings(m_md5_value.szABCD, m_md5_value.adwABCD);

	return m_md5_value;
}

/// Calculation Processing
u32 CMD5::CalculateSub1(u32 x, u32 y, u32 z)
{
	return (x & y) | (~x & z);
}

/// Calculation Processing
u32 CMD5::CalculateSub2(u32 x, u32 y, u32 z)
{
	return (x & z) | (y & ~z);
}

/// Calculation Processing
u32 CMD5::CalculateSub3(u32 x, u32 y, u32 z)
{
	return x ^ y ^ z;
}

/// Calculation Processing
u32 CMD5::CalculateSub4(u32 x, u32 y, u32 z)
{
	return y ^ (x | ~z);
}

/// Calculation Processing
void CMD5::CalculateSub5(u32& a, u32 b, u32 c, u32 d, u32 k, u32 s, u32 i)
{
	a = b + RotateLeft(a + CalculateSub1(b, c, d) + m_x[k] + md5_table[i], s);
}

/// Calculation Processing
void CMD5::CalculateSub6(u32& a, u32 b, u32 c, u32 d, u32 k, u32 s, u32 i)
{
	a = b + RotateLeft(a + CalculateSub2(b, c, d) + m_x[k] + md5_table[i], s);
}

/// Calculation Processing
void CMD5::CalculateSub7(u32& a, u32 b, u32 c, u32 d, u32 k, u32 s, u32 i)
{
	a = b + RotateLeft(a + CalculateSub3(b, c, d) + m_x[k] + md5_table[i], s);
}

/// Calculation Processing
void CMD5::CalculateSub8(u32& a, u32 b, u32 c, u32 d, u32 k, u32 s, u32 i)
{
	a = b + RotateLeft(a + CalculateSub4(b, c, d) + m_x[k] + md5_table[i], s);
}

/// Calculate Padding
///
/// @param size Size
///
size_t CMD5::CalculatePadding(size_t size)
{
	// Get padding
	const size_t work = size % 64;
	size_t padding = 64 - work;

	// Increase padding if 8 bytes or less.
	if (padding < 9)
	{
		padding += 64;
	}

	return padding;
}

/// Append Padding
///
/// @param src      Input data
/// @param src_size Input Data Size
/// @param padding  Padding
///
void CMD5::AppendPadding(void* src, size_t src_size, u32 padding)
{
	u8* byte_src = static_cast<u8*>(src);

	// Append padding
	byte_src[src_size] = 0x80;
	std::memset(&byte_src[src_size + 1], 0, padding - 9);

	// Additional data size (in bits)
	const auto bitsize = static_cast<u64>(src_size) * 8;
	std::memcpy(&byte_src[src_size + padding - 8], &bitsize, sizeof(u64));
}

/// Circular Rotate Left
u32 CMD5::RotateLeft(u32 value, u32 shift)
{
	return (value << shift) | (value >> (32 - shift));
}

/// MD5 value converted to a string of decimal digits
///
/// @param md5_dst Storage location of the string.
/// @param md5     MD5 value
///
void CMD5::ValueToStr(char* md5_dst, u32 md5)
{
	for (int i = 0 ; i < 4 ; i++)
	{
		md5_dst[i * 2 + 0] = hex_chars[(md5 >> 4) & 0x0F];
		md5_dst[i * 2 + 1] = hex_chars[md5 & 0x0F];

		md5 >>= 8;
	}
}

/// MD5 value converted to a string of decimal digits
///
/// @param md5_dst Storage location of the string
/// @param md5     MD5 value
///
void CMD5::MD5ToStrings(char* md5_dst, const u32* md5)
{
	for (int i = 0 ; i < 4 ; i++)
	{
		u32 entry = md5[i];

		for (int j = 0 ; j < 4 ; j++)
		{
			md5_dst[i * 8 + j * 2 + 0] = hex_chars[(entry >> 4) & 0x0F];
			md5_dst[i * 8 + j * 2 + 1] = hex_chars[entry & 0x0F];

			entry >>= 8;
		}
	}
}
