#pragma	once

class CNscr : public CExtractBase
{
public:

    BOOL Mount( CArcFile* pclArc );
    BOOL Decode( CArcFile* pclArc );


protected:

    BOOL MountNsa( CArcFile* pclArc );
    BOOL MountSar( CArcFile* pclArc );
    BOOL MountScr( CArcFile* pclArc );

    BOOL DecodeScr( CArcFile* pclArc );
    BOOL DecodeNBZ( CArcFile* pclArc );
    BOOL DecodeSPB( CArcFile* pclArc );
    BOOL DecodeLZSS( CArcFile* pclArc );

    void  GetFileExt( YCString& clsDst, const BYTE* pbtBuffer );
    DWORD GetBit( const BYTE* pbtSrc, DWORD dwReadBitLength, DWORD* pdwReadByteLength );


private:

    BYTE btMaskForGetBit;
    BYTE btSrcForGetBit;
};
