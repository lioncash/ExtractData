#include "StdAfx.h"
#include "Susie.h"

#include "ArcFile.h"
#include "Common.h"
#include "Image.h"

typedef int (WINAPI* SPI_PROGRESS)(int, int, long);
typedef int (WINAPI* GetPluginInfoProc)(int infono, LPSTR buf, int buflen);
typedef int (WINAPI* IsSupportedProc)(LPSTR filename, DWORD dw);
typedef int (WINAPI* GetArchiveInfoProc)(LPSTR buf, long len, unsigned int flag, HLOCAL* lphInf);
typedef int (WINAPI* GetFileProc)(LPSTR src, long len, LPSTR dest, unsigned int flag, FARPROC prgressCallback, long lData);
typedef int (WINAPI* GetPictureProc)(LPSTR buf, long len, unsigned int flag, HLOCAL* pHBInfo, HLOCAL* pHBm, SPI_PROGRESS lpPrgressCallback, long lData);
typedef int (WINAPI* ConfigurationDlgProc)(HWND, int);

#pragma pack(push, 1)
struct fileInfo
{
	BYTE    method[8];      // Type of compression method
	DWORD   position;       // Position on the file
	DWORD   compsize;       // Compressed size
	DWORD   filesize;       // Original file size
	DWORD   timestamp;      // Modified date of the file
	char    path[200];      // Relative Path
	char    filename[200];  // Filename
	DWORD   crc;            // CRC
};
#pragma pack(pop, 1)

std::vector<SSusieInfo> CSusie::m_stsiMain;
std::vector<SSusieInfo> CSusie::m_stsiTemporary;

//////////////////////////////////////////////////////////////////////////////////////////
// Initialization

