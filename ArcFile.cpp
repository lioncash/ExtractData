#include "stdafx.h"
#include "Common.h"
#include "Dialog/ExistsDialog.h"
#include "ExtractBase.h"
#include "Extract/Standard.h"
#include "MD5.h"
#include "ArcFile.h"

/// Constructor
CArcFile::CArcFile()
{
	m_dwArcsID = -1;
	m_pEnt = nullptr;
	m_hFile = INVALID_HANDLE_VALUE;
	m_ctEnt = 0;
}

/// Destructor
CArcFile::~CArcFile()
{
	Close();
}

/// Mount
bool CArcFile::Mount()
{
	CStandard clStandard;

	return clStandard.Mount(this);
}

/// Decode
bool CArcFile::Decode()
{
	CStandard clStandard;

	return clStandard.Decode(this);
}

/// Extract
bool CArcFile::Extract()
{
	CStandard clStandard;

	return clStandard.Extract(this);
}

////////////////////////////////////////////////////////////////////////
//
// Archive file manipulation
//
////////////////////////////////////////////////////////////////////////

/// Open archive file
///
/// @param pszPathToArc Path to the archive file
///
bool CArcFile::Open(LPCTSTR pszPathToArc)
{
	HANDLE hArc = CreateFile(pszPathToArc, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hArc == INVALID_HANDLE_VALUE)
	{
		CError error;
		if (PathFileExists(pszPathToArc))
			error.Message(GetForegroundWindow(), _T("%s could not be opened."), PathFindFileName(pszPathToArc));
		else
			error.Message(GetForegroundWindow(), _T("%s does not exist."), PathFindFileName(pszPathToArc));
	}
	else
	{
		m_dwArcsID++;
		m_hArcs.push_back(hArc);
		m_pclArcPaths.push_back(pszPathToArc);
		m_pclArcNames.push_back(PathFindFileName(pszPathToArc));
		m_pclArcExtens.push_back(PathFindExtension(pszPathToArc));
	}

	return hArc != INVALID_HANDLE_VALUE;
}

void CArcFile::Close()
{
	for (auto* handle : m_hArcs)
	{
		if (handle != INVALID_HANDLE_VALUE)
		{
			::CloseHandle(handle);
		}
	}

	m_dwArcsID = -1;
	m_hArcs.clear();
	m_pclArcPaths.clear();
	m_pclArcNames.clear();
	m_pclArcExtens.clear();
	m_vcFileInfoOfFileNameSorted.clear();
}

