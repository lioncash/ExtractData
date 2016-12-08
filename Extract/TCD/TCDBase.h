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

	virtual bool DecompLZSS(u8* dst, size_t dst_size, const u8* src, size_t src_size);
	virtual bool DecompRLE0(u8* dst, size_t dst_size, const u8* src, size_t src_size);
	virtual bool DecompRLE2(u8* dst, size_t dst_size, const u8* src, size_t src_size);
	virtual bool DecompSPD(u8* dst, size_t dst_size, const u8* src, size_t src_size, s32 width);

	virtual bool Decrypt(u8* data, size_t data_size);
};
