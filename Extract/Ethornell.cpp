#include "StdAfx.h"
#include "Extract/Ethornell.h"

#include "ArcFile.h"
#include "Image.h"

/// Mount
///
/// @param archive Archive
///
bool CEthornell::Mount(CArcFile* archive)
{
	if (archive->GetArcExten() != _T(".arc"))
		return false;

	if (memcmp(archive->GetHeader(), "PackFile    ", 12) != 0 &&
	    memcmp(archive->GetHeader(), "BURIKO ARC20", 12) != 0)
	{
		return false;
	}

	archive->SeekHed(12);

	// Get file count
	u32 num_files;
	archive->ReadU32(&num_files);

	// Get index size
	const size_t info_size =
		memcmp(archive->GetHeader(), "PackFile    ", 12) == 0
		? 32
		: 128;

	const size_t index_size = info_size * num_files;

	// Get index
	std::vector<u8> index(index_size);
	archive->Read(index.data(), index.size());

	// Get offset
	const u32 offset = 16 + index.size();

	// Get file info
	const size_t name_size = 
		memcmp(archive->GetHeader(), "PackFile    ", 12) == 0
		? 16
		: 96;
	for (size_t i = 0; i < index.size(); i += info_size)
	{
		SFileInfo file_info;
		file_info.name.Copy(reinterpret_cast<const char*>(&index[i]), name_size);
		file_info.start = *reinterpret_cast<const u32*>(&index[i + name_size]) + offset;
		file_info.sizeCmp = *reinterpret_cast<const u32*>(&index[i + name_size + sizeof(u32)]);
		file_info.sizeOrg = file_info.sizeCmp;
		file_info.end = file_info.start + file_info.sizeCmp;

		archive->AddFileInfo(file_info);
	}

	return true;
}

/// Decode
///
/// @param archive Archive
///
bool CEthornell::Decode(CArcFile* archive)
{
	u8 header[16];
	archive->Read(header, sizeof(header));
	archive->SeekCur(-(int)sizeof(header));

	// DSC
	if (memcmp(header, "DSC FORMAT 1.00\0", 16) == 0)
		return DecodeDSC(archive);

	// CompressedBG
	if (memcmp(header, "CompressedBG___\0", 16) == 0)
		return DecodeCBG(archive);

	// Other
	return DecodeStd(archive);
}

/// Decode DSC
///
/// @param archive Archive
///
bool CEthornell::DecodeDSC(CArcFile* archive)
{
	const SFileInfo* file_info = archive->GetOpenFileInfo();

	// Read
	std::vector<u8> src(file_info->sizeCmp);
	archive->Read(src.data(), src.size());

	// Ensure output buffer
	const u32 dst_size = *(u32*)&src[20];
	std::vector<u8> dst(dst_size);

	// Decompress DSC
	DecompDSC(dst.data(), dst.size(), src.data(), src.size());

	// Get image information
	const s32 width  = *(u16*)&dst[0];
	const s32 height = *(u16*)&dst[2];
	const u16 bpp    = *(u16*)&dst[4];
	const u16 flags  = *(u16*)&dst[6];
	const u32 offset = *(u32*)&dst[0];

	// Output

	if ((bpp == 8 || bpp == 24 || bpp == 32) && memcmp(&dst[8], "\0\0\0\0\0\0\0\0", 8) == 0)
	{
		// Image
		CImage          image;
		std::vector<u8> dst2;

		switch (flags)
		{
		case 0: // Common
			image.Init(archive, width, height, bpp);
			image.WriteReverse(&dst[16], dst.size() - 16);
			break;

		case 1:  // Type 1 encryption
			dst2.resize(dst.size() - 16);

			// Decryption
			DecryptBGType1(dst2.data(), &dst[16], width, height, bpp);

			// Output
			image.Init(archive, width, height, bpp);
			image.WriteReverse(dst2.data(), dst2.size());
			break;

		default: // Unknown Format
			archive->OpenFile();
			archive->WriteFile(dst.data(), dst.size(), src.size());
			break;
		}
	}
	else if (offset < dst.size() - 4 && memcmp(&dst[offset], "OggS", 4) == 0)
	{
		// Ogg Vorbis
		archive->OpenFile(_T(".ogg"));
		archive->WriteFile(&dst[offset], dst.size() - offset, src.size());
	}
	else
	{
		// Other
		archive->OpenFile();
		archive->WriteFile(dst.data(), dst.size(), src.size());
	}

	return true;
}

