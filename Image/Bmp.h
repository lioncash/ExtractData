#pragma once

#include "ImageBase.h"

class CBmp : public CImageBase
{
public:

	BOOL                Mount( CArcFile* pclArc );


protected:

	BITMAPFILEHEADER    m_stBMPFileHeader;
	BITMAPINFOHEADER    m_stBMPInfoHeader;
	RGBQUAD             m_astPallet[256];


protected:

	virtual BOOL        OnInit( const YCString& rfclsFileName );
	virtual BOOL        OnCreatePallet( const void* pvPallet, DWORD dwPalletSize );

	void                WriteHed( const YCString& rfclsFileName );
	virtual void        WriteLine( const void* pvBuffer );
	virtual void        WriteLineWithAlphaBlend( void* pvBuffer24, const void* pvBuffer32 );
};
