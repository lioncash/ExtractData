#pragma once

#include "../../ExtractBase.h"

class CPaz : public CExtractBase
{
public:
	// Key Info
	struct SKeyInfo
	{
		YCString clsType;
		BYTE     abtKey[64];
	};

	BOOL Mount(CArcFile* pclArc) override;
	BOOL Decode(CArcFile* pclArc) override;

protected:
	// Initialization of the table of 72 + 4096 bytes
	virtual void InitTable();

	// Initialization of the movie table
	virtual DWORD InitMovieTable(void* pvTable);

	// Get the base archive filename
	void GetBaseArcName(LPTSTR pszDst, LPCTSTR pszArcName);

	// Set 32 byte keys (Keys differ for each file)
	virtual void InitMountKey(CArcFile* pclArc) = 0;
	virtual void InitDecodeKey(CArcFile* pclArc) = 0;
	DWORD SetKey(CArcFile* pclArc, const SKeyInfo* pstKeyInfo);

	// Decrypt the table
	virtual void DecodeTable1();
	virtual void DecodeTable2();

	// Cipher-specific decryption
	virtual void Decrypt(void* pvTarget, DWORD dwSize) {}
	virtual void Decrypt2(void* pvTarget, DWORD dwSize) {}

	// Decrypt the data
	virtual void DecodeData(void* pvTarget, DWORD dwSize);

	// Decrypt movie data
	virtual void DecodeMovieData(void* pvTarget, DWORD dwSize);

	// Decode DWORD value
	virtual DWORD DecodeValueByTable(DWORD dwValue, void* pvTable);
	virtual void  DecodeValue(DWORD* pdwValue1, DWORD* pdwValue2, void* pvTable);

	// Get 
	virtual DWORD* GetTable();
	virtual BYTE*  GetMovieTable();

	virtual BYTE* GetKey();
	virtual DWORD GetMovieBufSize(CArcFile* pclArc);

private:

	DWORD m_adwTable[1042];
	BYTE  m_abtMovieTable[256];
	BYTE  m_abtKey[32];
};
