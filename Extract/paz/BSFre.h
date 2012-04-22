
#pragma	once

#include	"Paz.h"

//----------------------------------------------------------------------------------------
//-- BITTERSWEET FOOLS リパッケージ版 ----------------------------------------------------
//----------------------------------------------------------------------------------------

class	CBSFre : public CPaz
{
public:

	virtual	BOOL							Mount( CArcFile* pclArc );
	virtual	BOOL							Decode( CArcFile* pclArc );


protected:

	virtual	void							InitMountKey( CArcFile* pclArc );
	virtual	void							InitDecodeKey( CArcFile* pclArc );
};