DWORD CArcFile::Read(void* buf, DWORD size)
{
	DWORD dwReadSize;

	::ReadFile(m_hArcs[m_dwArcsID], buf, size, &dwReadSize, nullptr);

	return dwReadSize;
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

BYTE* CArcFile::ReadHed()
{
	Read(m_pHeader.data(), m_pHeader.size());
	return m_pHeader.data();
}

u64 CArcFile::Seek(s64 offset, DWORD seek_mode)
{
	LARGE_INTEGER li;
	li.QuadPart = offset;
	li.LowPart = SetFilePointer(m_hArcs[m_dwArcsID], li.LowPart, &li.HighPart, seek_mode);

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
	li.LowPart = GetFileSize(m_hArcs[m_dwArcsID], &reinterpret_cast<DWORD&>(li.HighPart));
	return static_cast<u64>(li.QuadPart);
}

////////////////////////////////////////////////////////////////////////
//
// Output file operation
//
////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////
/// Open File
///
/// @param pszReFileExt Extension after rename
///
bool CArcFile::OpenFile(LPCTSTR pszReFileExt)
{
	bool ret = false;

	// Create file path
	CreateFileName(pszReFileExt);

	// Open file
	if (m_clfOutput.Open(m_clsPathToFile, (YCFile::modeCreate | YCFile::modeWrite | YCFile::shareDenyWrite)))
	{
		// Opened file successfully

		if (m_clsPathToFile.Find(m_pOption->TmpDir) >= 0)
		{
			m_pInfFile->sTmpFilePath.insert(m_clsPathToFile);
		}

		ret = true;
	}
	else
	{
		// Failed to open file.

		CError clError;

		clError.Message(m_pProg->GetHandle(), _T("Failed to write to %s"), m_clsPathToFile);

		throw -1;
	}

	// Prepare simple decoding
	// InitDecrypt();

	return ret;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Opening the script file for writing

bool CArcFile::OpenScriptFile()
{
	YCString clsFileExt;

	if (m_pOption->bRenameScriptExt)
	{
		clsFileExt = _T(".txt");
	}

	return OpenFile(clsFileExt);
}

//////////////////////////////////////////////////////////////////////////////////////////
// Close File

void CArcFile::CloseFile()
{
	m_clfOutput.Close();

	if (m_pProg->OnCancel())
	{
		// If cancel is pressed

		::DeleteFile(m_clfOutput.GetFilePath());
	}
}

/// Simple initialization of the decryption key
///
/// @remark Obtains the decryption key from a file
///
DWORD CArcFile::InitDecrypt()
{
	m_deckey = 0;

	if (!m_pOption->bEasyDecrypt)
	{
		return m_deckey;
	}

	const SFileInfo* file_info = GetOpenFileInfo();

	const QWORD current_pos = GetArcPointer();
	if (current_pos != file_info->start)
	{
		SeekHed(file_info->start);
	}

	BYTE abtHeader[12] = {};
	Read(abtHeader, sizeof(abtHeader));
	SeekHed(current_pos);

	return InitDecrypt(abtHeader);
}

/// Simple initialization of the decryption key
///
/// @remark Gets the decryption key from data
///
DWORD CArcFile::InitDecrypt(const u8* data)
{
	m_deckey = 0;

	if (!m_pOption->bEasyDecrypt)
	{
		return m_deckey;
	}

	const SFileInfo* file_info = GetOpenFileInfo();

	if (file_info->format == _T("OGG"))
	{
		// Ogg Vorbis
		m_deckey = *(DWORD*) &data[0] ^ 0x5367674F;
	}
	else if (file_info->format == _T("PNG"))
	{
		// PNG
		m_deckey = *(DWORD*) &data[0] ^ 0x474E5089;
	}
	else if (file_info->format == _T("BMP"))
	{
		// BMP
		m_deckey = (*(WORD*) &data[6] << 16) | *(WORD*) &data[8];
	}
	else if ((file_info->format == _T("JPG")) || (file_info->format == _T("JPEG")))
	{
		// JPEG
		m_deckey = *(DWORD*) &data[0] ^ 0xE0FFD8FF;
	}
	else if ((file_info->format == _T("MPG")) || (file_info->format == _T("MPEG")))
	{
		// MPEG
		m_deckey = *(DWORD*) &data[0] ^ 0xBA010000;
	}
	else if (file_info->format == _T("TLG"))
	{
		// TLG
		m_deckey = *(DWORD*) &data[4] ^ 0x7200302E;

		// Try to decode
		*(DWORD*) &data[0] ^= m_deckey;

		if ((memcmp(data, "TLG5", 4) != 0) && (memcmp(data, "TLG6", 4) != 0))
		{
			// TLG0 Decision
			m_deckey = *(DWORD*) &data[4] ^ 0x7300302E;
		}
	}

	return m_deckey;
}

/// Simple initialization for the decryption key
///
/// @param text_data      Text Data
/// @param text_data_size Text data size
///
/// @remark Text-only data
///
DWORD CArcFile::InitDecryptForText(const u8* text_data, size_t text_data_size)
{
	m_deckey = *(WORD*) &text_data[text_data_size - 2] ^ 0x0A0D;
	m_deckey = (m_deckey << 16) | m_deckey;

	return m_deckey;
}

// Simple decoding
void CArcFile::Decrypt(u8* buffer, size_t buffer_size)
{
	if (m_deckey == 0)
		return;

	for (size_t i = 0; i < buffer_size; i += sizeof(u32))
		*(LPDWORD)&buffer[i] ^= m_deckey;
}

/// Write File
DWORD CArcFile::WriteFile(const void* pvBuffer, DWORD dwWriteSize, DWORD dwSizeOrg)
{
	DWORD dwResult = m_clfOutput.Write(pvBuffer, dwWriteSize);

	if (dwResult != dwWriteSize)
	{
		// Write Failure
		CError clError;

		// Check disk space
		ULARGE_INTEGER stuiFreeBytesAvailable;
		ULARGE_INTEGER stuiTotalNumberOfBytes;
		ULARGE_INTEGER stuiTotalNumberOfFreeBytes;

		if (::GetDiskFreeSpaceEx(m_clsPathToFile, &stuiFreeBytesAvailable, &stuiTotalNumberOfBytes, &stuiTotalNumberOfFreeBytes))
		{
			if (stuiTotalNumberOfFreeBytes.QuadPart < dwWriteSize)
			{

				clError.Message(m_pProg->GetHandle(), _T("Failed to write %s \n Not enough disk space"), m_clsPathToFile);
				return dwWriteSize;
			}
		}

		// Other causes
		clError.Message(m_pProg->GetHandle(), _T("Failed to write %s"), m_clsPathToFile);
	}

	// Progress update

	if (dwSizeOrg != 0xFFFFFFFF)
	{
		dwWriteSize = dwSizeOrg;
	}

	if (dwWriteSize != 0)
	{
		m_pProg->UpdatePercent(dwWriteSize);
	}

	return dwResult;
}

void CArcFile::ReadWrite()
{
	ReadWrite(m_pInfFile->sizeCmp);
}

void CArcFile::ReadWrite(DWORD FileSize)
{
	DWORD BufSize = GetBufSize();
	YCMemory<BYTE> buf(BufSize);

	for (DWORD WriteSize = 0; WriteSize != FileSize; WriteSize += BufSize)
	{
		// Adjust buffer size
		SetBufSize(&BufSize, WriteSize, FileSize);

		// Output
		Read(&buf[0], BufSize);
		Decrypt(&buf[0], BufSize);
		WriteFile(&buf[0], BufSize);
	}
}

DWORD CArcFile::GetBufSize() const
{
	return (m_pOption->BufSize << 10);
}

void CArcFile::SetBufSize(LPDWORD BufSize, DWORD WriteSize)
{
	SetBufSize(BufSize, WriteSize, GetOpenFileInfo()->sizeOrg);
}

void CArcFile::SetBufSize(LPDWORD BufSize, DWORD WriteSize, DWORD FileSize)
{
	if (WriteSize + *BufSize > FileSize)
		*BufSize = FileSize - WriteSize;
}

/// Additional file information
///
/// @param rfstFileInfo File info to be added.
///
void CArcFile::AddFileInfo(SFileInfo& rfstFileInfo)
{
	if (rfstFileInfo.format == _T(""))
	{
		rfstFileInfo.format = SetFileFormat(rfstFileInfo.name);
	}

	rfstFileInfo.arcName = m_pclArcNames[m_dwArcsID];
	rfstFileInfo.arcID = m_dwArcID;
	rfstFileInfo.arcsID = m_dwArcsID;
	rfstFileInfo.sSizeOrg = SetCommaFormat(rfstFileInfo.sizeOrg);
	rfstFileInfo.sSizeCmp = SetCommaFormat(rfstFileInfo.sizeCmp);

	m_pEnt->push_back(rfstFileInfo);

	// Progress Update
	m_pProg->UpdatePercent(rfstFileInfo.sizeCmp);

	m_ctEnt++;
}

/// Additional file information
///
/// @param rfstFileInfo File info to be added.
/// @param dwFile       File number
/// @param pszFileExt   Extension

void CArcFile::AddFileInfo(SFileInfo& rfstFileInfo, DWORD& dwFile, LPCTSTR pszFileExt)
{
	// Set filename
	TCHAR szFileName[_MAX_FNAME];
	_stprintf(szFileName, _T("%s_%06d%s"), GetArcName().GetString(), dwFile++, pszFileExt);
	rfstFileInfo.name = szFileName;

	if (rfstFileInfo.format == _T(""))
	{
		rfstFileInfo.format = SetFileFormat(rfstFileInfo.name);
	}

	rfstFileInfo.arcName = m_pclArcNames[m_dwArcsID];
	rfstFileInfo.arcID = m_dwArcID;
	rfstFileInfo.arcsID = m_dwArcsID;
	rfstFileInfo.sSizeOrg = SetCommaFormat(rfstFileInfo.sizeOrg);
	rfstFileInfo.sSizeCmp = SetCommaFormat(rfstFileInfo.sizeCmp);

	m_pEnt->push_back(rfstFileInfo);

	m_ctEnt++;
}

YCString CArcFile::SetFileFormat(const YCString& sFilePath)
{
	TCHAR szFileFormat[256];
	lstrcpy(szFileFormat, PathFindExtension(sFilePath));
	if (lstrcmp(szFileFormat, _T("")) == 0)
		return _T("");

	LPTSTR pszFileFormat = &szFileFormat[1];
	CharUpper(pszFileFormat);
	YCString FileFormat(pszFileFormat);

	return FileFormat;
}

/// Function that separates every three digits in the filesize by commas
YCString CArcFile::SetCommaFormat(DWORD dwSize)
{
	TCHAR buf[256];
	_stprintf(buf, _T("%u"), dwSize);
	YCString sSize(buf);

	int len = sSize.GetLength();
	int comma_num = (len - 1) / 3;
	int comma_pos = len % 3;

	if (comma_pos == 0)
		comma_pos = 3;

	if (comma_num == 0)
		comma_pos = 0;

	for (int i = 0; i < comma_num; i++)
		sSize.Insert(comma_pos + 3 * i + i, _T(','));

	return sSize;
}

/// Function to create the directories leading up to the lowest level you want to create
void CArcFile::MakeDirectory(LPCTSTR pFilePath)
{
	std::vector<YCString> sDirPathList;
	LPCTSTR pFilePathBase = pFilePath;

	while ((pFilePath = PathFindNextComponent(pFilePath)) != nullptr)
	{
		YCString sDirPath(pFilePathBase, pFilePath - pFilePathBase - 1); // You do not put a '\' at the end just to be sure to -1
		sDirPathList.push_back(sDirPath);
	}

	// Create a directory in the order from the root
	for (size_t i = 0; i < sDirPathList.size() - 1; i++) // -1 so as not to create a directory of the file name
	{
		CreateDirectory(sDirPathList[i], nullptr);
	}
}

void CArcFile::ReplaceBackslash(LPTSTR pszPath)
{
	while (*pszPath != _T('\0'))
	{
		if (!::IsDBCSLeadByte(*pszPath))
		{
			// Half-width characters

			// \\ is back substitution

			if (*pszPath == _T('/'))
			{
				*pszPath = _T('\\');
			}
		}

		// To next character
		
		pszPath = CharNext(pszPath);
	}
}

/// Create output filename
///
/// @param pszRenameFileExt Extension after rename
///
YCString CArcFile::CreateFileName(LPCTSTR pszRenameFileExt)
{
	// Get filename
	YCString clsFileName;

	// Extraction for each folder
	if (m_pOption->bCreateFolder)
	{
		clsFileName = m_pInfFile->name;

		// Replace '/' to '\'
		clsFileName.Replace(_T('/'), _T('\\'));
	}
	else // Copy only the filename
	{
		clsFileName = m_pInfFile->name.GetFileName();
	}

	// Get the file path
	YCString clsPathToFile;
	clsPathToFile.Format(_T("%s\\%s"), m_pSaveDir, clsFileName);

//  lstrcpy(szFilePath, m_pSaveDir);
//  PathAddBackslash(szFilePath);

//  lstrcat(szFilePath, szFileName);

	// Changing the extension
	if (pszRenameFileExt != nullptr)
	{
		clsPathToFile.RenameExtension(pszRenameFileExt);
		clsPathToFile.Replace(_T('/'), _T('\\'));
	}

	// Limited to MAX_PATH
	clsPathToFile.Delete(MAX_PATH, clsPathToFile.GetLength());

	// Create directory
	MakeDirectory(clsPathToFile);

	// Overwrite confirmation
	if (PathFileExists(clsPathToFile))
	{
		CExistsDialog clExistsDlg;

		clExistsDlg.DoModal(m_pProg->GetHandle(), clsPathToFile);
	}

	m_clsPathToFile = clsPathToFile;

	return m_clsPathToFile;
}

bool CArcFile::CheckExe(LPCTSTR pExeName)
{
	// Gets the path to the executable file
	TCHAR szExePath[MAX_PATH];
	lstrcpy(szExePath, GetArcPath());
	PathRemoveFileSpec(szExePath);
	PathAppend(szExePath, pExeName);

	return PathFileExists(szExePath) == TRUE;
}

bool CArcFile::CheckDir(LPCTSTR pDirName)
{
	// Get directory name
	TCHAR szDirPath[MAX_PATH];
	lstrcpy(szDirPath, GetArcPath());
	PathRemoveFileSpec(szDirPath);

	if (lstrcmp(PathFindFileName(szDirPath), pDirName) == 0)
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

	for (auto& entry : *m_pEnt)
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
/// @param pszFileName Target file name
///
/// @remark Binary Search
///
const SFileInfo* CArcFile::GetFileInfoForBinarySearch(LPCTSTR pszFileName)
{
	// Information is stored in files that are sorted by file name

	if (m_vcFileInfoOfFileNameSorted.empty())
	{
		// Uninitialized

		size_t uFileInfoStartIndex = GetStartEnt();
		size_t uFileInfoCount = GetCtEnt();

		m_vcFileInfoOfFileNameSorted.resize(uFileInfoCount);

		for (size_t i = 0, j = uFileInfoStartIndex; i < uFileInfoCount; i++, j++)
		{
			m_vcFileInfoOfFileNameSorted[i] = (*m_pEnt)[j];
		}

		std::sort(m_vcFileInfoOfFileNameSorted.begin(), m_vcFileInfoOfFileNameSorted.end(), CompareForFileInfo);
	}

	// Binary Search

	return SearchForFileInfo(m_vcFileInfoOfFileNameSorted, pszFileName);
}

void CArcFile::SetMD5(SMD5 stmd5File)
{
	m_vtstmd5File.push_back(stmd5File);
}

void CArcFile::ClearMD5()
{
	m_bSetMD5 = false;
	m_vtstmd5File.clear();
}

/// Comparison function for sorting
///
/// @param rstfiTarget1 Left 
/// @param rstfiTarget2 Right
///
bool CArcFile::CompareForFileInfo(const SFileInfo& rstfiTarget1, const SFileInfo& rstfiTarget2)
{
	return lstrcmpi(rstfiTarget1.name, rstfiTarget2.name) < 0;
}

/// Binary Search
///
/// @param rvcFileInfo File info list
/// @param pszFileName Target filename
///
SFileInfo* CArcFile::SearchForFileInfo(std::vector<SFileInfo>& rvcFileInfo, LPCTSTR pszFileName)
{
	int nLow = 0;
	int nHigh = (rvcFileInfo.size() - 1);

	while (nLow <= nHigh)
	{
		int nMid = ((nHigh + nLow) / 2);
		int nResult = lstrcmpi(pszFileName, rvcFileInfo[nMid].name);

		if (nResult == 0)
		{
			return &rvcFileInfo[nMid];
		}

		if (nResult > 0)
		{
			nLow = (nMid + 1);
			continue;
		}

		if (nResult < 0)
		{
			nHigh = (nMid - 1);
			continue;
		}
	}

	return nullptr;
}
