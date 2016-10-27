#pragma once

class CStandard : public CExtractBase
{
public:
	bool Mount(CArcFile* pclArc) override;
	bool Decode(CArcFile* pclArc) override;
	bool Extract(CArcFile* pclArc) override;

protected:
	virtual BOOL DecodeLZSS(CArcFile* pclArc);
	virtual BOOL DecodeZlib(CArcFile* pclArc);
	virtual BOOL DecodeAhx(CArcFile* pclArc);
	virtual BOOL DecodeImage(CArcFile* pclArc);
	virtual BOOL DecodeOgg(CArcFile* pclArc);
};
