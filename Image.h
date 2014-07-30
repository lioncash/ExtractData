#pragma	once

#include "Image/Bmp.h"
#include "Image/Png.h"

class CImage : public CExtractBase
{
public:

                                            CImage();

    virtual	BOOL                            Mount(CArcFile* pclArc);
    virtual	BOOL                            Decode(CArcFile* pclArc);

    BOOL                                    Init(CArcFile* pclArc, void* pvBMP, const YCString& rfclsFileName = _T(""));
    BOOL                                    Init(CArcFile* pclArc, long lWidth, long lHeight, WORD wBpp, const void* pvPallet = NULL, DWORD dwPalletSize = 1024, const YCString& rfclsFileName = _T(""));

    void                                    Close();

    BOOL                                    Write(DWORD dwBMPSize, BOOL bProgress = TRUE);
    BOOL                                    Write(const void* pvBMPData, DWORD dwBMPDataSize, BOOL bProgress = TRUE);
    BOOL                                    WriteReverse(DWORD dwBMPSize, BOOL bProgress = TRUE);
    BOOL                                    WriteReverse(const void* pvBMPData, DWORD dwBMPDataSize, BOOL bProgress = TRUE);

    //void Compose(LPBYTE buf, DWORD buf_len);
    BOOL                                    WriteCompoBGRA(DWORD dwBMPSize, BOOL bProgress = TRUE);
    BOOL                                    WriteCompoBGRA(const void* pvBMPData, DWORD dwBMPDataSize, BOOL bProgress = TRUE);
    BOOL                                    WriteCompoBGRAReverse(DWORD dwBMPSize, BOOL bProgress = TRUE);
    BOOL                                    WriteCompoBGRAReverse(const void* pvBMPData, DWORD dwBMPDataSize, BOOL bProgress = TRUE);

    //void ComposeRGBA(LPBYTE buf, DWORD buf_len);
    BOOL                                    WriteCompoRGBA(DWORD dwBMPSize, BOOL bProgress = TRUE);
    BOOL                                    WriteCompoRGBA(const void* pvBMPData, DWORD dwBMPDataSize, BOOL bProgress = TRUE);
    BOOL                                    WriteCompoRGBAReverse(DWORD dwBMPSize, BOOL bProgress = TRUE);
    BOOL                                    WriteCompoRGBAReverse(const void* pvBMPData, DWORD dwBMPDataSize, BOOL bProgress = TRUE);

    LPBITMAPFILEHEADER                      GetBmpFileHeader();
    LPBITMAPINFOHEADER                      GetBmpInfoHeader();


protected:

    CArcFile*                               m_pclArc;
    SOption*                                m_pstOption;

    BYTE*                                   m_pbtBMP;
    BOOL                                    m_bBMPHeader;
    BITMAPFILEHEADER*                       m_pstBMPFileHeader;
    BITMAPINFOHEADER*                       m_pstBMPInfoHeader;

    CBmp                                    m_clBMP;
    CPng                                    m_clPNG;

    void                                    WriteLine(const BYTE* pbtBuffer);
    void                                    WriteLine32to24(const BYTE* pbtBuffer, const BYTE* pbtBuffer24);
};
