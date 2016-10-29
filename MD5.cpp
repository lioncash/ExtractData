#include "stdafx.h"
#include "File.h"
#include "MD5.h"

#include <vector>

/// Constructor
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

/// Calculate MD5 Value
SMD5 CMD5::Calculate(LPCTSTR pszPathToFile)
{
	// Open file
	CFile file;
	if (!file.OpenForRead(pszPathToFile))
		return m_stmd5Value;

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
/// Remark: If bAlignment assumes that the input data is aligned on 64 bytes
///         then the efficiency is not good
///
SMD5 CMD5::Calculate(const void* src, u32 src_size, const u32* initialize, bool alignment)
{
	const auto* pdwSrc = static_cast<const u32*>(src);

	// Alignment
	std::vector<u8> clmbtSrc;

	if (alignment)
	{
		// Alignment request
		const u32 padding = CalculatePadding(src_size);
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

		m_stmd5Value.adwABCD[0] = 0x67452301;
		m_stmd5Value.adwABCD[1] = 0xEFCDAB89;
		m_stmd5Value.adwABCD[2] = 0x98BADCFE;
		m_stmd5Value.adwABCD[3] = 0x10325476;
	}
	else
	{
		m_stmd5Value.adwABCD[0] = initialize[0];
		m_stmd5Value.adwABCD[1] = initialize[1];
		m_stmd5Value.adwABCD[2] = initialize[2];
		m_stmd5Value.adwABCD[3] = initialize[3];
	}

	for (u32 i = 0, src_ptr = 0; i < src_size; i += 64, src_ptr += 16)
	{
		for (int j = 0; j < 16; j++)
		{
			// Copy

			m_adwX[j] = pdwSrc[src_ptr + j];
		}

		// Save
		u32 a = m_stmd5Value.adwABCD[0];
		u32 b = m_stmd5Value.adwABCD[1];
		u32 c = m_stmd5Value.adwABCD[2];
		u32 d = m_stmd5Value.adwABCD[3];

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

		m_stmd5Value.adwABCD[0] += a;
		m_stmd5Value.adwABCD[1] += b;
		m_stmd5Value.adwABCD[2] += c;
		m_stmd5Value.adwABCD[3] += d;
	}

	// Convert to a string
	MD5ToStrings(m_stmd5Value.szABCD, m_stmd5Value.adwABCD);

	return m_stmd5Value;
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
	a = b + RotateLeft(a + CalculateSub1(b, c, d) + m_adwX[k] + m_anTable[i], s);
}

/// Calculation Processing
void CMD5::CalculateSub6(u32& a, u32 b, u32 c, u32 d, u32 k, u32 s, u32 i)
{
	a = b + RotateLeft(a + CalculateSub2(b, c, d) + m_adwX[k] + m_anTable[i], s);
}

/// Calculation Processing
void CMD5::CalculateSub7(u32& a, u32 b, u32 c, u32 d, u32 k, u32 s, u32 i)
{
	a = b + RotateLeft(a + CalculateSub3(b, c, d) + m_adwX[k] + m_anTable[i], s);
}

/// Calculation Processing
void CMD5::CalculateSub8(u32& a, u32 b, u32 c, u32 d, u32 k, u32 s, u32 i)
{
	a = b + RotateLeft(a + CalculateSub4(b, c, d) + m_adwX[k] + m_anTable[i], s);
}

/// Calculate Padding
///
/// @param size Size
///
u32 CMD5::CalculatePadding(u32 size)
{
	// Get padding
	const u32 work = size % 64;
	u32 padding = 64 - work;

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
void CMD5::AppendPadding(void* src, u32 src_size, u32 padding)
{
	u8* byte_src = static_cast<u8*>(src);

	// Append padding
	byte_src[src_size] = 0x80;
	ZeroMemory(&byte_src[src_size + 1], padding - 9);

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
void CMD5::ValueToStr(LPSTR md5_dst, u32 md5)
{
	static const char acHex[16] =
	{
		'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'
	};

	for (int i = 0 ; i < 4 ; i++)
	{
		md5_dst[i * 2 + 0] = acHex[((md5 >> 4) & 0x0F)];
		md5_dst[i * 2 + 1] = acHex[(md5 & 0x0F)];

		md5 >>= 8;
	}
}

/// MD5 value converted to a string of decimal digits
///
/// @param md5_dst Storage location of the string
/// @param md5     MD5 value
///
void CMD5::MD5ToStrings(LPSTR md5_dst, const u32* md5)
{
	static const char acHex[16] =
	{
		'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'
	};

	for (int i = 0 ; i < 4 ; i++)
	{
		u32 entry = md5[i];

		for (int j = 0 ; j < 4 ; j++)
		{
			md5_dst[i * 8 + j * 2 + 0] = acHex[((entry >> 4) & 0x0F)];
			md5_dst[i * 8 + j * 2 + 1] = acHex[(entry & 0x0F)];

			entry >>= 8;
		}
	}
}
