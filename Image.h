#pragma	once

#include "Image/Bmp.h"
#include "Image/Png.h"

class CImage : public CExtractBase
{
public:
    CImage();

    bool Mount(CArcFile* pclArc) override;
    bool Decode(CArcFile* pclArc) override;

    bool Init(CArcFile* pclArc, void* pvBMP, const YCString& rfclsFileName = _T(""));
    bool Init(CArcFile* pclArc, long lWidth, long lHeight, WORD wBpp, const void* pvPallet = nullptr, DWORD dwPalletSize = 1024, const YCString& rfclsFileName = _T(""));

    void Close();

    bool Write(DWORD dwBMPSize, bool progress = true);
    bool Write(const void* pvBMPData, DWORD dwBMPDataSize, bool progress = true);
    bool WriteReverse(DWORD dwBMPSize, bool progress = true);
    bool WriteReverse(const void* pvBMPData, DWORD dwBMPDataSize, bool progress = true);

    //void Compose(LPBYTE buf, DWORD buf_len);
    bool WriteCompoBGRA(DWORD dwBMPSize, bool progress = true);
    bool WriteCompoBGRA(const void* pvBMPData, DWORD dwBMPDataSize, bool progress = true);
    bool WriteCompoBGRAReverse(DWORD dwBMPSize, bool progress = true);
    bool WriteCompoBGRAReverse(const void* pvBMPData, DWORD dwBMPDataSize, bool progress = true);

    //void ComposeRGBA(LPBYTE buf, DWORD buf_len);
    bool WriteCompoRGBA(DWORD dwBMPSize, bool progress = true);
    bool WriteCompoRGBA(const void* pvBMPData, DWORD dwBMPDataSize, bool progress = true);
    bool WriteCompoRGBAReverse(DWORD dwBMPSize, bool progress = true);
    bool WriteCompoRGBAReverse(const void* pvBMPData, DWORD dwBMPDataSize, bool progress = true);

    LPBITMAPFILEHEADER GetBmpFileHeader();
    LPBITMAPINFOHEADER GetBmpInfoHeader();


protected:
    CArcFile* m_pclArc = nullptr;
    SOption*  m_pstOption = nullptr;

    BYTE* m_pbtBMP = nullptr;
    bool  m_bBMPHeader = true;
    BITMAPFILEHEADER* m_pstBMPFileHeader = nullptr;
    BITMAPINFOHEADER* m_pstBMPInfoHeader = nullptr;

    CBmp m_clBMP;
    CPng m_clPNG;
};
