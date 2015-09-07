#pragma once

//----------------------------------------------------------------------------------------
//-- TCD Base Class ----------------------------------------------------------------------
//----------------------------------------------------------------------------------------

#include "../../ExtractBase.h"

class CTCDBase : public CExtractBase
{
public:
	BOOL Mount(CArcFile* pclArc) override;
	BOOL Decode(CArcFile* pclArc) override;

protected:
	virtual BOOL DecodeTCT(CArcFile* pclArc);
	virtual BOOL DecodeTSF(CArcFile* pclArc);
	virtual BOOL DecodeSPD(CArcFile* pclArc);
	virtual BOOL DecodeOgg(CArcFile* pclArc);

	virtual BOOL DecompLZSS(void* pvDst, DWORD dwDstSize, const void* pvSrc, DWORD dwSrcSize);
	virtual BOOL DecompRLE0(void* pvDst, DWORD dwDstSize, const void* pvSrc, DWORD dwSrcSize);
	virtual BOOL DecompRLE2(void* pvDst, DWORD dwDstSize, const void* pvSrc, DWORD dwSrcSize);
	virtual BOOL DecompSPD(void* pvDst, DWORD dwDstSize, const void* pvSrc, DWORD dwSrcSize, long lWidth);

	virtual BOOL Decrypt(void* pvData, DWORD dwDataSize);
};
