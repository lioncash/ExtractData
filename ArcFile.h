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
	bool Open(LPCTSTR pszPathToArc);
	void Close();

	DWORD Read(void* pvBuffer, DWORD dwReadSize);
	bool ReadS8(s8* out);
	bool ReadS16(s16* out);
	bool ReadS32(s32* out);
	bool ReadS64(s64* out);
	bool ReadU8(u8* out);
	bool ReadU16(u16* out);
	bool ReadU32(u32* out);
	bool ReadU64(u64* out);
	BYTE* ReadHed();

	template <typename T, size_t N>
	bool ReadArray(T (&arr)[N])
	{
		static_assert(std::is_trivially_copyable<T>(), "T must be trivially copyable");

		constexpr size_t bytes_to_read = N * sizeof(T);
		return Read(arr, bytes_to_read) == bytes_to_read;
	}

	u64 Seek(s64 offset, DWORD seek_mode);
	u64 SeekHed(s64 offset = 0);
	u64 SeekEnd(s64 offset = 0);
	u64 SeekCur(s64 offset);

	u64 GetArcPointer();
	u64 GetArcSize() const;

	// Output file operations
	YCString CreateFileName(LPCTSTR pszReFileExt = nullptr);

	bool OpenFile(LPCTSTR pszReFileExt = nullptr);
	bool OpenScriptFile();
	void CloseFile();

	DWORD WriteFile(const void* pvBuffer, DWORD dwWriteSize, DWORD dwSizeOrg = 0xFFFFFFFF);
	void  ReadWrite();
	void  ReadWrite(DWORD FileSize);

	// Simple decoding
	DWORD InitDecrypt();
	DWORD InitDecrypt(const u8* data);
	DWORD InitDecryptForText(const u8* text_data, size_t text_data_size);
	void  Decrypt(u8* buffer, size_t buffer_size);

	// File information to be added to the list in the archive
	void AddFileInfo(SFileInfo& infFile);
	void AddFileInfo(SFileInfo& infFile, DWORD& ctFile, LPCTSTR pFileExt);

	YCString SetFileFormat(const YCString& sFilePath);
	YCString SetCommaFormat(DWORD dwSize);

	// Initialization processes performed prior to decoding/mounting
	void SetFileInfo(DWORD num)               { m_pInfFile = &(*m_pEnt)[num]; m_dwArcsID = m_pInfFile->arcsID; m_dwInfFileNum = num; }
	void SetArcID(DWORD dwArcID)              { m_dwArcID = dwArcID; }
	void SetEnt(std::vector<SFileInfo>& rEnt) { m_pEnt = &rEnt; m_nStartEnt = rEnt.size(); }
	void SetProg(CProgBar& prog)              { m_pProg = &prog; }
	void SetOpt(SOption* pOption)             { m_pOption = pOption; }
	void SetSaveDir(LPCTSTR pSaveDir)         { m_pSaveDir = pSaveDir; }

	// TRUE if file is supported(Closed immediately if the file is not supported)
	void SetState(bool bState) { m_bState = bState; }

	// Split files support
	void            SetFirstArc()             { m_dwArcsID = 0; }
	void            SetNextArc()              { m_dwArcsID++; }
	HANDLE          GetArcHandle()            { return m_hArcs[m_dwArcsID]; }
	void            SetArcsID(DWORD dwArcsID) { m_dwArcsID = dwArcsID; }
	DWORD           GetArcsID()   const       { return m_dwArcsID; }
	size_t          GetArcCount() const       { return m_hArcs.size(); }
	const YCString& GetArcPath()  const       { return m_pclArcPaths[m_dwArcsID]; }
	const YCString& GetArcName()  const       { return m_pclArcNames[m_dwArcsID]; }
	const YCString& GetArcExten() const       { return m_pclArcExtens[m_dwArcsID]; }

	// Returns previously loaded archive file header
	LPBYTE GetHed() { return m_pHeader.data(); }

	SFileInfo*              GetFileInfo(DWORD num) const { return &(*m_pEnt)[num]; }
	SFileInfo*              GetFileInfo(LPCTSTR filepath, bool compare_filename_only = false) const;
	const SFileInfo*        GetFileInfoForBinarySearch(LPCTSTR pszFileName);
	std::vector<SFileInfo>& GetFileInfo() const        { return *m_pEnt; }
	SFileInfo*              GetOpenFileInfo() const    { return m_pInfFile; }
	DWORD                   GetOpenFileInfoNum() const { return m_dwInfFileNum; }

	DWORD     GetArcID() const          { return m_dwArcID; }
	size_t    GetStartEnt() const       { return m_nStartEnt; }
	size_t    GetCtEnt() const          { return m_ctEnt; }
	CProgBar* GetProg() const           { return m_pProg; }
	SOption*  GetOpt() const            { return m_pOption; }

	bool      GetState() const          { return m_bState; }

	DWORD     GetBufSize() const;
	void      SetBufSize(LPDWORD BufSize, DWORD WriteSize);
	void      SetBufSize(LPDWORD BufSize, DWORD WriteSize, DWORD FileSize);

	void      ReplaceBackslash(LPTSTR pFileName);
	void      MakeDirectory(LPCTSTR wFileName);

	bool      CheckExe(LPCTSTR pExeName);
	bool      CheckDir(LPCTSTR pDirName);

	// Susie plugin mounting
	void      SetMountSusie() { m_bMountWasSusie = true; }
	bool      GetMountSusie() const { return m_bMountWasSusie; }

	// MD5 value setting
	void      SetMD5(SMD5 stmd5File);

	// Set flag of MD5 value
	void      SetMD5OfFlag(bool bSetMD5) { m_bSetMD5 = bSetMD5; }

	// Get MD5 value
	std::vector<SMD5> GetMD5() const { return m_vtstmd5File; }

	// Check if MD5 value is set
	bool CheckMD5OfSet() const { return m_bSetMD5; }

	// Clear MD5 value
	void ClearMD5();

	// Work flag setting
	void SetFlag(bool bFlag) { m_bWork = bFlag; }

	// Workload capture flag
	bool GetFlag() const { return m_bWork; }

	// Comparison function for sorting
	static bool CompareForFileInfo(const SFileInfo& rstfiTarget1, const SFileInfo& rstfiTarget2);

	// Binary search
	static SFileInfo* SearchForFileInfo(std::vector<SFileInfo>& rvcFileInfo, LPCTSTR pszFileName);

