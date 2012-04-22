
#pragma	once

#include	"HaruotoFD.h"

class	CEFLatter : public CHaruotoFD
{
public:

	virtual	BOOL							Mount( CArcFile* pclArc );
	virtual	BOOL							Decode( CArcFile* pclArc );


protected:

	BOOL									IsSupported( CArcFile* pclArc );

	virtual	void							InitMountKey( CArcFile* pclArc );
	virtual	void							InitDecodeKey( CArcFile* pclArc );

	virtual	DWORD							InitMovieTable( void* pvTable );

	virtual	void							Decrypt( void* pvTarget, DWORD dwSize );
	virtual	void							Decrypt2( void * pvTarget, DWORD dwSize );

	void									SetDecryptKey2( CArcFile* pclArc );

	virtual	void							DecodeMovieData( void* pvTarget, DWORD dwSize );


private:

	BYTE									m_btKey;

	char									m_szKey[256];
	BYTE									m_abtKeyTable[256];
	DWORD									m_dwKeyTableIndex1;
	DWORD									m_dwKeyTableIndex2;

	BYTE									m_abtMovieDecodeTable[65536];
	DWORD									m_dwMovieDecodeTablePtr;
};
