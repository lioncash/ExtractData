#include "StdAfx.h"
#include "Extract/VA.h"

#include "ArcFile.h"
#include "Sound/Wav.h"

namespace
{
// NWA Format
struct NWAHeader
{
	u16   channels;             // Number of channels
	u16   bits;                 // Number of bits per sample (bit/sample)
	u32   freq;                 // Sampling Rate
	u32   compression_level;    // Compression Level
	u32   dummy1;               // Unknown
	u32   blocks;               // Number of blocks
	u32   data_size;            // Number of bytes of waveform data after decompression
	u32   compressed_file_size; // Compressed file size
	u32   samples;              // Number of decompressed data
	u32   block_size;           // The number of units of data samples of a block when you expand the data
	u32   rest_size;            // Number of sample units of data when you expand the last block
	u32   dummy2;               // Unknown
};

// NWK Index Header
struct IndexNwk
{
	u32 size;
	u32 start;
	u32 dummy;
};
} // Anonymous namespace

bool CVA::Mount(CArcFile* archive)
{
	if (MountNwa(archive))
		return true;
	if (MountNwk(archive))
		return true;
	if (MountOvk(archive))
		return true;

	return false;
}

// Function that gets the information of a *.nwa file.
bool CVA::MountNwa(CArcFile* archive)
{
	if (archive->GetArcExten() != _T(".nwa"))
		return false;

	// Read nwa header
	NWAHeader nwa_header;
	archive->Read(&nwa_header, sizeof(NWAHeader));

	// Add to listview
	SFileInfo file_info;
	file_info.name = archive->GetArcName();
	file_info.sizeOrg = nwa_header.data_size + 44;
	file_info.sizeCmp = (nwa_header.compressed_file_size == 0) ? file_info.sizeOrg : nwa_header.compressed_file_size;
	file_info.start = 0x00;
	file_info.end = file_info.start + file_info.sizeCmp;
	archive->AddFileInfo(file_info);

	return true;
}

/// Function that gets the information of a *.nwk file
bool CVA::MountNwk(CArcFile* archive)
{
	if (archive->GetArcExten() != _T(".nwk"))
		return false;

	// Get file count
	u32 num_files;
	archive->ReadU32(&num_files);

	// Number of files retrieved from the index
	const u32 index_size = num_files * 12;

	// Get index
	std::vector<u8> index(index_size);
	archive->Read(index.data(), index.size());

	// Get the base filename
	TCHAR base_file_name[_MAX_FNAME];
	lstrcpy(base_file_name, archive->GetArcName());
	PathRemoveExtension(base_file_name);

	const u8* index_ptr = index.data();
	for (u32 i = 0; i < num_files; i++)
	{
		// Get filename
		TCHAR file_name[_MAX_FNAME];
		_stprintf(file_name, _T("%s_%06u.nwa"), base_file_name, *reinterpret_cast<const u32*>(&index_ptr[8]));

		SFileInfo file_info;
		file_info.name = file_name;
		file_info.sizeCmp = *reinterpret_cast<const u32*>(&index_ptr[0]);
		//file_info.sizeOrg = file_info.sizeCmp;
		file_info.start = *reinterpret_cast<const u32*>(&index_ptr[4]);
		file_info.end = file_info.start + file_info.sizeCmp;

		// Get filesize
		archive->Seek(file_info.start + 20, FILE_BEGIN);
		archive->Read(&file_info.sizeOrg, 4);
		file_info.sizeOrg += 44;

		archive->AddFileInfo(file_info);

		index_ptr += 12;
	}

	return true;
}

/// Gets information from .ovk files
bool CVA::MountOvk(CArcFile* archive)
{
	if (archive->GetArcExten() != _T(".ovk"))
		return false;

	// Get file count
	u32 num_files;
	archive->ReadU32(&num_files);

	// Get index size
	const u32 index_size = num_files * 16;

	// Read index
	std::vector<u8> index(index_size);
	archive->Read(index.data(), index.size());

	// Get base filename
	TCHAR base_file_name[_MAX_FNAME];
	lstrcpy(base_file_name, archive->GetArcName());
	PathRemoveExtension(base_file_name);

	// Get file information
	for (u32 i = 0; i < index_size; i += 16)
	{
		// Get filename
		TCHAR file_name[_MAX_FNAME];
		_stprintf(file_name, _T("%s_%06u.ogg"), base_file_name, *reinterpret_cast<const u32*>(&index[i + 8]));

		// Get file information
		SFileInfo file_info;
		file_info.name = file_name;
		file_info.sizeCmp = *reinterpret_cast<const u32*>(&index[i]);
		file_info.sizeOrg = file_info.sizeCmp;
		file_info.start = *reinterpret_cast<const u32*>(&index[i + 4]);
		file_info.end = file_info.start + file_info.sizeCmp;

		archive->AddFileInfo(file_info);
	}

	return true;
}

int CVA::GetBits(u8*& data, int& shift, int bits)
{
	if (shift > 8)
	{
		data++;
		shift -= 8;
	}
	
	const int ret = *reinterpret_cast<const u16*>(data) >> shift;
	shift += bits;
	
	return ret & ((1 << bits) - 1); // mask
}

bool CVA::Decode(CArcFile* archive)
{
	if (DecodeNwa(archive))
		return true;

	return false;
}

