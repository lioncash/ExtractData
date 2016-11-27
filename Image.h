#pragma once

#include "ExtractBase.h"
#include "Image/Bmp.h"
#include "Image/Png.h"

struct SOption;

class CImage : public CExtractBase
{
public:
    CImage();

    bool Mount(CArcFile* archive) override;
    bool Decode(CArcFile* archive) override;

    bool Init(CArcFile* archive, void* bmp, const YCString& file_name = _T(""));
    bool Init(CArcFile* archive, long width, long height, u16 bpp, const void* pallet = nullptr, size_t pallet_size = 1024, const YCString& file_name = _T(""));

    void Close();

    bool Write(size_t bmp_size, bool progress = true);
    bool Write(const void* bmp_data, size_t bmp_data_size, bool progress = true);
    bool WriteReverse(size_t bmp_size, bool progress = true);
    bool WriteReverse(const void* bmp_data, size_t bmp_data_size, bool progress = true);

    bool WriteCompoBGRA(size_t bmp_size, bool progress = true);
    bool WriteCompoBGRA(const void* bmp_data, size_t bmp_data_size, bool progress = true);
    bool WriteCompoBGRAReverse(size_t bmp_size, bool progress = true);
    bool WriteCompoBGRAReverse(const void* bmp_data, size_t bmp_data_size, bool progress = true);

    bool WriteCompoRGBA(size_t bmp_size, bool progress = true);
    bool WriteCompoRGBA(const void* bmp_data, size_t bmp_data_size, bool progress = true);
    bool WriteCompoRGBAReverse(size_t bmp_size, bool progress = true);
    bool WriteCompoRGBAReverse(const void* bmp_data, size_t bmp_data_size, bool progress = true);

    LPBITMAPFILEHEADER GetBmpFileHeader() const;
    LPBITMAPINFOHEADER GetBmpInfoHeader() const;

protected:
    CArcFile* m_archive = nullptr;
    SOption*  m_option = nullptr;

    u8* m_bmp_data = nullptr;
    bool  m_is_valid_bmp_header = true;
    BITMAPFILEHEADER* m_bmp_file_header = nullptr;
    BITMAPINFOHEADER* m_bmp_info_header = nullptr;

    CBmp m_bmp;
    CPng m_png;
};