void CSusie::Init()
{
	m_stsiTemporary = m_stsiMain;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Apply

void CSusie::Apply()
{
	m_stsiMain = m_stsiTemporary;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Mount
//
// Parameters:
//   - pclArc - Archive

bool CSusie::Mount(CArcFile* pclArc)
{
	// Get header
	const u8* pbtHeader = pclArc->GetHeader();

	// Mount
	for (auto& pstsiTarget : m_stsiMain)
	{
		if (pstsiTarget.bValidity == 0)
		{
			// Invalid Susie plugin
			continue;
		}

		// Get IsSupported()
		IsSupportedProc IsSupported = reinterpret_cast<IsSupportedProc>(pstsiTarget.cllPlugin.GetProcAddress(_T("IsSupported")));
		if (IsSupported == nullptr)
		{
			// IsSupported function is not implemented
			continue;
		}

		// Copy the archive file path
		char      szPathToArc[MAX_PATH];
		YCStringA clsPathToArc = pclArc->GetArcPath();
		strcpy(szPathToArc, clsPathToArc);

		// Call IsSupported()
		BYTE abtHeader[2048];
		memcpy(abtHeader, pbtHeader, sizeof(abtHeader));
		if (IsSupported(szPathToArc, (DWORD) abtHeader) == 0)
		{
			// Archive file not supported
			continue;
		}

		// Get GetArchiveInfo()
		GetArchiveInfoProc GetArchiveInfo = reinterpret_cast<GetArchiveInfoProc>(pstsiTarget.cllPlugin.GetProcAddress(_T("GetArchiveInfo")));
		if (GetArchiveInfo == nullptr)
		{
			// We trust IsSupported() and attempt to mount
			return pclArc->Mount();
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
			return pclArc->Mount();
		}

		// Error in the function GetArchiveInfo()
		if (cllmFileInfo.GetHandle() == nullptr)
		{
			// We will trust IsSupported() and attempt to mount
			return pclArc->Mount();
		}

		// Get file information
		fileInfo* pstFileInfo = (fileInfo*) cllmFileInfo.Lock();

		size_t    uSusieFileInfoSize = cllmFileInfo.GetSize();
		size_t    uFileCount = uSusieFileInfoSize / sizeof(fileInfo);

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

			pclArc->AddFileInfo(stFileInfo);
			pstFileInfo++;
		}

		// Release resources
		cllmFileInfo.Free();

		return true;
	}

	return false;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Decode
//
// Parameters:
//   - pclArc - Archive

bool CSusie::Decode(CArcFile* pclArc)
{
	YCStringA clsPathToArc = pclArc->GetArcPath();
	YCStringA clsFileName = pclArc->GetOpenFileInfo()->name;

	// Get header
	const u8* pbtHeader = pclArc->GetHeader();

	// GetPicture() file input
	std::vector<YCLibrary*> vtSupportPlugin;

	for (auto& pstsiTarget : m_stsiMain)
	{
		if (pstsiTarget.bValidity == 0)
		{
			// Invalid Susie plugin
			continue;
		}

		// Get IsSupported()
		IsSupportedProc IsSupported = reinterpret_cast<IsSupportedProc>(pstsiTarget.cllPlugin.GetProcAddress(_T("IsSupported")));
		if (IsSupported == nullptr)
		{
			// IsSupported() function is not implemented 
			continue;
		}

		// Copy the archive's file path
		char szPathToArc[MAX_PATH];
		strcpy(szPathToArc, clsPathToArc);

		BYTE abtHeader[2048];
		memcpy(abtHeader, pbtHeader, sizeof(abtHeader));

		if (!IsSupported(szPathToArc, (DWORD) abtHeader))
		{
			// Archive file is not supported
			continue;
		}

		// Archive file is supported
		vtSupportPlugin.push_back(&pstsiTarget.cllPlugin);

		// Copy the archive file path
		strcpy(szPathToArc, clsPathToArc);

		// Get GetPicture()
		GetPictureProc GetPicture = reinterpret_cast<GetPictureProc>(pstsiTarget.cllPlugin.GetProcAddress(_T("GetPicture")));
		if (GetPicture == nullptr)
		{
			// GetPicture() function is not implemented
			continue;
		}

		YCLocalMemory cllmBitmapInfo;
		YCLocalMemory cllmBitmapData;

		// Call GetPicture()
		if (GetPicture(szPathToArc, pclArc->GetOpenFileInfo()->start, 0x0000, &cllmBitmapInfo.GetHandle(), &cllmBitmapData.GetHandle(), nullptr, 0) != 0)
		{
			// GetPicture() has failed
			continue;
		}

		// Get image information
		LPBITMAPINFO pBMPInfo = (LPBITMAPINFO) cllmBitmapInfo.Lock();
		LPBYTE       pHBm = (LPBYTE) cllmBitmapData.Lock();
		size_t       HBmSize = cllmBitmapData.GetSize();

		// Image output
		CImage image;
		image.Init(pclArc, pBMPInfo->bmiHeader.biWidth, pBMPInfo->bmiHeader.biHeight, pBMPInfo->bmiHeader.biBitCount, (LPBYTE) pBMPInfo->bmiColors);
		image.Write(pHBm, HBmSize);
		image.Close();

		// Exit (Free resources)
		cllmBitmapInfo.Free();
		cllmBitmapData.Free();

		return true;
	}

	// File input - GetFile()

	YCLocalMemory cllmSrc;

	for (auto& supportPlugin : vtSupportPlugin)
	{
		// Copy archive file path
		char szPathToArc[MAX_PATH];
		strcpy(szPathToArc, clsPathToArc);

		// Get GetFile()
		GetFileProc GetFile = reinterpret_cast<GetFileProc>(supportPlugin->GetProcAddress(_T("GetFile")));
		if (GetFile == nullptr)
		{
			// GetFile() function is not supported
			continue;
		}

		// Call GetFile()
		if (GetFile(szPathToArc, pclArc->GetOpenFileInfo()->start, (LPSTR)&cllmSrc.GetHandle(), 0x0100, nullptr, 0) != 0)
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
	LPBYTE pbtSrc = nullptr;
	DWORD  dwSrcSize;

	if (cllmSrc.GetHandle() != nullptr)
	{
		// Successful GetFile()

		pbtSrc = (LPBYTE) cllmSrc.Lock();
		dwSrcSize = cllmSrc.GetSize();
	}
	else
	{
		// GetFile() has failed
		bGetFileSuccess = false;

		// Memory allocation
		dwSrcSize = pclArc->GetOpenFileInfo()->sizeCmp;

		cllmSrc.Alloc(LHND, dwSrcSize);
		pbtSrc = (LPBYTE) cllmSrc.Lock();

		// Reading
		pclArc->Read(pbtSrc, dwSrcSize);
		pclArc->SeekCur(-(INT64)dwSrcSize);
	}

	// Memory Input - GetPicture()
	for (auto& pstsiTarget : m_stsiMain)
	{
		if (pstsiTarget.bValidity == 0)
		{
			// Invalid Susie plugin
			continue;
		}

		// Get IsSupported()
		IsSupportedProc IsSupported = reinterpret_cast<IsSupportedProc>(pstsiTarget.cllPlugin.GetProcAddress(_T("IsSupported")));
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
		BYTE  abtSrcHeader[2048];
		DWORD dwCopySize = (dwSrcSize <= sizeof(abtSrcHeader)) ? dwSrcSize : sizeof(abtSrcHeader);

		ZeroMemory(abtSrcHeader, sizeof(abtSrcHeader));
		memcpy(abtSrcHeader, pbtSrc, dwCopySize);

		// Call IsSupported()
		if (!IsSupported(szPathToFile, (DWORD) abtSrcHeader))
		{
			// File is not supported
			continue;
		}

		// Get GetPicture()
		GetPictureProc GetPicture = reinterpret_cast<GetPictureProc>(pstsiTarget.cllPlugin.GetProcAddress(_T("GetPicture")));
		if (GetPicture == nullptr)
		{
			// GetPicture() function is not implemented
			continue;
		}

		YCLocalMemory cllmBitmapInfo;
		YCLocalMemory cllmBitmapData;

		// Call GetPicture()
		if (GetPicture((LPSTR)pbtSrc, dwSrcSize, 0x0001, &cllmBitmapInfo.GetHandle(), &cllmBitmapData.GetHandle(), nullptr, 0) != 0)
		{
			// GetPicture() has failed
			continue;
		}

		// Get image info
		LPBITMAPINFO pBMPInfo = (LPBITMAPINFO) cllmBitmapInfo.Lock();
		LPBYTE       pHBm = (LPBYTE) cllmBitmapData.Lock();
		size_t       HBmSize = cllmBitmapData.GetSize();

		// Image output
		CImage image;
		image.Init(pclArc, pBMPInfo->bmiHeader.biWidth, pBMPInfo->bmiHeader.biHeight, pBMPInfo->bmiHeader.biBitCount, (BYTE*) pBMPInfo->bmiColors);
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
	pclArc->OpenFile();
	pclArc->WriteFile(pbtSrc, dwSrcSize, pclArc->GetOpenFileInfo()->sizeCmp);
	pclArc->CloseFile();

	// Exit (Free resources)
	cllmSrc.Free();

	return true;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Load Susie Plugin

void CSusie::LoadSpi(const YCString& rfclsPathToSusieFolder)
{
	// Release / Free Susie Plugin
	m_stsiMain.clear();

	// Susie Plugin Path
	TCHAR szPathToSusiePlugin[MAX_PATH];

	lstrcpy(szPathToSusiePlugin, rfclsPathToSusieFolder);
	PathAppend(szPathToSusiePlugin, _T("*.spi"));

	// Susie Plugin Info
	YCFileFindSx          clffsSusie;
	std::vector<YCString> vcPathToSusiePlugin;

	clffsSusie.FindFile(vcPathToSusiePlugin, rfclsPathToSusieFolder, _T("*.spi"), false);

	// Get Susie Plugin Information
	YCIni cliSusie(SBL_STR_INI_SUSIE);
	cliSusie.SetSection(_T("Plugins"));

	size_t uIndex = 0;

	m_stsiMain.resize(vcPathToSusiePlugin.size());

	for (auto& pszPathToTarget : vcPathToSusiePlugin)
	{
		SSusieInfo* pstsiTarget = &m_stsiMain[uIndex];

		// Load SPI
		if (!pstsiTarget->cllPlugin.Load(pszPathToTarget))
		{
			// Loading failed
			continue;
		}

		// Validate SPI
		cliSusie.SetKey(PathFindFileName(pszPathToTarget));
		cliSusie.ReadDec(&pstsiTarget->bValidity, 1);

		// SPI File Path Information
		pstsiTarget->clsPath = pszPathToTarget;

		// SPI Folder Name Information
		pstsiTarget->clsName = PathFindFileName(pszPathToTarget);

		// Get SPI Info
		char szBuffer[1024];

		GetPluginInfoProc GetPluginInfo = (GetPluginInfoProc) pstsiTarget->cllPlugin.GetProcAddress(_T("GetPluginInfo"));
		if (GetPluginInfo != nullptr)
		{
			if (GetPluginInfo(0, szBuffer, sizeof(szBuffer)) != 0)
			{
				// Successful at getting SPI info
				pstsiTarget->clsVersion = szBuffer;
			}

			if (GetPluginInfo(1, szBuffer, sizeof(szBuffer)) != 0)
			{
				// Successful at getting SPI info
				pstsiTarget->clsInfo = szBuffer;
			}

			for (int nInfoNumber = 2; ; nInfoNumber += 2)
			{
				if (GetPluginInfo(nInfoNumber, szBuffer, sizeof(szBuffer)) == 0)
				{
					// Could not get any more info
					break;
				}

				if (!pstsiTarget->clsSupportFormat.IsEmpty())
				{
					pstsiTarget->clsSupportFormat += _T(";");
				}

				pstsiTarget->clsSupportFormat += szBuffer;
			}
		}

		// Check if the SPI has a configuration dialog
		ConfigurationDlgProc ConfigurationDlg = (ConfigurationDlgProc) pstsiTarget->cllPlugin.GetProcAddress(_T("ConfigurationDlg"));
		pstsiTarget->bConfig = (ConfigurationDlg != nullptr);

		// Prepare the next SPI
		uIndex++;
	}

	// Load segments
	m_stsiMain.resize(uIndex);
}

//////////////////////////////////////////////////////////////////////////////////////////
// Save Susie Plugin Information

void CSusie::SaveSpi()
{
	YCIni cliSusie(SBL_STR_INI_SUSIE);
	cliSusie.SetSection(_T("Plugins"));

	// Delete info relating to Susie plugins that no longer exist.
	cliSusie.DeleteSection();

	// Save Susie plugin info
	for (const auto& pstsiTarget : m_stsiMain)
	{
		cliSusie.SetKey(pstsiTarget.clsName);
		cliSusie.WriteDec(pstsiTarget.bValidity);
	}
}