/// Decode CompressedBG
///
/// @param archive Archive
///
bool CEthornell::DecodeCBG(CArcFile* archive)
{
	const SFileInfo* file_info = archive->GetOpenFileInfo();

	// Read CompressedBG
	std::vector<u8> src(file_info->sizeCmp);
	archive->Read(src.data(), src.size());

	// Width, Height, Get number of colors
	const s32 width = *reinterpret_cast<const u16*>(&src[16]);
	const s32 height = *reinterpret_cast<const u16*>(&src[18]);
	const u16 bpp = *reinterpret_cast<const u16*>(&src[20]);

	// Ensure output buffer
	const u32 dst_size = width * height * (bpp >> 3);
	std::vector<u8> dst(dst_size);

	// CompressedBG Decompression
	DecompCBG(dst.data(), src.data());

	// Output Image
	CImage image;
	image.Init(archive, width, height, bpp);
	image.WriteReverse(dst.data(), dst.size());

	return true;
}

// Decode other files
///
/// @param archive Archive
///
bool CEthornell::DecodeStd(CArcFile* archive)
{
	// Get offset
	u32 offset;
	archive->ReadU32(&offset);

	// Check file header
	u8 header[4] = {};
	if (archive->GetArcPointer() + offset < archive->GetArcSize())
	{
		// Seek possible file offset value
		archive->SeekCur(offset - 4);
		archive->Read(header, sizeof(header));
	}
	else
	{
		// Cannot find a file offset value (Not an offset value)
		offset = 0;
	}

	// Output
	if (memcmp(header, "OggS", 4) == 0)
	{
		// Ogg Vorbis
		archive->SeekHed(archive->GetOpenFileInfo()->start + offset);
		archive->OpenFile(_T(".ogg"));
	}
	else
	{
		// Other
		offset = 0;
		archive->SeekHed(archive->GetOpenFileInfo()->start);
		archive->OpenFile();
	}

	archive->ReadWrite(archive->GetOpenFileInfo()->sizeOrg - offset);

	return true;
}

/// Get Key
///
/// @param key Input data and output
///
u32 CEthornell::GetKey(u32* key)
{
	const u32 work1 = 20021 * (*key & 0xFFFF);
	const u32 work2 = 20021 * (*key >> 16);
	const u32 work  = 346 * (*key) + work2 + (work1 >> 16);

	*key = (work << 16) + (work1 & 0xFFFF) + 1;

	return work & 0x7FFF;
}

/// Get variable-length data
///
/// @param src         Input data
/// @param read_length Length of the data read
///
u32 CEthornell::GetVariableData(const u8* src, size_t* read_length)
{
	size_t src_ptr = 0;
	u8 current_src;

	u32 data = 0;
	u32 shift = 0;

	do
	{
		current_src = src[src_ptr++];
		data |= (current_src & 0x7F) << shift;
		shift += 7;
	} while (current_src & 0x80);

	if (read_length != nullptr)
	{
		*read_length = src_ptr;
	}

	return data;
}

