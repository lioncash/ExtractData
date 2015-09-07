#pragma once

class CAxl final : public CExtractBase
{
public:
	BOOL Mount(CArcFile* pclArc) override;
	BOOL Decode(CArcFile* pclArc) override;

	void InitMountKey(LPVOID deckey);
	BOOL CreateKey(LPBYTE key, LPINT key_len, LPBYTE pIndex, DWORD index_size);
	BOOL DecryptIndex(LPBYTE pIndex, DWORD index_size, QWORD arcSize);

private:
	BYTE m_deckey[32];
	DWORD m_len;
};
