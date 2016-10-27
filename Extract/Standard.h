#pragma once

class CStandard : public CExtractBase
{
public:
	bool Mount(CArcFile* pclArc) override;
	bool Decode(CArcFile* pclArc) override;
	bool Extract(CArcFile* pclArc) override;

protected:
	virtual bool DecodeLZSS(CArcFile* pclArc);
	virtual bool DecodeZlib(CArcFile* pclArc);
	virtual bool DecodeAhx(CArcFile* pclArc);
	virtual bool DecodeImage(CArcFile* pclArc);
	virtual bool DecodeOgg(CArcFile* pclArc);
};
