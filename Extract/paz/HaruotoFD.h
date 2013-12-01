#pragma once

#include "Paz.h"

class CHaruotoFD : public CPaz
{
public:

	virtual BOOL Mount( CArcFile* pclArc );
	virtual BOOL Decode( CArcFile* pclArc );


protected:

	virtual void InitMountKey( CArcFile* pclArc );
	virtual void InitDecodeKey( CArcFile* pclArc );

	virtual void DecodeTable2();

	virtual void DecodeData( void* pvTarget, DWORD dwSize );
};
