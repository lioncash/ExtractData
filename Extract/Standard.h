#pragma once

#include "../ExtractBase.h"

class CStandard : public CExtractBase
{
public:
	bool Mount(CArcFile* archive) override;
	bool Decode(CArcFile* archive) override;
	bool Extract(CArcFile* archive) override;

protected:
	virtual bool DecodeLZSS(CArcFile* archive);
	virtual bool DecodeZlib(CArcFile* archive);
	virtual bool DecodeAhx(CArcFile* archive);
	virtual bool DecodeImage(CArcFile* archive);
	virtual bool DecodeOgg(CArcFile* archive);
};
