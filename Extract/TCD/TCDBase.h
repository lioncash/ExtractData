#pragma once

//----------------------------------------------------------------------------------------
//-- TCD Base Class ----------------------------------------------------------------------
//----------------------------------------------------------------------------------------

#include "../../ExtractBase.h"

class CTCDBase : public CExtractBase
{
public:
	bool Mount(CArcFile* pclArc) override;
	bool Decode(CArcFile* pclArc) override;

protected:
	virtual bool DecodeTCT(CArcFile* pclArc);
	virtual bool DecodeTSF(CArcFile* pclArc);
	virtual bool DecodeSPD(CArcFile* pclArc);
	virtual bool DecodeOgg(CArcFile* pclArc);

	virtual bool DecompLZSS(void* pvDst, DWORD dwDstSize, const void* pvSrc, DWORD dwSrcSize);
	virtual bool DecompRLE0(void* pvDst, DWORD dwDstSize, const void* pvSrc, DWORD dwSrcSize);
	virtual bool DecompRLE2(void* pvDst, DWORD dwDstSize, const void* pvSrc, DWORD dwSrcSize);
	virtual bool DecompSPD(void* pvDst, DWORD dwDstSize, const void* pvSrc, DWORD dwSrcSize, long lWidth);

	virtual bool Decrypt(void* pvData, DWORD dwDataSize);
};
