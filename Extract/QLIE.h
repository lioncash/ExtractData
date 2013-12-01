#pragma once

class CQLIE : public CExtractBase
{
private:

	struct SFileNameInfo
	{
		YCString clsFileName; // Filename
		DWORD    dwCount;     // Number of times
	};

	BOOL DecodeB( CArcFile* pclArc, BYTE* pbtSrc, DWORD dwSrcSize );
	BOOL DecodeABMP7( CArcFile* pclArc, BYTE* pbtSrc, DWORD dwSrcSize, DWORD* pdwSrcIndex = NULL, const YCString& rfclsBFileName = _T("") );
	BOOL DecodeABMP10( CArcFile* pclArc, BYTE* pbtSrc, DWORD dwSrcSize, DWORD* pdwSrcIndex = NULL, const YCString& rfclsBFileName = _T("") );

	void Decomp(LPBYTE dst, DWORD dstSize, LPBYTE src, DWORD srcSize);

	static void DecryptFileName(LPBYTE in, DWORD size, DWORD seed);
	static void Decrypt(LPBYTE buf, DWORD buf_len, DWORD seed);

	QWORD padw(QWORD a, QWORD b);
	DWORD crc_or_something(LPBYTE buff, DWORD len);
	static void DecryptFileNameV3(LPBYTE buff, DWORD len, DWORD seed);
	static void DecryptV3(LPBYTE buff, DWORD len, DWORD seed);

	YCString GetExtension( BYTE* pbtSrc );

	void EraseNotUsePathWord( YCString& clsPath );

public:

	enum
	{
		ABIMGDAT10 = 0x00000000,
		ABIMGDAT11,
		ABSNDDAT10,
		ABSNDDAT11,
	};

	BOOL Mount(CArcFile* pclArc);
	BOOL Decode(CArcFile* pclArc);
};
