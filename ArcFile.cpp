#include "StdAfx.h"
#include "ArcFile.h"

#include "Common.h"
#include "UI/Dialog/ExistsDialog.h"
#include "Extract/Standard.h"
#include "MD5.h"

CArcFile::CArcFile()
{
	m_split_archive_id = -1;
	m_entries = nullptr;
	m_file_handle = INVALID_HANDLE_VALUE;
	m_entry_count = 0;
}

CArcFile::~CArcFile()
{
	Close();
}

bool CArcFile::Mount()
{
	CStandard standard;
	return standard.Mount(this);
}

bool CArcFile::Decode()
{
	CStandard standard;
	return standard.Decode(this);
}

bool CArcFile::Extract()
{
	CStandard standard;
	return standard.Extract(this);
}

/// Open archive file
///
/// @param archive_path Path to the archive file
///
bool CArcFile::Open(LPCTSTR archive_path)
{
	HANDLE archive_handle = CreateFile(archive_path, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (archive_handle == INVALID_HANDLE_VALUE)
	{
		CError error;
		if (PathFileExists(archive_path))
			error.Message(GetForegroundWindow(), _T("%s could not be opened."), PathFindFileName(archive_path));
		else
			error.Message(GetForegroundWindow(), _T("%s does not exist."), PathFindFileName(archive_path));
	}
	else
	{
		m_split_archive_id++;
		m_archive_handles.push_back(archive_handle);
		m_archive_paths.push_back(archive_path);
		m_archive_names.push_back(PathFindFileName(archive_path));
		m_archive_extensions.push_back(PathFindExtension(archive_path));
	}

	return archive_handle != INVALID_HANDLE_VALUE;
}

void CArcFile::Close()
{
	for (auto* handle : m_archive_handles)
	{
		if (handle != INVALID_HANDLE_VALUE)
		{
			::CloseHandle(handle);
		}
	}

	m_split_archive_id = -1;
	m_archive_handles.clear();
	m_archive_paths.clear();
	m_archive_names.clear();
	m_archive_extensions.clear();
	m_file_info_sorted.clear();
}

u32 CArcFile::Read(void* buffer, u32 read_size)
{
	DWORD bytes_read;
	::ReadFile(m_archive_handles[m_split_archive_id], buffer, read_size, &bytes_read, nullptr);

	return static_cast<u32>(bytes_read);
}

bool CArcFile::ReadS8(s8* out)
{
	return Read(out, sizeof(s8)) == sizeof(s8);
}

bool CArcFile::ReadS16(s16* out)
{
	return Read(out, sizeof(s16)) == sizeof(s16);
}

bool CArcFile::ReadS32(s32* out)
{
	return Read(out, sizeof(s32)) == sizeof(s32);
}

bool CArcFile::ReadS64(s64* out)
{
	return Read(out, sizeof(s64)) == sizeof(s64);
}

bool CArcFile::ReadU8(u8* out)
{
	return Read(out, sizeof(u8)) == sizeof(u8);
}

bool CArcFile::ReadU16(u16* out)
{
	return Read(out, sizeof(u16)) == sizeof(u16);
}

bool CArcFile::ReadU32(u32* out)
{
	return Read(out, sizeof(u32)) == sizeof(u32);
}

bool CArcFile::ReadU64(u64* out)
{
	return Read(out, sizeof(u64)) == sizeof(u64);
}

u8* CArcFile::ReadHeader()
{
	Read(m_header.data(), m_header.size());
	return m_header.data();
}

u64 CArcFile::Seek(s64 offset, u32 seek_mode)
{
	LARGE_INTEGER li;
	li.QuadPart = offset;
	li.LowPart = SetFilePointer(m_archive_handles[m_split_archive_id], li.LowPart, &li.HighPart, seek_mode);

	if (li.LowPart == INVALID_SET_FILE_POINTER && GetLastError() != NO_ERROR)
		li.QuadPart = -1;

	return static_cast<u64>(li.QuadPart);
}

u64 CArcFile::SeekHed(s64 offset)
{
	return Seek(offset, FILE_BEGIN);
}

u64 CArcFile::SeekEnd(s64 offset)
{
	return Seek(-offset, FILE_END);
}

u64 CArcFile::SeekCur(s64 offset)
{
	return Seek(offset, FILE_CURRENT);
}

u64 CArcFile::GetArcPointer()
{
	return Seek(0, FILE_CURRENT);
}

u64 CArcFile::GetArcSize() const
{
	LARGE_INTEGER li = {};
	li.LowPart = GetFileSize(m_archive_handles[m_split_archive_id], &reinterpret_cast<DWORD&>(li.HighPart));
	return static_cast<u64>(li.QuadPart);
}

/// Open File
///
/// @param rename_file_ext Extension after rename
///
bool CArcFile::OpenFile(LPCTSTR rename_file_ext)
{
	bool ret = false;

	// Create file path
	CreateFileName(rename_file_ext);

	// Open file
	if (m_output.Open(m_file_path, YCFile::modeCreate | YCFile::modeWrite | YCFile::shareDenyWrite))
	{
		// Opened file successfully
		if (m_file_path.Find(m_option->TmpDir) >= 0)
		{
			m_file_info->sTmpFilePath.insert(m_file_path);
		}

		ret = true;
	}
	else
	{
		// Failed to open file.
		CError error;
		error.Message(m_progress_bar->GetHandle(), _T("Failed to write to %s"), m_file_path);

		throw -1;
	}

	// Prepare simple decoding
	// InitDecrypt();

	return ret;
}

/// Opens the script file for writing
bool CArcFile::OpenScriptFile()
{
	YCString file_extension;

	if (m_option->bRenameScriptExt)
	{
		file_extension = _T(".txt");
	}

	return OpenFile(file_extension);
}


/// Close File
void CArcFile::CloseFile()
{
	m_output.Close();

	if (m_progress_bar->OnCancel())
	{
		// If cancel is pressed
		::DeleteFile(m_output.GetFilePath());
	}
}

/// Simple initialization of the decryption key
///
/// @remark Obtains the decryption key from a file
///
u32 CArcFile::InitDecrypt()
{
	m_decryption_key = 0;

	if (!m_option->bEasyDecrypt)
	{
		return m_decryption_key;
	}

	const SFileInfo* file_info = GetOpenFileInfo();

	const u64 current_pos = GetArcPointer();
	if (current_pos != file_info->start)
	{
		SeekHed(file_info->start);
	}

	u8 header[12] = {};
	Read(header, sizeof(header));
	SeekHed(current_pos);

	return InitDecrypt(header);
}

/// Simple initialization of the decryption key
///
/// @remark Gets the decryption key from data
///
u32 CArcFile::InitDecrypt(u8* data)
{
	m_decryption_key = 0;

	if (!m_option->bEasyDecrypt)
	{
		return m_decryption_key;
	}

	const SFileInfo* file_info = GetOpenFileInfo();

	if (file_info->format == _T("OGG"))
	{
		// Ogg Vorbis
		m_decryption_key = *reinterpret_cast<u32*>(&data[0]) ^ 0x5367674F;
	}
	else if (file_info->format == _T("PNG"))
	{
		// PNG
		m_decryption_key = *reinterpret_cast<u32*>(&data[0]) ^ 0x474E5089;
	}
	else if (file_info->format == _T("BMP"))
	{
		// BMP
		m_decryption_key = (*reinterpret_cast<u16*>(&data[6]) << 16) | *reinterpret_cast<u16*>(&data[8]);
	}
	else if ((file_info->format == _T("JPG")) || (file_info->format == _T("JPEG")))
	{
		// JPEG
		m_decryption_key = *reinterpret_cast<u32*>(&data[0]) ^ 0xE0FFD8FF;
	}
	else if ((file_info->format == _T("MPG")) || (file_info->format == _T("MPEG")))
	{
		// MPEG
		m_decryption_key = *reinterpret_cast<u32*>(&data[0]) ^ 0xBA010000;
	}
	else if (file_info->format == _T("TLG"))
	{
		// TLG
		m_decryption_key = *reinterpret_cast<u32*>(&data[4]) ^ 0x7200302E;

		// Try to decode
		*reinterpret_cast<u32*>(&data[0]) ^= m_decryption_key;

		if (memcmp(data, "TLG5", 4) != 0 && memcmp(data, "TLG6", 4) != 0)
		{
			// TLG0 Decision
			m_decryption_key = *reinterpret_cast<u32*>(&data[4]) ^ 0x7300302E;
		}
	}

	return m_decryption_key;
}

/// Simple initialization for the decryption key
///
/// @param text_data      Text Data
/// @param text_data_size Text data size
///
/// @remark Text-only data
///
u32 CArcFile::InitDecryptForText(const u8* text_data, size_t text_data_size)
{
	m_decryption_key = *reinterpret_cast<const u16*>(&text_data[text_data_size - 2]) ^ 0x0A0D;
	m_decryption_key = (m_decryption_key << 16) | m_decryption_key;

	return m_decryption_key;
}

// Simple decoding
void CArcFile::Decrypt(u8* buffer, size_t buffer_size)
{
	if (m_decryption_key == 0)
		return;

	for (size_t i = 0; i < buffer_size; i += sizeof(u32))
		*reinterpret_cast<u32*>(&buffer[i]) ^= m_decryption_key;
}

/// Write File
DWORD CArcFile::WriteFile(const void* buffer, DWORD write_size, DWORD original_size)
{
	const DWORD result = m_output.Write(buffer, write_size);

	if (result != write_size)
	{
		// Write Failure
		CError error;

		// Check disk space
		ULARGE_INTEGER free_bytes_available;
		ULARGE_INTEGER total_number_of_bytes;
		ULARGE_INTEGER total_number_of_free_bytes;

		if (::GetDiskFreeSpaceEx(m_file_path, &free_bytes_available, &total_number_of_bytes, &total_number_of_free_bytes))
		{
			if (total_number_of_free_bytes.QuadPart < write_size)
			{

				error.Message(m_progress_bar->GetHandle(), _T("Failed to write %s \n Not enough disk space"), m_file_path);
				return write_size;
			}
		}

		// Other causes
		error.Message(m_progress_bar->GetHandle(), _T("Failed to write %s"), m_file_path);
	}

	// Progress update
	if (original_size != 0xFFFFFFFF)
	{
		write_size = original_size;
	}

	if (write_size != 0)
	{
		m_progress_bar->UpdatePercent(write_size);
	}

	return result;
}

void CArcFile::ReadWrite()
{
	ReadWrite(m_file_info->sizeCmp);
}

void CArcFile::ReadWrite(size_t file_size)
{
	size_t buffer_size = GetBufSize();
	std::vector<u8> buffer(buffer_size);

	for (size_t write_size = 0; write_size != file_size; write_size += buffer_size)
	{
		// Adjust buffer size
		SetBufSize(&buffer_size, write_size, file_size);

		// Output
		Read(buffer.data(), buffer_size);
		Decrypt(buffer.data(), buffer_size);
		WriteFile(buffer.data(), buffer_size);
	}
}

size_t CArcFile::GetBufSize() const
{
	return m_option->BufSize << 10;
}

void CArcFile::SetBufSize(size_t* buffer_size, size_t write_size)
{
	SetBufSize(buffer_size, write_size, GetOpenFileInfo()->sizeOrg);
}

void CArcFile::SetBufSize(size_t* buffer_size, size_t write_size, size_t file_size)
{
	if (write_size + *buffer_size > file_size)
		*buffer_size = file_size - write_size;
}

/// Additional file information
///
/// @param file_info File info to be added.
///
void CArcFile::AddFileInfo(SFileInfo& file_info)
{
	if (file_info.format == _T(""))
	{
		file_info.format = SetFileFormat(file_info.name);
	}

	file_info.arcName = m_archive_names[m_split_archive_id];
	file_info.arcID = m_archive_id;
	file_info.arcsID = m_split_archive_id;
	file_info.sSizeOrg = SetCommaFormat(file_info.sizeOrg);
	file_info.sSizeCmp = SetCommaFormat(file_info.sizeCmp);

	m_entries->push_back(file_info);

	// Progress Update
	m_progress_bar->UpdatePercent(file_info.sizeCmp);

	m_entry_count++;
}

/// Additional file information
///
/// @param file_info      File info to be added.
/// @param file_number    File number
/// @param file_extension Extension

void CArcFile::AddFileInfo(SFileInfo& file_info, u32& file_number, LPCTSTR file_extension)
{
	// Set filename
	TCHAR szFileName[_MAX_FNAME];
	_stprintf(szFileName, _T("%s_%06d%s"), GetArcName().GetString(), file_number++, file_extension);
	file_info.name = szFileName;

	if (file_info.format == _T(""))
	{
		file_info.format = SetFileFormat(file_info.name);
	}

	file_info.arcName = m_archive_names[m_split_archive_id];
	file_info.arcID = m_archive_id;
	file_info.arcsID = m_split_archive_id;
	file_info.sSizeOrg = SetCommaFormat(file_info.sizeOrg);
	file_info.sSizeCmp = SetCommaFormat(file_info.sizeCmp);

	m_entries->push_back(file_info);

	m_entry_count++;
}

YCString CArcFile::SetFileFormat(const YCString& file_path)
{
	TCHAR file_format[256];
	lstrcpy(file_format, PathFindExtension(file_path));
	if (lstrcmp(file_format, _T("")) == 0)
		return _T("");

	LPTSTR file_format_upper = &file_format[1];
	CharUpper(file_format_upper);
	return file_format_upper;
}

/// Function that separates every three digits in the filesize by commas
YCString CArcFile::SetCommaFormat(u32 size)
{
	TCHAR buf[256];
	_stprintf(buf, _T("%u"), size);
	YCString size_string(buf);

	int len = size_string.GetLength();
	int comma_num = (len - 1) / 3;
	int comma_pos = len % 3;

	if (comma_pos == 0)
		comma_pos = 3;

	if (comma_num == 0)
		comma_pos = 0;

	for (int i = 0; i < comma_num; i++)
		size_string.Insert(comma_pos + 3 * i + i, _T(','));

	return size_string;
}

/// Function to create the directories leading up to the lowest level you want to create
void CArcFile::MakeDirectory(LPCTSTR file_path)
{
	std::vector<YCString> dir_paths;
	LPCTSTR file_path_base = file_path;

	while ((file_path = PathFindNextComponent(file_path)) != nullptr)
	{
		YCString dir_path(file_path_base, file_path - file_path_base - 1); // You do not put a '\' at the end just to be sure to -1
		dir_paths.push_back(dir_path);
	}

	// Create a directory in the order from the root
	for (size_t i = 0; i < dir_paths.size() - 1; i++) // -1 so as not to create a directory of the file name
	{
		CreateDirectory(dir_paths[i], nullptr);
	}
}

void CArcFile::ReplaceBackslash(LPTSTR path)
{
	while (*path != _T('\0'))
	{
		if (!::IsDBCSLeadByte(*path))
		{
			// Half-width characters
			// \\ is back substitution
			if (*path == _T('/'))
			{
				*path = _T('\\');
			}
		}

		// To next character
		path = CharNext(path);
	}
}

/// Create output filename
///
/// @param rename_file_ext Extension after rename
///
YCString CArcFile::CreateFileName(LPCTSTR rename_file_ext)
{
	// Get filename
	YCString file_name;

	// Extraction for each folder
	if (m_option->bCreateFolder)
	{
		file_name = m_file_info->name;

		// Replace '/' to '\'
		file_name.Replace(_T('/'), _T('\\'));
	}
	else // Copy only the filename
	{
		file_name = m_file_info->name.GetFileName();
	}

	// Get the file path
	YCString file_path;
	file_path.Format(_T("%s\\%s"), m_save_dir, file_name);

	// Changing the extension
	if (rename_file_ext != nullptr)
	{
		file_path.RenameExtension(rename_file_ext);
		file_path.Replace(_T('/'), _T('\\'));
	}

	// Limited to MAX_PATH
	file_path.Delete(MAX_PATH, file_path.GetLength());

	// Create directory
	MakeDirectory(file_path);

	// Overwrite confirmation
	if (PathFileExists(file_path))
	{
		CExistsDialog exists_dialog;
		exists_dialog.DoModal(m_progress_bar->GetHandle(), file_path);
	}

	m_file_path = file_path;

	return m_file_path;
}

bool CArcFile::CheckExe(LPCTSTR exe_name) const
{
	// Gets the path to the executable file
	TCHAR exe_path[MAX_PATH];
	lstrcpy(exe_path, GetArcPath());
	PathRemoveFileSpec(exe_path);
	PathAppend(exe_path, exe_name);

	return PathFileExists(exe_path) == TRUE;
}

bool CArcFile::CheckDir(LPCTSTR dir_name) const
{
	// Get directory name
	TCHAR dir_path[MAX_PATH];
	lstrcpy(dir_path, GetArcPath());
	PathRemoveFileSpec(dir_path);

	if (lstrcmp(PathFindFileName(dir_path), dir_name) == 0)
		return true;

	return false;
}

/// Return information to the appropriate file from the file name
///
/// @param filepath              Target file path
/// @param compare_filename_only Whether or not to compare just the file names
///
/// @remark Linear search
///
SFileInfo* CArcFile::GetFileInfo(LPCTSTR filepath, bool compare_filename_only) const
{
	if (compare_filename_only)
	{
		// Comparison in the requested file name only
		filepath = PathFindFileName(filepath);
	}

	for (auto& entry : *m_entries)
	{
		LPCTSTR name = entry.name;

		if (compare_filename_only)
		{
			// Comparison in the requested file name only
			name = PathFindFileName(name);
		}

		if (lstrcmpi(name, filepath) == 0)
		{
			return &entry;
		}
	}

	return nullptr;
}

/// Return information to the appropriate file from the file name
///
/// @param file_name Target file name
///
/// @remark Binary Search
///
const SFileInfo* CArcFile::GetFileInfoForBinarySearch(LPCTSTR file_name)
{
	// Information is stored in files that are sorted by file name
	if (m_file_info_sorted.empty())
	{
		// Uninitialized
		const size_t file_info_start_index = GetStartEnt();
		const size_t file_info_count = GetCtEnt();

		m_file_info_sorted.resize(file_info_count);

		for (size_t i = 0, j = file_info_start_index; i < file_info_count; i++, j++)
		{
			m_file_info_sorted[i] = (*m_entries)[j];
		}

		std::sort(m_file_info_sorted.begin(), m_file_info_sorted.end(), CompareForFileInfo);
	}

	// Binary Search
	return SearchForFileInfo(m_file_info_sorted, file_name);
}

void CArcFile::SetMD5(SMD5 md5)
{
	m_file_md5s.push_back(md5);
}

void CArcFile::ClearMD5()
{
	m_set_md5 = false;
	m_file_md5s.clear();
}

/// Comparison function for sorting
bool CArcFile::CompareForFileInfo(const SFileInfo& left, const SFileInfo& right)
{
	return lstrcmpi(left.name, right.name) < 0;
}

/// Binary Search
///
/// @param file_info File info list
/// @param file_name Target filename
///
SFileInfo* CArcFile::SearchForFileInfo(std::vector<SFileInfo>& file_info, LPCTSTR file_name)
{
	int low = 0;
	int high = file_info.size() - 1;

	while (low <= high)
	{
		const int mid = (high + low) / 2;
		const int result = lstrcmpi(file_name, file_info[mid].name);

		if (result == 0)
		{
			return &file_info[mid];
		}

		if (result > 0)
		{
			low = mid + 1;
			continue;
		}

		if (result < 0)
		{
			high = mid - 1;
			continue;
		}
	}

	return nullptr;
}
