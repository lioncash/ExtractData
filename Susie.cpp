#include "StdAfx.h"
#include "Susie.h"

#include "ArcFile.h"
#include "Common.h"
#include "Image.h"

using SPI_PROGRESS         = int (WINAPI*)(int, int, long);
using GetPluginInfoProc    = int (WINAPI*)(int infono, char* buf, int buflen);
using IsSupportedProc      = int (WINAPI*)(char* filename, DWORD dw);
using GetArchiveInfoProc   = int (WINAPI*)(char* buf, long len, unsigned int flag, HLOCAL* lphInf);
using GetFileProc          = int (WINAPI*)(char* src, long len, char* dest, unsigned int flag, FARPROC prgressCallback, long lData);
using GetPictureProc       = int (WINAPI*)(char* buf, long len, unsigned int flag, HLOCAL* pHBInfo, HLOCAL* pHBm, SPI_PROGRESS lpPrgressCallback, long lData);
using ConfigurationDlgProc = int (WINAPI*)(HWND, int);

#pragma pack(push, 1)
struct fileInfo
{
	u8   method[8];      // Type of compression method
	u32  position;       // Position on the file
	u32  compsize;       // Compressed size
	u32  filesize;       // Original file size
	u32  timestamp;      // Modified date of the file
	char path[200];      // Relative Path
	char filename[200];  // Filename
	u32  crc;            // CRC
};
#pragma pack(pop, 1)

std::vector<SSusieInfo> CSusie::m_main;
std::vector<SSusieInfo> CSusie::m_temporary;

/// Initialization
void CSusie::Init()
{
	m_temporary = m_main;
}

/// Apply
void CSusie::Apply()
{
	m_main = m_temporary;
}

/// Mount
///
/// @param archive Archive
///
bool CSusie::Mount(CArcFile* archive)
{
	// Get header
	const u8* pbtHeader = archive->GetHeader();

	// Mount
	for (const auto& target : m_main)
	{
		if (target.validity == 0)
		{
			// Invalid Susie plugin
			continue;
		}

		// Get IsSupported()
		auto* IsSupported = reinterpret_cast<IsSupportedProc>(target.plugin.GetProcAddress(_T("IsSupported")));
		if (IsSupported == nullptr)
		{
			// IsSupported function is not implemented
			continue;
		}

		// Copy the archive file path
		char      szPathToArc[MAX_PATH];
		YCStringA clsPathToArc = archive->GetArcPath();
		strcpy(szPathToArc, clsPathToArc);

		// Call IsSupported()
		u8 abtHeader[2048];
		memcpy(abtHeader, pbtHeader, sizeof(abtHeader));
		if (IsSupported(szPathToArc, reinterpret_cast<DWORD>(abtHeader)) == 0)
		{
			// Archive file not supported
			continue;
		}

		// Get GetArchiveInfo()
		auto* GetArchiveInfo = reinterpret_cast<GetArchiveInfoProc>(target.plugin.GetProcAddress(_T("GetArchiveInfo")));
		if (GetArchiveInfo == nullptr)
		{
			// We trust IsSupported() and attempt to mount
			return archive->Mount();
		}

		// Copy archive path (Since there is a chance that IsSupported() can be overwritten)
		strcpy(szPathToArc, clsPathToArc);

		// Call GetArchiveInfo()
		YCLocalMemory cllmFileInfo;
		if (GetArchiveInfo(szPathToArc, 0, 0, &cllmFileInfo.GetHandle()) != 0)
		{
			// Error in function GetArchiveInfo()
			cllmFileInfo.Free();

			// We will trust IsSupported() and attempt to mount
			return archive->Mount();
		}

		// Error in the function GetArchiveInfo()
		if (cllmFileInfo.GetHandle() == nullptr)
		{
			// We will trust IsSupported() and attempt to mount
			return archive->Mount();
		}

		// Get file information
		const auto* pstFileInfo = static_cast<fileInfo*>(cllmFileInfo.Lock());

		const size_t uSusieFileInfoSize = cllmFileInfo.GetSize();
		const size_t uFileCount = uSusieFileInfoSize / sizeof(fileInfo);

		for (size_t uIndex = 0; uIndex < uFileCount; uIndex++)
		{
			if (pstFileInfo->method[0] == '\0')
			{
				// Exit
				break;
			}

			char szFileName[MAX_PATH];

			if (pstFileInfo->path[0] != '\0')
			{
				// Path exists
				strcpy(szFileName, pstFileInfo->path);
				PathAppendA(szFileName, pstFileInfo->filename);
			}
			else
			{
				strcpy(szFileName, pstFileInfo->filename);
			}

			// Set the file information
			SFileInfo stFileInfo;
			stFileInfo.name = szFileName;
			stFileInfo.sizeCmp = (pstFileInfo->compsize == 0) ? pstFileInfo->filesize : pstFileInfo->compsize;
			stFileInfo.sizeOrg = pstFileInfo->filesize;
			stFileInfo.start = pstFileInfo->position;
			stFileInfo.end = stFileInfo.start + stFileInfo.sizeCmp;
			stFileInfo.format.Append((LPTSTR)pstFileInfo->method, 8);

			archive->AddFileInfo(stFileInfo);
			pstFileInfo++;
		}

		// Release resources
		cllmFileInfo.Free();

		return true;
	}

	return false;
}

