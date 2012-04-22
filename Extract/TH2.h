
#pragma		once

class	CTH2 : public CExtractBase
{
public:

	BOOL									Mount( CArcFile* pclArc );
	BOOL									Decode( CArcFile* pclArc );


protected:

	BOOL									MountKCAP( CArcFile* pclArc );
	BOOL									MountLAC( CArcFile* pclArc );
	BOOL									MountDpl( CArcFile* pclArc );
	BOOL									MountWMV( CArcFile* pclArc );

	BOOL									DecodeWMV( CArcFile* pclArc );
	BOOL									DecodeEtc( CArcFile* pclArc );
};