/// DSC Decompression
///
/// @param dst      Destination
/// @param dst_size Destination size
/// @param src      Compressed data
/// @param src_size Compressed data size
///
void CEthornell::DecompDSC(u8* dst, size_t dst_size, const u8* src, size_t src_size)
{
	size_t src_ptr = 32;
	size_t dst_ptr = 0;

	u32 buffer[512 + 1] = {};
	u32 buffer2[1024] = {};
	u8  buffer3[0x3FF0] = {};

	u32 key = *reinterpret_cast<const u32*>(&src[16]);
	u32 buffer_size = 0;

	for (u32 i = 0; i < 512; i++)
	{
		const u8 work = src[src_ptr] - static_cast<u8>(GetKey(&key));

		if (work != 0)
		{
			buffer[buffer_size++] = (work << 16) + i;
		}

		src_ptr++;
	}
	buffer[buffer_size] = 0;

	// Sort
	for (size_t i = 0; i < buffer_size - 1; i++)
	{
		for (size_t j = i + 1; j < buffer_size; j++)
		{
			if (buffer[i] > buffer[j])
			{
				std::swap(buffer[i], buffer[j]);
			}
		}
	}

	// 

	int min = 0;
	int max = 1;
	size_t buffer_idx = 0;
	size_t buffer2_idx = 0;
	size_t buffer2_idx_prev = 0x200;
	u32 code = 1;

	for (size_t index = 0; buffer_idx < buffer_size; index++)
	{
		if (index & 1)
		{
			buffer2_idx = 0;
			buffer2_idx_prev = 512;
		}
		else
		{
			buffer2_idx = 512;
			buffer2_idx_prev = 0;
		}

		min = 0;

		while ((buffer[buffer_idx] >> 16) == index)
		{
			u32* buffer3_ptr = (u32*)&buffer3[buffer2[buffer2_idx_prev] << 4];

			buffer3_ptr[0] = 0;
			buffer3_ptr[1] = buffer[buffer_idx] & 0x1FF;

			buffer_idx++;
			buffer2_idx_prev++;

			min++;
		}

		for (int i = min; i < max; i++)
		{
			u32*   buffer3_ptr = (u32*)&buffer3[buffer2[buffer2_idx_prev] << 4];
			size_t buffer3_idx = 0;

			buffer3_ptr[buffer3_idx] = 1;
			buffer3_idx += 2;

			buffer2[buffer2_idx] = code;
			buffer3_ptr[buffer3_idx] = code;
			buffer2_idx++;
			buffer3_idx++;
			code++;

			buffer2[buffer2_idx] = code;
			buffer3_ptr[buffer3_idx] = code;
			buffer2_idx++;
			buffer3_idx++;
			code++;

			buffer2_idx_prev++;
		}

		max = (max - min) * 2;
	}

	//

	const u32 size = *reinterpret_cast<const u32*>(&src[24]);
	u32 src_val = 0;
	u32 count = 0;

	for (size_t i = 0; i < size && src_ptr < src_size && dst_ptr < dst_size; i++)
	{
		u32 index = 0;

		do
		{
			if (count == 0)
			{
				src_val = src[src_ptr++];
				count = 8;
			}

			index = index * 4 + ((src_val & 0xFF) >> 7);
			src_val <<= 1;
			count--;

			index = *(u32*)&buffer3[4 * index + 8];
		} while (*(u32*)&buffer3[index << 4] != 0);

		index <<= 4;
		const u8 btWork = buffer3[index + 4];

		if (buffer3[index + 5] == 1)
		{
			u32 bit_buffer = (src_val & 0xFF) >> (8 - count);
			u32 bit_count = count;

			if (count < 12)
			{
				const u32 end = (19 - count) >> 3;
				bit_count = count + 8 * end;

				for (u32 j = 0; j < end; j++)
				{
					bit_buffer = (bit_buffer << 8) + src[src_ptr++];
				}
			}

			count = bit_count - 12;
			src_val = bit_buffer << (8 - count);
			const u32 back = ((bit_buffer >> count) & 0xFFFF) + 2;
			const u32 length = btWork + 2;

			if (back > dst_ptr || dst_ptr >= dst_size)
			{
				break;
			}

			for (size_t j = 0; j < length; j++)
			{
				dst[dst_ptr + j] = dst[dst_ptr + j - back];
			}

			dst_ptr += length;
		}
		else
		{
			dst[dst_ptr++] = btWork;
		}
	}
}

