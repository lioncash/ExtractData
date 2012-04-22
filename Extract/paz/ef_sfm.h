
#pragma	once

#include	"HaruotoFD.h"

//----------------------------------------------------------------------------------------
//-- ef - Second Fan Mix -----------------------------------------------------------------
//----------------------------------------------------------------------------------------

class	CEFsfm : public CHaruotoFD
{
public:

	virtual	BOOL							Mount( CArcFile* pclArc );
	virtual	BOOL							Decode( CArcFile* pclArc );


protected:

	BOOL									IsSupported( CArcFile* pclArc );

	virtual	void							InitMountKey( CArcFile* pclArc );
	virtual	void							InitDecodeKey( CArcFile* pclArc );

	virtual	DWORD							InitMovieTable( void* pvTable );

	virtual	void							DecodeMovieData( void* pvTarget, DWORD dwSize );

	virtual	DWORD							GetMovieBufSize( CArcFile* pclArc );


private:

	BYTE									m_btKey;
	BYTE									m_aabtMovieTable[256][256];
	DWORD									m_dwMovieTableID;
};
