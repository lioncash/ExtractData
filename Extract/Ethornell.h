#pragma once

class CEthornell : public CExtractBase
{
public:

	struct SNodeInfo
	{
		BOOL				bValidity;					// Validity
		DWORD				dwFreq;						// Frequency
		DWORD				dwLeft;						// Left Node
		DWORD				dwRight;					// Right Node
	};

	BOOL					Mount( CArcFile* pclArc );
	BOOL					Decode( CArcFile* pclArc );


protected:

	BOOL					DecodeDSC( CArcFile* pclArc );
	BOOL					DecodeCBG( CArcFile* pclArc );
	BOOL					DecodeStd( CArcFile* pclArc );

	DWORD					GetKey( DWORD* dwKey );
	DWORD					GetVariableData( const BYTE* pbtSrc, DWORD* pdwDstOfReadLength );

	void					DecompDSC( BYTE* pbtDst, DWORD dwDstSize, const BYTE* pbtSrc, DWORD dwSrcSize );
	void					DecompCBG( BYTE* pbtDst, BYTE* pbtSrc );

	void					DecryptBGType1( BYTE* pbtDst, BYTE* pbtSrc, long lWidth, long lHeight, WORD wBpp );
};
