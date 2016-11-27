#pragma once

//----------------------------------------------------------------------------------------
//-- TCD Base Class ----------------------------------------------------------------------
//----------------------------------------------------------------------------------------

#include "ExtractBase.h"

class CTCDBase : public CExtractBase
{
public:
	bool Mount(CArcFile* archive) override;
	bool Decode(CArcFile* archive) override;

protected:
	virtual bool DecodeTCT(CArcFile* archive);
	virtual bool DecodeTSF(CArcFile* archive);
	virtual bool DecodeSPD(CArcFile* archive);
	virtual bool DecodeOgg(CArcFile* archive);

	virtual bool DecompLZSS(void* dst, DWORD dst_size, const void* src, DWORD src_size);
	virtual bool DecompRLE0(void* dst, DWORD dst_size, const void* src, DWORD src_size);
	virtual bool DecompRLE2(void* dst, DWORD dst_size, const void* src, DWORD src_size);
	virtual bool DecompSPD(void* dst, DWORD dst_size, const void* src, DWORD src_size, long width);

	virtual bool Decrypt(void* data, DWORD data_size);
};
