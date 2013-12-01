#pragma once

class CStandard : public CExtractBase
{
public:

    virtual BOOL Mount( CArcFile* pclArc );
    virtual BOOL Decode( CArcFile* pclArc );
    virtual BOOL Extract( CArcFile* pclArc );


protected:

    virtual BOOL DecodeLZSS( CArcFile* pclArc );
    virtual BOOL DecodeZlib( CArcFile* pclArc );
    virtual BOOL DecodeAhx( CArcFile* pclArc );
    virtual BOOL DecodeImage( CArcFile* pclArc );
    virtual BOOL DecodeOgg( CArcFile* pclArc );
};
