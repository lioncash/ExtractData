#pragma once

class CAxl : public CExtractBase
{
private:
	BYTE m_deckey[32];
	DWORD m_len;

public:
	BOOL Mount(CArcFile* pclArc);
	BOOL Decode(CArcFile* pclArc);

	void InitMountKey(LPVOID deckey);
	BOOL CreateKey(LPBYTE key, LPINT key_len, LPBYTE pIndex, DWORD index_size);
	BOOL DecryptIndex(LPBYTE pIndex, DWORD index_size, QWORD arcSize);
};
