#pragma once

#include "ProgressBar.h"
#include "MD5.h"

#include <cstddef>
#include <cstdint>
#include <type_traits>

class CArcFile
{
public:
	CArcFile();
	virtual ~CArcFile();

	bool Mount();
	bool Decode();
	bool Extract();

	// Archive file manipulation
	bool Open(LPCTSTR archive_path);
	void Close();

	u32 Read(void* buffer, u32 read_size);
	bool ReadS8(s8* out);
	bool ReadS16(s16* out);
	bool ReadS32(s32* out);
	bool ReadS64(s64* out);
	bool ReadU8(u8* out);
	bool ReadU16(u16* out);
	bool ReadU32(u32* out);
	bool ReadU64(u64* out);
	u8* ReadHed();

	template <typename T, size_t N>
	bool ReadArray(T (&arr)[N])
	{
		static_assert(std::is_trivially_copyable<T>(), "T must be trivially copyable");

		constexpr size_t bytes_to_read = N * sizeof(T);
		return Read(arr, bytes_to_read) == bytes_to_read;
	}

	u64 Seek(s64 offset, u32 seek_mode);
	u64 SeekHed(s64 offset = 0);
	u64 SeekEnd(s64 offset = 0);
	u64 SeekCur(s64 offset);

	u64 GetArcPointer();
	u64 GetArcSize() const;

	// Output file operations
	YCString CreateFileName(LPCTSTR rename_file_ext = nullptr);

	bool OpenFile(LPCTSTR rename_file_ext = nullptr);
	bool OpenScriptFile();
	void CloseFile();

	DWORD WriteFile(const void* buffer, DWORD write_size, DWORD original_size = 0xFFFFFFFF);
	void  ReadWrite();
	void  ReadWrite(size_t file_size);

	// Simple decoding
	u32 InitDecrypt();
	u32 InitDecrypt(u8* data);
	u32 InitDecryptForText(const u8* text_data, size_t text_data_size);
	void  Decrypt(u8* buffer, size_t buffer_size);

	// File information to be added to the list in the archive
	void AddFileInfo(SFileInfo& file_info);
	void AddFileInfo(SFileInfo& file_info , u32& file_number, LPCTSTR file_extension);

	YCString SetFileFormat(const YCString& file_path);
	YCString SetCommaFormat(u32 size);

	// Initialization processes performed prior to decoding/mounting
	void SetFileInfo(size_t num)             { m_file_info = &(*m_entries)[num]; m_split_archive_id = m_file_info->arcsID; m_file_info_num = num; }
	void SetArcID(u32 archive_id)            { m_archive_id = archive_id; }
	void SetEnt(std::vector<SFileInfo>& ent) { m_entries = &ent; m_start_entry_index = ent.size(); }
	void SetProg(CProgBar& prog)             { m_progress_bar = &prog; }
	void SetOpt(SOption* option)             { m_option = option; }
	void SetSaveDir(LPCTSTR save_dir)        { m_save_dir = save_dir; }

	// TRUE if file is supported(Closed immediately if the file is not supported)
	void SetState(bool state) { m_state = state; }

	// Split files support
	void            SetFirstArc()             { m_split_archive_id = 0; }
	void            SetNextArc()              { m_split_archive_id++; }
	HANDLE          GetArcHandle()            { return m_archive_handles[m_split_archive_id]; }
	void            SetArcsID(u32 archive_id) { m_split_archive_id = archive_id; }
	u32             GetArcsID()   const       { return m_split_archive_id; }
	size_t          GetArcCount() const       { return m_archive_handles.size(); }
	const YCString& GetArcPath()  const       { return m_archive_paths[m_split_archive_id]; }
	const YCString& GetArcName()  const       { return m_archive_names[m_split_archive_id]; }
	const YCString& GetArcExten() const       { return m_archive_extensions[m_split_archive_id]; }

	// Returns previously loaded archive file header
	u8*       GetHed()       { return m_header.data(); }
	const u8* GetHed() const { return m_header.data(); }

	SFileInfo*              GetFileInfo(size_t num) const { return &(*m_entries)[num]; }
	SFileInfo*              GetFileInfo(LPCTSTR filepath, bool compare_filename_only = false) const;
	const SFileInfo*        GetFileInfoForBinarySearch(LPCTSTR file_name);
	std::vector<SFileInfo>& GetFileInfo() const        { return *m_entries; }
	SFileInfo*              GetOpenFileInfo() const    { return m_file_info; }
	size_t                  GetOpenFileInfoNum() const { return m_file_info_num; }

	u32       GetArcID() const          { return m_archive_id; }
	size_t    GetStartEnt() const       { return m_start_entry_index; }
	size_t    GetCtEnt() const          { return m_entry_count; }
	CProgBar* GetProg() const           { return m_progress_bar; }
	SOption*  GetOpt() const            { return m_option; }

	bool      GetState() const          { return m_state; }

	size_t    GetBufSize() const;
	void      SetBufSize(size_t* buffer_size, size_t write_size);
	void      SetBufSize(size_t* buffer_size, size_t write_size, size_t file_size);

	void      ReplaceBackslash(LPTSTR file_name);
	void      MakeDirectory(LPCTSTR file_name);

	bool      CheckExe(LPCTSTR exe_name);
	bool      CheckDir(LPCTSTR dir_name);

	// Susie plugin mounting
	void      SetMountSusie() { m_mounted_with_susie = true; }
	bool      GetMountSusie() const { return m_mounted_with_susie; }

	// MD5 value setting
	void      SetMD5(SMD5 md5);

	// Set flag of MD5 value
	void      SetMD5OfFlag(bool set_md5) { m_set_md5 = set_md5; }

	// Get MD5 value
	std::vector<SMD5> GetMD5() const { return m_file_md5s; }

	// Check if MD5 value is set
	bool CheckMD5OfSet() const { return m_set_md5; }

	// Clear MD5 value
	void ClearMD5();

	// Work flag setting
	void SetFlag(bool flag) { m_work = flag; }

	// Workload capture flag
	bool GetFlag() const { return m_work; }

	// Comparison function for sorting
	static bool CompareForFileInfo(const SFileInfo& left, const SFileInfo& right);

	// Binary search
	static SFileInfo* SearchForFileInfo(std::vector<SFileInfo>& file_info, LPCTSTR file_name);

private:
	YCFile                  m_output;

	// File information in the archive
	std::vector<SFileInfo>* m_entries;
	SFileInfo*              m_file_info;

	// Sorted by file name
	std::vector<SFileInfo>  m_file_info_sorted;

	// Archive file split support
	u32                     m_split_archive_id;
	std::vector<HANDLE>     m_archive_handles;
	std::vector<YCString>   m_archive_paths;
	std::vector<YCString>   m_archive_names;
	std::vector<YCString>   m_archive_extensions;

	std::array<u8, 2048>    m_header;
	u32                     m_archive_id;
	size_t                  m_start_entry_index;  // Starting index of the archive file information
	size_t                  m_entry_count;        // Number of archive file information
	size_t                  m_file_info_num;

	bool                    m_state = false;

	CProgBar*               m_progress_bar;
	SOption*                m_option;

	HANDLE                  m_file_handle;
	YCString                m_file_path;
	LPCTSTR                 m_save_dir;

	u32                     m_decryption_key;

	bool                    m_mounted_with_susie = false;   // Check if Susie was used to mount a plugin

	// MD5 value
	std::vector<SMD5>       m_file_md5s;

	// Set MD5 value flag
	bool                    m_set_md5 = false;

	// Flag variable that can be set for each archive
	bool                    m_work = false;
};