private:

	YCMultiFile             m_clmfArc;
	YCFile                  m_clfOutput;

	// File information in the archive
	std::vector<SFileInfo>* m_pEnt;
	SFileInfo*              m_pInfFile;

	std::vector<SFileInfo>  m_vcFileInfoOfFileNameSorted;

	// Archive file split support
	DWORD                   m_dwArcsID;
	std::vector<HANDLE>     m_hArcs;
	std::vector<YCString>   m_pclArcPaths;
	std::vector<YCString>   m_pclArcNames;
	std::vector<YCString>   m_pclArcExtens;

	std::array<BYTE, 2048>  m_pHeader;
	DWORD                   m_dwArcID;
	size_t                  m_nStartEnt;    // Starting index of the archive file information
	size_t                  m_ctEnt;        // Number of archive file information
	DWORD                   m_dwInfFileNum;

	bool                    m_bState = false;

	CProgBar*               m_pProg;
	SOption*                m_pOption;

	HANDLE                  m_hFile;
	YCString                m_clsPathToFile;
	LPCTSTR                 m_pSaveDir;

	DWORD                   m_deckey;

	bool                    m_bMountWasSusie = false;   // Check if Susie was used to mount a plugin

	// MD5 value
	std::vector<SMD5>       m_vtstmd5File;

	// Set MD5 value flag
	bool                    m_bSetMD5 = false;

	// Flag variable that can be set for each archive
	bool                    m_bWork = false;
};