/// Decode
///
/// @param archive Archive
///
bool CSusie::Decode(CArcFile* archive)
{
	YCStringA clsPathToArc = archive->GetArcPath();
	YCStringA clsFileName = archive->GetOpenFileInfo()->name;

	// Get header
	const u8* pbtHeader = archive->GetHeader();

	// GetPicture() file input
	std::vector<const YCLibrary*> vtSupportPlugin;

	for (const auto& target : m_main)
	{
		if (target.validity == 0)
		{
			// Invalid Susie plugin
			continue;
		}

		// Get IsSupported()
		auto* IsSupported = reinterpret_cast<IsSupportedProc>(target.plugin.GetProcAddress(_T("IsSupported")));
		if (IsSupported == nullptr)
		{
			// IsSupported() function is not implemented 
			continue;
		}

		// Copy the archive's file path
		char szPathToArc[MAX_PATH];
		strcpy(szPathToArc, clsPathToArc);

		u8 abtHeader[2048];
		memcpy(abtHeader, pbtHeader, sizeof(abtHeader));

		if (!IsSupported(szPathToArc, reinterpret_cast<DWORD>(abtHeader)))
		{
			// Archive file is not supported
			continue;
		}

		// Archive file is supported
		vtSupportPlugin.push_back(&target.plugin);

		// Copy the archive file path
		strcpy(szPathToArc, clsPathToArc);

		// Get GetPicture()
		auto* GetPicture = reinterpret_cast<GetPictureProc>(target.plugin.GetProcAddress(_T("GetPicture")));
		if (GetPicture == nullptr)
		{
			// GetPicture() function is not implemented
			continue;
		}

		YCLocalMemory cllmBitmapInfo;
		YCLocalMemory cllmBitmapData;

		// Call GetPicture()
		if (GetPicture(szPathToArc, archive->GetOpenFileInfo()->start, 0x0000, &cllmBitmapInfo.GetHandle(), &cllmBitmapData.GetHandle(), nullptr, 0) != 0)
		{
			// GetPicture() has failed
			continue;
		}

		// Get image information
		const auto*  pBMPInfo = static_cast<LPBITMAPINFO>(cllmBitmapInfo.Lock());
		const u8*    pHBm = static_cast<u8*>(cllmBitmapData.Lock());
		const size_t HBmSize = cllmBitmapData.GetSize();

		// Image output
		CImage image;
		image.Init(archive, pBMPInfo->bmiHeader.biWidth, pBMPInfo->bmiHeader.biHeight, pBMPInfo->bmiHeader.biBitCount, (u8*) pBMPInfo->bmiColors);
		image.Write(pHBm, HBmSize);
		image.Close();

		// Exit (Free resources)
		cllmBitmapInfo.Free();
		cllmBitmapData.Free();

		return true;
	}

	// File input - GetFile()
	YCLocalMemory cllmSrc;

	for (const auto& support_plugin : vtSupportPlugin)
	{
		// Copy archive file path
		char szPathToArc[MAX_PATH];
		strcpy(szPathToArc, clsPathToArc);

		// Get GetFile()
		auto* GetFile = reinterpret_cast<GetFileProc>(support_plugin->GetProcAddress(_T("GetFile")));
		if (GetFile == nullptr)
		{
			// GetFile() function is not supported
			continue;
		}

		// Call GetFile()
		if (GetFile(szPathToArc, archive->GetOpenFileInfo()->start, reinterpret_cast<char*>(&cllmSrc.GetHandle()), 0x0100, nullptr, 0) != 0)
		{
			// GetFile has failed
			cllmSrc.Free();
			continue;
		}

		// Successful completion
		break;
	}

	// Lock the memory for reading
	bool   bGetFileSuccess = true;
	u8* pbtSrc = nullptr;
	u32 dwSrcSize;

	if (cllmSrc.GetHandle() != nullptr)
	{
		// Successful GetFile()

		pbtSrc = static_cast<u8*>(cllmSrc.Lock());
		dwSrcSize = cllmSrc.GetSize();
	}
	else
	{
		// GetFile() has failed
		bGetFileSuccess = false;

		// Memory allocation
		dwSrcSize = archive->GetOpenFileInfo()->sizeCmp;

		cllmSrc.Alloc(LHND, dwSrcSize);
		pbtSrc = static_cast<u8*>(cllmSrc.Lock());

		// Reading
		archive->Read(pbtSrc, dwSrcSize);
		archive->SeekCur(-(s64)dwSrcSize);
	}

	// Memory Input - GetPicture()
	for (const auto& target : m_main)
	{
		if (target.validity == 0)
		{
			// Invalid Susie plugin
			continue;
		}

		// Get IsSupported()
		auto* IsSupported = reinterpret_cast<IsSupportedProc>(target.plugin.GetProcAddress(_T("IsSupported")));
		if (IsSupported == nullptr)
		{
			// IsSupported() function is not implemented 
			continue;
		}

		// Copy file path
		// Plugin fails if the full path does not exist

		char szPathToFile[MAX_PATH];
		strcpy(szPathToFile, clsPathToArc.GetDirPath());
		PathAppendA(szPathToFile, clsFileName);

		// Needs 2KB
		u8 abtSrcHeader[2048];
		const size_t dwCopySize = (dwSrcSize <= sizeof(abtSrcHeader)) ? dwSrcSize : sizeof(abtSrcHeader);

		ZeroMemory(abtSrcHeader, sizeof(abtSrcHeader));
		memcpy(abtSrcHeader, pbtSrc, dwCopySize);

		// Call IsSupported()
		if (!IsSupported(szPathToFile, reinterpret_cast<DWORD>(abtSrcHeader)))
		{
			// File is not supported
			continue;
		}

		// Get GetPicture()
		auto* GetPicture = reinterpret_cast<GetPictureProc>(target.plugin.GetProcAddress(_T("GetPicture")));
		if (GetPicture == nullptr)
		{
			// GetPicture() function is not implemented
			continue;
		}

		YCLocalMemory cllmBitmapInfo;
		YCLocalMemory cllmBitmapData;

		// Call GetPicture()
		if (GetPicture(reinterpret_cast<char*>(pbtSrc), dwSrcSize, 0x0001, &cllmBitmapInfo.GetHandle(), &cllmBitmapData.GetHandle(), nullptr, 0) != 0)
		{
			// GetPicture() has failed
			continue;
		}

		// Get image info
		const auto*  pBMPInfo = static_cast<LPBITMAPINFO>(cllmBitmapInfo.Lock());
		const u8*    pHBm = static_cast<u8*>(cllmBitmapData.Lock());
		const size_t HBmSize = cllmBitmapData.GetSize();

		// Image output
		CImage image;
		image.Init(archive, pBMPInfo->bmiHeader.biWidth, pBMPInfo->bmiHeader.biHeight, pBMPInfo->bmiHeader.biBitCount, (u8*) pBMPInfo->bmiColors);
		image.Write(pHBm, HBmSize);
		image.Close();

		// Exit
		cllmBitmapInfo.Free();
		cllmBitmapData.Free();
		cllmSrc.Free();

		return true;
	}

	// Exit if the file could not be obtained in the function GetFile()
	if (!bGetFileSuccess)
	{
		cllmSrc.Free();

		return false;
	}

	// Output file obtained from GetFile()
	archive->OpenFile();
	archive->WriteFile(pbtSrc, dwSrcSize, archive->GetOpenFileInfo()->sizeCmp);
	archive->CloseFile();

	// Exit (Free resources)
	cllmSrc.Free();

	return true;
}