// Function to convert to WAV for extraction
bool CVA::DecodeNwa(CArcFile* archive)
{
	const SFileInfo* file_info = archive->GetOpenFileInfo();

	if (file_info->format != _T("NWA"))
		return false;

	// Read NWA header
	NWAHeader nwa_header;
	archive->Read(&nwa_header, sizeof(NWAHeader));

	CWav wav;
	wav.Init(archive, nwa_header.data_size, nwa_header.freq, nwa_header.channels, nwa_header.bits);

	// If no compression, just change the WAV header
	if (nwa_header.compression_level == 0xFFFFFFFF)
	{
		wav.Write();
	}
	else
	{
		// RLE compression
		const bool rle = nwa_header.compression_level == 5 && nwa_header.channels != 2;

		// Memory allocation of the offset
		const u32 offset_size = nwa_header.blocks * 4;
		std::vector<u32> offsets(offset_size);

		// Allocate memory for writing data
		const u32 buf_len = nwa_header.block_size * (nwa_header.bits >> 3);
		std::vector<u8> buf(buf_len);

		// Allocate memory for data loading
		const u32 z_buf_len = buf_len * 2;
		std::vector<u8> z_buf(z_buf_len);

		// Read offset
		archive->Read(offsets.data(), offsets.size());

		for (u32 i = 0; i < nwa_header.blocks; i++)
		{
			u8* z_pbuf = z_buf.data();
			u8* pbuf = buf.data();

			// Get the size of the data to be read/decoded
			u32 current_block_size, current_compressed_size;
			if (i < nwa_header.blocks - 1)
			{
				current_block_size = buf_len;
				current_compressed_size = offsets[i + 1] - offsets[i];
			}
			else
			{
				current_block_size = nwa_header.rest_size * (nwa_header.bits >> 3);
				current_compressed_size = z_buf_len;
			}

			// Ensure buffers exist
			const u8* src_end = z_pbuf + current_compressed_size;
			const u8* dst_end = pbuf + current_block_size;

			// Read data
			archive->Read(z_pbuf, current_compressed_size);

			int d[2];
			int shift = 0;

			// Read first data
			if (nwa_header.bits == 8)
			{
				d[0] = *z_pbuf++;
			}
			else // fNWA.bits == 16
			{
				d[0] = *reinterpret_cast<const u16*>(z_pbuf);
				z_pbuf += 2;
			}

			if (nwa_header.channels == 2)
			{
				if (nwa_header.bits == 8)
				{
					d[1] = *z_pbuf++;
				}
				else  // fNWA.bits == 16
				{
					d[1] = *reinterpret_cast<const u16*>(z_pbuf);
					z_pbuf += 2;
				}
			}

			const int dsize = current_block_size / (nwa_header.bits >> 3);
			bool flip = false; // Stereo

			for (int j = 0; j < dsize; j++)
			{
				if (z_pbuf >= src_end || pbuf >= dst_end)
				{
					// Exit
					break;
				}

				const int type = GetBits(z_pbuf, shift, 3);

				// 7 : Large difference
				if (type == 7)
				{
					if (GetBits(z_pbuf, shift, 1) == 1)
					{
						d[flip] = 0; // Unused
					}
					else
					{
						int BITS, SHIFT;
						if (nwa_header.compression_level >= 3)
						{
							BITS = 8;
							SHIFT = 9;
						}
						else
						{
							BITS = 8 - nwa_header.compression_level;
							SHIFT = 2 + type + nwa_header.compression_level;
						}

						const int MASK1 = (1 << (BITS - 1));
						const int MASK2 = (1 << (BITS - 1)) - 1;
						const int b = GetBits(z_pbuf, shift, BITS);
						if (b & MASK1)
							d[flip] -= (b & MASK2) << SHIFT;
						else
							d[flip] += (b & MASK2) << SHIFT;
					}
				}
				// 1-6 : Ordinary differential
				else if (type != 0)
				{
					int BITS, SHIFT;
					if (nwa_header.compression_level >= 3)
					{
						BITS = 3 + nwa_header.compression_level;
						SHIFT = 1 + type;
					}
					else
					{
						BITS = 5 - nwa_header.compression_level;
						SHIFT = 2 + type + nwa_header.compression_level;
					}

					const int MASK1 = (1 << (BITS - 1));
					const int MASK2 = (1 << (BITS - 1)) - 1;
					const int b = GetBits(z_pbuf, shift, BITS);
					if (b & MASK1)
						d[flip] -= (b & MASK2) << SHIFT;
					else
						d[flip] += (b & MASK2) << SHIFT;
				}
				else // type == 0
				{
					if (rle)
					{
						// Run-length compression 
						int run_length = GetBits(z_pbuf, shift, 1);
						if (run_length == 1)
						{
							run_length = GetBits(z_pbuf, shift, 2);

							if (run_length == 3)
							{
								run_length = GetBits(z_pbuf, shift, 8);
							}
						}

						// Going to write the same data as the previous data
						for (int k = 0; k <= run_length; k++)
						{
							if (nwa_header.bits == 8)
							{
								*pbuf++ = static_cast<u8>(d[flip]);
							}
							else
							{
								*reinterpret_cast<u16*>(pbuf) = d[flip];
								pbuf += 2;
							}

							if (nwa_header.channels == 2)
							{
								flip ^= 1; // channel switching
							}
						}

						j += run_length;
						continue;
					}
				}

				if (nwa_header.bits == 8)
				{
					*pbuf++ = static_cast<u8>(d[flip]);
				}
				else
				{
					*reinterpret_cast<u16*>(pbuf) = d[flip];
					pbuf += 2;
				}

				if (nwa_header.channels == 2)
				{
					flip ^= 1; // channel switching
				}
			}

			wav.Write(buf.data(), current_block_size);
		}
	}

	return true;
}
