#include "StdAfx.h"
#include "Extract/SummerDays.h"

#include "ArcFile.h"

enum : s32
{
	TYPE_NONE   = 0x00000000,
	TYPE_FOLDER = 0x00000001
};

bool CSummerDays::Mount(CArcFile* archive)
{
	if (memcmp(archive->GetHeader(), "Í2nY", 4) != 0)
		return false;

	// Code starts at 0x8003
	m_context_count = 0x8003;

	// Get header
	u16 num_headers;
	archive->Seek(10, FILE_BEGIN);
	archive->ReadU16(&num_headers);

	// Get file name
	u16 file_name_length;
	archive->ReadU16(&file_name_length);
	TCHAR file_name[256];
	archive->Read(file_name, file_name_length);
	file_name[file_name_length] = _T('\0');

	// Get count
	u16 count;
	archive->ReadU16(&count);
	const u32 total_headers = num_headers > 1 ? num_headers + count - 1 : count;

	TCHAR path[MAX_PATH] = {};
	for (u32 i = 0; i < total_headers; i++)
	{
		if (!Sub(archive, path))
		{
			m_contexts.clear();
			return false;
		}
	}

	m_contexts.clear();

	return true;
}

u16 CSummerDays::CreateNewContext(CArcFile* archive, u16 length)
{
	m_context_count++;

	// Get class name
	TCHAR name[256];
	archive->Read(name, length);
	name[length] = _T('\0');

	Context ctx;
	ctx.name = name;
	ctx.code = m_context_count;
	ctx.type = ctx.name == _T("CAutoFolder") ? TYPE_FOLDER : TYPE_NONE;
	m_contexts.push_back(ctx);

	return m_context_count;
}

int CSummerDays::FindContextTypeWithCode(u16 code)
{
	const auto iter = std::find_if(m_contexts.begin(), m_contexts.end(),
	                               [code](const auto& entry) { return entry.code == code; });

	if (iter == m_contexts.end())
		return -1;

	return iter->type;
}

bool CSummerDays::Sub(CArcFile* archive, LPTSTR path)
{
	union
	{
		u32 u32_value[2];
		u16 u16_value[4];
		u8  u8_value[8];
		s8  s8_value[8];
	} data;

	m_context_count++;

	// Get file name (Folder name)
	u8 length;
	archive->ReadU8(&length);
	TCHAR name[256];
	archive->Read(name, length);
	name[length] = _T('\0');

	// Get start address
	archive->ReadU8(&length);
	archive->Read(data.s8_value, 8);
	u16 context = data.u16_value[1];

	if (context == static_cast<u16>(0xffff))
	{
		context = CreateNewContext(archive, data.u16_value[3]);
		archive->Read(&data.u32_value[1], 4);
	}

	const int type = FindContextTypeWithCode(context);
	const s32 position = static_cast<s32>(data.u32_value[1] - 0xA2FB6AD1U);

	// Get file size
	archive->Read(&data.u32_value[0], 4);
	const u32 base = data.u32_value[0] + 0x184A2608;
	u32 size = (base >> 13) & 0x0007FFFF;
	size |= ((base - size) & 0x00000FFF) << 19;
	
	TCHAR full_path[MAX_PATH];
	lstrcpy(full_path, path);
	PathAppend(full_path, name);

	if (type & TYPE_FOLDER)
	{
		lstrcat(full_path, _T(".Op"));
		u16 count;
		archive->ReadU16(&count);

		for (u32 i = 0; i < count; i++)
		{
			if (!Sub(archive, full_path))
			{
				return false;
			}
		}
	}
	else
	{
		// Add to listview
		SFileInfo file_info;
		file_info.name = full_path;
		file_info.sizeCmp = size;
		file_info.sizeOrg = file_info.sizeCmp;
		file_info.start = position;
		file_info.end = file_info.start + file_info.sizeCmp;
		archive->AddFileInfo(file_info);
	}

	return true;
}