/// CompressedBG Decompression
///
/// @param dst Destination
/// @param src Compressed data
///
void CEthornell::DecompCBG(u8* dst, u8* src)
{
	size_t src_ptr = 48;
	size_t dst_ptr = 0;

	const s32 width = *reinterpret_cast<const u16*>(&src[16]);
	const s32 height = *reinterpret_cast<const u16*>(&src[18]);
	const u16 bpp = *reinterpret_cast<const u16*>(&src[20]);

	const u32 huffman_dst_size = *reinterpret_cast<const u32*>(&src[32]);
	u32 key = *reinterpret_cast<const u32*>(&src[36]);
	const u32 decrypt_size = *reinterpret_cast<const u32*>(&src[40]);

	// Decryption
	for (size_t i = 0; i < decrypt_size; i++)
	{
		src[src_ptr + i] -= static_cast<u8>(GetKey(&key));
	}

	// Get frequency table
	std::array<u32, 256> freq_table;
	for (auto& freq : freq_table)
	{
		size_t read_length;
		freq = GetVariableData(&src[src_ptr], &read_length);

		src_ptr += read_length;
	}

	// Leaf node entry
	std::array<NodeInfo, 511> node_info;
	u32 freq_total = 0;

	for (size_t i = 0; i < 256; i++)
	{
		node_info[i].is_valid = freq_table[i] > 0;
		node_info[i].freq = freq_table[i];
		node_info[i].left = static_cast<u32>(i);
		node_info[i].right = static_cast<u32>(i);

		freq_total += freq_table[i];
	}

	// Initialization of the branch node
	for (size_t i = 256; i < 511; i++)
	{
		node_info[i].is_valid = false;
		node_info[i].freq = 0;
		node_info[i].left = 0xFFFFFFFF;
		node_info[i].right = 0xFFFFFFFF;
	}

	// Branch node entry
	u32 nodes;
	for (nodes = 256; nodes < 511; nodes++)
	{
		// Obtain value of minimum two
		u32 freq = 0;
		std::array<u32, 2> children;

		for (auto& child : children)
		{
			u32 min = 0xFFFFFFFF;
			child = 0xFFFFFFFF;

			for (u32 j = 0; j < nodes; j++)
			{
				if (node_info[j].is_valid && node_info[j].freq < min)
				{
					min = node_info[j].freq;
					child = j;
				}
			}

			if (child != 0xFFFFFFFF)
			{
				node_info[child].is_valid = false;

				freq += node_info[child].freq;
			}
		}

		// Registration of branch node 
		node_info[nodes].is_valid = true;
		node_info[nodes].freq = freq;
		node_info[nodes].left = children[0];
		node_info[nodes].right = children[1];

		if (freq == freq_total)
		{
			// Exit
			break;
		}
	}

	// Huffman Decompression
	const u32 root = nodes;
	u32 mask = 0x80;

	std::vector<u8> huffman_dst(huffman_dst_size);

	for (size_t i = 0; i < huffman_dst.size(); i++)
	{
		u32 node = root;

		while (node >= 256)
		{
			if (src[src_ptr] & mask)
			{
				node = node_info[node].right;
			}
			else
			{
				node = node_info[node].left;
			}

			mask >>= 1;

			if (mask == 0)
			{
				src_ptr++;
				mask = 0x80;
			}
		}

		huffman_dst[i] = static_cast<u8>(node);
	}

	// RLE Decompression
	size_t huffman_dst_ptr = 0;
	u8 zero_flag = 0;

	while (huffman_dst_ptr < huffman_dst.size())
	{
		size_t read_length;
		const u32 length = GetVariableData(&huffman_dst[huffman_dst_ptr], &read_length);
		huffman_dst_ptr += read_length;

		if (zero_flag)
		{
			std::memset(&dst[dst_ptr], 0, length);

			dst_ptr += length;
		}
		else
		{
			memcpy(&dst[dst_ptr], &huffman_dst[huffman_dst_ptr], length);

			dst_ptr += length;
			huffman_dst_ptr += length;
		}

		zero_flag ^= 1;
	}

	// 

	const u16 colors = bpp >> 3;
	const s32 line = width * colors;

	dst_ptr = 0;

	for (s32 y = 0; y < height; y++)
	{
		for (s32 x = 0; x < width; x++)
		{
			for (size_t i = 0; i < colors; i++)
			{
				u8 work;

				if (y == 0 && x == 0)
				{
					// Top-left corner
					work = 0;
				}
				else if (y == 0)
				{
					// Upper
					// Gets the pixel on the left
					work = dst[dst_ptr - colors];
				}
				else if (x == 0)
				{
					// Left-hand corner
					// Gets the pixel above
					work = dst[dst_ptr - line];
				}
				else
				{
					// Other
					// Gets the average pixels on the left
					work = (dst[dst_ptr - colors] + dst[dst_ptr - line]) >> 1;
				}

				dst[dst_ptr++] += work;
			}
		}
	}
}

/// Image Decoding
///
/// @param dst    Destination for the decoded image data
/// @param src    Encrypted image data
/// @param width  Width
/// @param height Height
/// @param bpp    Bit depth
void CEthornell::DecryptBGType1(u8* dst, const u8* src, s32 width, s32 height, u16 bpp)
{
	const u16 colors = bpp >> 3;

	// Get a pointer to each color component
	const u8* component_src[4];
	for (size_t i = 0; i < colors; i++)
	{
		component_src[i] = &src[width * height * i];
	}

	// Initialization of variables
	u8 prev[4] = {};

	// Decryption
	u8* dst2 = dst;
	for (s32 i = 0; i < height; i++)
	{
		if (i & 0x01)
		{
			dst2 += width * colors;

			for (s32 j = 0; j < width; j++)
			{
				dst2 -= colors;

				for (size_t k = 0; k < colors; k++)
				{
					dst2[k] = *component_src[k]++ + prev[k];
					prev[k] = dst2[k];
				}
			}

			dst2 += width * colors;
		}
		else
		{
			for (s32 j = 0; j < width; j++)
			{
				for (size_t k = 0; k < colors; k++)
				{
					*dst2 = *component_src[k]++ + prev[k];
					prev[k] = *dst2++;
				}
			}
		}
	}
}
