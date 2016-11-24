#pragma once

class CTga final
{
public:
	struct TGAHeader
	{
		u8  id;
		u8  color_map;
		u8  image_type;
		u8  color_map_info[5];
		u16 origin_x;
		u16 origin_y;
		u16 width;
		u16 height;
		u8  depth;
		u8  descriptor;
	};

	bool Decode(CArcFile* archive, const u8* src, size_t src_size, const YCString& file_last_name = _T(""));

	bool Decomp(u8* dst, size_t dst_size, const u8* src, size_t src_size);

private:
	bool DecompRLE(u8* dst, size_t dst_size, const u8* src, size_t src_size, u8 bpp);
};
