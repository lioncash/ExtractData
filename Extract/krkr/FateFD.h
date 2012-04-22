
#pragma	once

#include	"Krkr.h"

class	CFateFD : public CKrkr
{
protected:

	virtual	BOOL							OnCheckDecrypt( CArcFile* pclArc );
	virtual	DWORD							OnInitDecrypt( CArcFile* pclArc );
	virtual	DWORD							OnDecrypt( BYTE* pvTarget, DWORD dwTargetSize, DWORD dwOffset, DWORD dwDecryptKey );


private:

	BYTE									m_abtKey[6];
	DWORD									m_adwOffset[6];

	// デコード情報
	struct DecodeInfo {
		BYTE key;
		DWORD size;
	};
};
