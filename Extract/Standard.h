#pragma once

class CStandard : public CExtractBase
{
public:
	BOOL Mount(CArcFile* pclArc) override;
	BOOL Decode(CArcFile* pclArc) override;
	BOOL Extract(CArcFile* pclArc) override;


protected:
	virtual BOOL DecodeLZSS(CArcFile* pclArc);
	virtual BOOL DecodeZlib(CArcFile* pclArc);
	virtual BOOL DecodeAhx(CArcFile* pclArc);
	virtual BOOL DecodeImage(CArcFile* pclArc);
	virtual BOOL DecodeOgg(CArcFile* pclArc);
};