/// Load Susie Plugin
void CSusie::LoadSpi(const YCString& susie_folder_path)
{
	// Release / Free Susie Plugin
	m_main.clear();

	// Susie Plugin Path
	TCHAR susie_plugin_path[MAX_PATH];

	lstrcpy(susie_plugin_path, susie_folder_path);
	PathAppend(susie_plugin_path, _T("*.spi"));

	// Susie Plugin Info
	YCFileFindSx          finder;
	std::vector<YCString> susie_plugin_paths;

	finder.FindFile(susie_plugin_paths, susie_folder_path, _T("*.spi"), false);

	// Get Susie Plugin Information
	YCIni susie_ini(SBL_STR_INI_SUSIE);
	susie_ini.SetSection(_T("Plugins"));

	size_t index = 0;

	m_main.resize(susie_plugin_paths.size());

	for (auto& target_path : susie_plugin_paths)
	{
		SSusieInfo* target = &m_main[index];

		// Load SPI
		if (!target->plugin.Load(target_path))
		{
			// Loading failed
			continue;
		}

		// Validate SPI
		susie_ini.SetKey(PathFindFileName(target_path));
		susie_ini.ReadDec(&target->validity, 1);

		// SPI File Path Information
		target->path = target_path;

		// SPI Folder Name Information
		target->name = PathFindFileName(target_path);

		// Get SPI Info
		char buffer[1024];

		auto* GetPluginInfo = reinterpret_cast<GetPluginInfoProc>(target->plugin.GetProcAddress(_T("GetPluginInfo")));
		if (GetPluginInfo != nullptr)
		{
			if (GetPluginInfo(0, buffer, sizeof(buffer)) != 0)
			{
				// Successful at getting SPI info
				target->version = buffer;
			}

			if (GetPluginInfo(1, buffer, sizeof(buffer)) != 0)
			{
				// Successful at getting SPI info
				target->info = buffer;
			}

			for (int info_num = 2; ; info_num += 2)
			{
				if (GetPluginInfo(info_num, buffer, sizeof(buffer)) == 0)
				{
					// Could not get any more info
					break;
				}

				if (!target->supported_formats.IsEmpty())
				{
					target->supported_formats += _T(";");
				}

				target->supported_formats += buffer;
			}
		}

		// Check if the SPI has a configuration dialog
		auto* ConfigurationDlg = reinterpret_cast<ConfigurationDlgProc>(target->plugin.GetProcAddress(_T("ConfigurationDlg")));
		target->has_config_dialog = ConfigurationDlg != nullptr;

		// Prepare the next SPI
		index++;
	}

	// Load segments
	m_main.resize(index);
}

/// Save Susie Plugin Information
void CSusie::SaveSpi()
{
	YCIni susie_ini(SBL_STR_INI_SUSIE);
	susie_ini.SetSection(_T("Plugins"));

	// Delete info relating to Susie plugins that no longer exist.
	susie_ini.DeleteSection();

	// Save Susie plugin info
	for (const auto& target : m_main)
	{
		susie_ini.SetKey(target.name);
		susie_ini.WriteDec(target.validity);
	}
}
