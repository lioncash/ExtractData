#include "StdAfx.h"
#include "ExtractData.h"

#include "ArcFile.h"
#include "File.h"
#include "Common.h"
#include "res/ResExtractData.h"

#include "UI/Dialog/ExistsDialog.h"
#include "UI/Dialog/FileDialog.h"
#include "UI/Dialog/FolderInputDialog.h"
#include "UI/MainListView.h"
#include "UI/ProgressBar.h"

#define WM_INPUT_FILE  (WM_USER+1)
#define WM_OUTPUT_FILE (WM_USER+2)
#define WM_THREAD_END  (WM_USER+3)

BOOL g_bThreadEnd;

CExtractData::CExtractData()
{
	m_hParentWnd = nullptr;
	m_hParentInst = nullptr;
	m_pOption = nullptr;
	m_pListView = nullptr;

	GetModuleFileName(nullptr, m_szPathToTmpFileList, MAX_PATH);
	PathRemoveFileSpec(m_szPathToTmpFileList);
	PathAppend(m_szPathToTmpFileList, _T("TmpFileList.txt"));
}

CExtractData::~CExtractData() = default;

void CExtractData::Init(HWND hWnd, SOption& option, CMainListView& listview)
{
	m_hParentWnd = hWnd;
	m_hParentInst = reinterpret_cast<HINSTANCE>(GetWindowLongPtr(hWnd, GWLP_HINSTANCE));
	m_pOption = &option;
	m_pListView = &listview;
}

void CExtractData::Open(LPTSTR pOpenDir)
{
	std::vector<TCHAR> szFileNames(MAX_PATH * 1000, 0);

	CFileDialog clFileDlg;

	if (clFileDlg.DoModal(m_hParentWnd, &szFileNames[0], pOpenDir))
	{
		Mount(&szFileNames[0]);
	}
}

void CExtractData::OpenHistory(const YCString& file_path)
{
	std::vector<TCHAR> szFileNames(32000, 0);
	lstrcpy(&szFileNames[0], file_path);
	Mount(&szFileNames[0]);
}

void CExtractData::OpenDrop(WPARAM wp)
{
	HDROP hDrop = reinterpret_cast<HDROP>(wp);
	UINT uFileNo = DragQueryFile(hDrop, -1, nullptr, 0);

	std::vector<TCHAR> szFileNames(MAX_PATH * uFileNo, 0);

	DragQueryFile(hDrop, 0, &szFileNames[0], MAX_PATH);

	if (uFileNo >= 2)
	{
		// Gets the first file name
		TCHAR filename[MAX_PATH];
		lstrcpy(filename, PathFindFileName(&szFileNames[0]));

		// Get the directory name
		PathRemoveFileSpec(&szFileNames[0]);

		// The following refers to the '\0'
		TCHAR *pszFileName = &szFileNames[lstrlen(&szFileNames[0]) + 1];

		// Attach filename following the '\0'
		memcpy(pszFileName, filename, lstrlen(filename));
		pszFileName += lstrlen(filename) + 1;

		for (int i = 1; i < static_cast<int>(uFileNo); i++)
		{
			DragQueryFile(hDrop, i, filename, MAX_PATH);
			memcpy(pszFileName, PathFindFileName(filename), lstrlen(PathFindFileName(filename)));
			pszFileName += lstrlen(PathFindFileName(filename)) + 1;
		}
	}

	DragFinish(hDrop);

	Mount(&szFileNames[0]);
}

void CExtractData::Close()
{
	// Close all open archive files
	m_ArcList.clear();

	// Close the decoding class
	CExtract::Close();

	// Delete all temporary files
	DeleteTmpFile();

	// Clear the list view
	m_pListView->Clear();
}

void CExtractData::Mount(LPCTSTR c_pclArcNames)
{
	Close(); // Close the last opened 
	m_pclArcNames = c_pclArcNames;
	m_bInput = true;
	DialogBoxParam(m_hParentInst, _T("PROGBAR"), m_hParentWnd, reinterpret_cast<DLGPROC>(WndStaticProc), reinterpret_cast<LPARAM>(this));
	m_pListView->Show();
}

UINT WINAPI CExtractData::MountThread(LPVOID lpParam)
{
	CExtractData* pObj = static_cast<CExtractData*>(lpParam);

	try
	{
		SOption* pOption = pObj->m_pOption;
		LPCTSTR pclArcNames = pObj->m_pclArcNames;
		std::vector<SFileInfo>& rEnt = pObj->m_pListView->GetFileInfo();

		std::vector<YCString> sArcNameList;

		YCString sArcNames(pclArcNames);
		sArcNameList.push_back(sArcNames);
		pclArcNames += lstrlen(pclArcNames) + 1; // Because of ZeroMemory, we have to add one for the null terminator char.

		// If you select more than one
		if (lstrcmp(pclArcNames, _T("")) != 0)
		{
			// Get directory name
			TCHAR tmpDir[_MAX_DIR];
			lstrcpy(tmpDir, sArcNameList[0]);
			PathAddBackslash(tmpDir);
			YCString Dir(tmpDir);
			sArcNameList.clear();

			// Directory Name + Filename in the list.
			while (lstrcmp(pclArcNames, _T("")) != 0)
			{
				YCString sArcName(pclArcNames);
				sArcNameList.push_back(Dir + sArcName);
				pclArcNames += lstrlen(pclArcNames) + 1;
			}

			sort(sArcNameList.begin(), sArcNameList.end());
		}

		// Entire file size
		QWORD AllArcSize = 0;
		for (auto itr = sArcNameList.begin(); itr != sArcNameList.end(); )
		{
			// Open the archive file
			auto archive = std::make_unique<CArcFile>();

			if (archive->Open(*itr))
			{
				// Add archive filesize
				AllArcSize += archive->GetArcSize();
				pObj->m_ArcList.push_back(std::move(archive));
				++itr;
			}
			else
			{
				// Unable to open archive. Remove its name from the list.
				itr = sArcNameList.erase(itr);
			}
		}

		// Initialize the progress bar
		CProgBar prog;
		prog.Init(pObj->m_window, AllArcSize);

		// Reading
		DWORD dwArcID = 0;
		for (auto& archive : pObj->m_ArcList)
		{
			archive->SetArcID(dwArcID);
			archive->SetEnt(rEnt);
			archive->SetProg(prog);
			archive->SetOpt(pOption);

			// View the archive filename
			prog.SetArcName(archive->GetArcName());

			// Corresponding file
			if (CExtract::Mount(archive.get()))
			{
				dwArcID++;
				archive->SetState(true);
			}
		}
		//MessageBox(pObj->m_hWnd, "", "", MB_OK);
		// Progress towards 100%
		prog.UpdatePercent();
	}
	catch (std::bad_alloc)
	{
		// Out of memory
		CError error;
		error.bad_alloc(pObj->m_window);
	}
	catch (...)
	{
		// or if canceled
	}

	PostMessage(pObj->m_window, WM_THREAD_END, 0, 0);

	_endthreadex(0);

	return 0;
}

void CExtractData::Save(DWORD ExtractMode, LPTSTR pSaveDir, bool convert)
{
	SOption* pstOption = m_pOption;

	if (pstOption->bSaveSel)
	{
		// Specifies output destination

		CFolderInputDialog clFolderInputDlg;

		if (clFolderInputDlg.DoModal(m_hParentWnd, pSaveDir ) == IDOK)
		{
			Decode(ExtractMode, pSaveDir, convert);
		}
	}
	else if (pstOption->bSaveSrc)
	{
		// Output to input destination

		Decode(ExtractMode, nullptr, convert);
	}
	else if (pstOption->bSaveDir)
	{
		// Output to a fixed destination

		Decode(ExtractMode, pstOption->SaveDir, convert);
	}
}

void CExtractData::SaveSel(LPTSTR pSaveDir, bool convert)
{
	Save(EXTRACT_SELECT, pSaveDir, convert);
}

void CExtractData::SaveAll(LPTSTR pSaveDir, bool convert)
{
	Save(EXTRACT_ALL, pSaveDir, convert);
}

void CExtractData::SaveDrop()
{
/*
	YCString sFiles;
	YCString sSaveDir = m_pOption->TmpDir + _T('\\');
	CMainListView* pListView = m_pListView;
	int nItem = -1;
	while ((nItem = pListView->GetNextItem(nItem)) != -1)
	{
		sFiles += sSaveDir + pListView->GetFileInfo()[nItem].name;
		sFiles += _T('\0');
	}
	sFiles += _T('\0');

	CDataObject* pObj = new CDataObject();
	CDropSource* pSrc = new CDropSource();

	// Set the structure of FORMATETC
	FORMATETC fmtetc;
	fmtetc.cfFormat = CF_HDROP;
	fmtetc.ptd = nullptr;
	fmtetc.dwAspect = DVASPECT_CONTENT;
	fmtetc.lindex = -1;
	fmtetc.tymed = TYMED_HGLOBAL;

	// Set the structure of STGMEDIUM
	STGMEDIUM medium;
	medium.tymed = TYMED_HGLOBAL;
	medium.hGlobal = GlobalAlloc(GMEM_MOVEABLE, sizeof(DROPFILES) + sFiles.size());
	medium.pUnkForRelease = nullptr;
	LPTSTR p = (LPTSTR)GlobalLock(medium.hGlobal);
	((DROPFILES*)p)->pFiles = sizeof(DROPFILES);
	((DROPFILES*)p)->fWide = FALSE;
	CopyMemory(p + sizeof(DROPFILES), sFiles, sFiles.size());
	GlobalUnlock(medium.hGlobal);

	// Sets IDataObject
	pObj->SetData(&fmtetc, &medium, FALSE);

	// Starts OLE Drag & Drop
	DWORD effect;
	HRESULT hr = DoDragDrop(pObj, pSrc, DROPEFFECT_MOVE | DROPEFFECT_COPY | DROPEFFECT_LINK, &effect);

	// Free resources
	pObj->Release();
	pSrc->Release();
*/
}

void CExtractData::Decode(DWORD ExtractMode, LPCTSTR pSaveDir, bool convert)
{
	CExistsDialog ExistsDlg;
	ExistsDlg.GetOverWrite() = 0x01; // Perform Overwrite Confirmation
	m_dwExtractMode = ExtractMode;
	m_pSaveDir = pSaveDir;
	m_bConvert = convert;
	m_bInput = false;
	DialogBoxParam(m_hParentInst, _T("PROGBAR"), m_hParentWnd, reinterpret_cast<DLGPROC>(WndStaticProc), reinterpret_cast<LPARAM>(this));
}

void CExtractData::DecodeTmp()
{
	CExistsDialog ExistsDlg;
	ExistsDlg.GetOverWrite() = 0x00; // Overwrite confirmation is not perform
	m_dwExtractMode = EXTRACT_SELECT;
	m_pSaveDir = m_pOption->TmpDir;
	m_bConvert = true;
	m_bInput = false;
	DialogBoxParam(m_hParentInst, _T("PROGBAR"), m_hParentWnd, reinterpret_cast<DLGPROC>(WndStaticProc), reinterpret_cast<LPARAM>(this));
}

UINT WINAPI CExtractData::DecodeThread(LPVOID lpParam)
{
	CExtractData* pObj = static_cast<CExtractData*>(lpParam);
	CArcFile* pclArc = nullptr;

	try
	{
		SOption* pOption  = pObj->m_pOption;
		LPCTSTR  pSaveDir = pObj->m_pSaveDir;
		bool     convert = pObj->m_bConvert;

		// Determine entire filesize
		std::vector<int> nSelects;
		QWORD AllFileSize = 0;
		auto& rArcList = pObj->m_ArcList;
		if (pObj->m_dwExtractMode == EXTRACT_SELECT)
		{
			int nItem = -1;
			while ((nItem = pObj->m_pListView->GetNextItem(nItem)) != -1)
			{
				nSelects.push_back(nItem);
				AllFileSize += rArcList[0]->GetFileInfo(nItem)->sizeOrg;
			}
		}
		else
		{
			size_t nItemCount = rArcList[0]->GetFileInfo().size();
			for (int nItem = 0; nItem < static_cast<int>(nItemCount); nItem++)
			{
				nSelects.push_back(nItem);
				AllFileSize += rArcList[0]->GetFileInfo(nItem)->sizeOrg;
			}
		}

		// Initialize progressbar
		CProgBar prog;
		prog.Init(pObj->m_window, AllFileSize);

		for (size_t i = 0; i < nSelects.size(); i++)
		{
			SFileInfo* pInfFile = rArcList[0]->GetFileInfo(nSelects[i]);
			pclArc = rArcList[pInfFile->arcID].get();
			pclArc->SetProg(prog);

			// Create destination folder name from the destination filename input
			if (pSaveDir == nullptr && pOption->bSaveSrc == TRUE)
			{
				TCHAR SaveDir[MAX_PATH];

				// Get input
				lstrcpy(SaveDir, pclArc->GetArcPath());

				// Delete the filename
				PathRemoveFileSpec(SaveDir);

				if (pclArc->GetCtEnt() == 1)
				{
					// Archive folder creation stores files one by one
					TCHAR ArcDirName[_MAX_DIR];
					lstrcpy(ArcDirName, PathFindFileName(SaveDir));
					PathAppend(SaveDir, _T("_"));
					lstrcat(SaveDir, ArcDirName);
				}
				else
				{
					// Create a folder with the name of the archive files if there are more than two files present
					PathAppend(SaveDir, _T("_"));
					lstrcat(SaveDir, pclArc->GetArcName());
				}
				pclArc->SetSaveDir(SaveDir);
			}
			else
			{
				pclArc->SetSaveDir(pSaveDir);
			}

			pclArc->SetFileInfo(nSelects[i]);

			// View filename
			prog.SetFileName(pInfFile->name);

			// Extraction
			CExtract::Decode(pclArc, convert);

			// Close the file (For when I forget to close it)
			pclArc->CloseFile();
		}
		//MessageBox(pObj->m_hWnd, "", "", MB_OK);
	}
	catch (std::bad_alloc)
	{
		// Out of memory error
		CError error;
		error.bad_alloc(pObj->m_window);
	}
	catch (...)
	{
		// Or if we cancel the operation
		//CError error;
		//error.Message(pObj->m_hWnd, "");
		// Corresponds to the case where the file was still not closed, even with the exception thrown.
		//if (pclArc != nullptr)
		//	pclArc->CloseFile();
	}

	// Corresponds to the case where the file was still not closed, even with the exception thrown.
	if (pclArc != nullptr)
		pclArc->CloseFile();

	// Send message to terminate the thread
	PostMessage(pObj->m_window, WM_THREAD_END, 0, 0);

	_endthreadex(0);

	return 0;
}

void CExtractData::OpenRelate()
{
	CMainListView* pListView = m_pListView;
	int            nItem = -1;

	while ((nItem = pListView->GetNextItem(nItem)) != -1)
	{
		std::set<YCString>&	sTmpFilePathList = pListView->GetFileInfo()[nItem].sTmpFilePath;

		if (!sTmpFilePathList.empty())
		{
			const YCString&	 clsTmpFilePath = *sTmpFilePathList.begin();

			// Open from file association

			if (::ShellExecute(nullptr, nullptr, clsTmpFilePath, nullptr, nullptr, SW_SHOWNORMAL) == reinterpret_cast<HINSTANCE>(SE_ERR_NOASSOC))
			{
				// If there is no association with the file, issue a 
				// dialog to select an application which can open the file

				YCString clsParam = _T("shell32.dll, OpenAs_RunDLL ") + clsTmpFilePath;

				::ShellExecute(nullptr, nullptr, _T("rundll32.exe"), clsParam, nullptr, TRUE);
			}

			// Added to the temporary file list

			for (const YCString& str : sTmpFilePathList)
			{
				if (str != _T(""))
				{
					m_ssTmpFile.insert(str);
				}
			}

			sTmpFilePathList.clear();
		}
	}
}

void CExtractData::DeleteTmpFile()
{
	// Add the last remaining temporary files
	LoadTmpFileList();

	for (auto iter = m_ssTmpFile.begin(); iter != m_ssTmpFile.end(); )
	{
		TCHAR szTmp[MAX_PATH];
		lstrcpy(szTmp, *iter);

		// Delete temporary files
		if (PathFileExists(szTmp))
		{
			// File exists

			if (!DeleteFile(szTmp))
			{
				// Fails to remove it
				++iter;
				continue;
			}
		}

		while (lstrcmp(szTmp, m_pOption->TmpDir) != 0)
		{
			// Delete folder
			if (!PathRemoveFileSpec(szTmp))
			{
				break;
			}

			RemoveDirectory(szTmp);
		}

		iter = m_ssTmpFile.erase(iter);
	}

	// Save the list of remaining temp files
	SaveTmpFileList();
}

void CExtractData::LoadTmpFileList()
{
	CFile clfTmpFileList;
	if (clfTmpFileList.Open(m_szPathToTmpFileList, CFile::FILE_READ) == INVALID_HANDLE_VALUE)
	{
		// Failed to open import file
		return;
	}

	while (true)
	{
		char buf[MAX_PATH];

		if (clfTmpFileList.ReadLine(buf, sizeof(buf), true) == 0)
			break;

		m_ssTmpFile.insert(buf);
	}
}

void CExtractData::SaveTmpFileList()
{
	if (m_ssTmpFile.empty())
	{
		// Was able to remove all
		DeleteFile(m_szPathToTmpFileList);
		return;
	}

	CFile clfTmpFileList;
	if (clfTmpFileList.Open(m_szPathToTmpFileList, CFile::FILE_WRITE) == INVALID_HANDLE_VALUE)
	{
		// Failed to open file for writing
		return;
	}

	for (const YCString& str : m_ssTmpFile)
	{
		clfTmpFileList.WriteLine(str);
		clfTmpFileList.Write("\r\n", 2);
	}

	m_ssTmpFile.clear();
}

LRESULT CExtractData::WndProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp)
{
	static HANDLE hThread;
	UINT thId;

	switch (msg)
	{
		case WM_INITDIALOG:
		{
			CWindowBase::Init();

			g_bThreadEnd = FALSE;

			SetFocus(GetDlgItem(hWnd, IDC_CANCEL));
			if (m_bInput)
			{
				// Reading
				SetWindowText(hWnd, _T("Reading the file..."));
				PostMessage(hWnd, WM_INPUT_FILE, 0, 0);
			}
			else
			{
				// Extraction
				SetWindowText(hWnd, _T("Extracting..."));
				PostMessage(hWnd, WM_OUTPUT_FILE, 0, 0);
			}
		}
		return FALSE;

		case WM_COMMAND:
		{
			switch (LOWORD(wp))
			{
			case IDCANCEL:
			case IDC_CANCEL:
				// Cancel is pressed
				g_bThreadEnd = TRUE;
				return FALSE;
			}
		}
		return FALSE;

		case WM_INPUT_FILE:
		{
			// Information Acquisition Start
			hThread = reinterpret_cast<HANDLE>(_beginthreadex(nullptr, 0, MountThread, this, 0, &thId));
			//hThread = CreateThread(NULL, 0, MountThread, this, 0, &thId);
		}
		return FALSE;

		case WM_OUTPUT_FILE:
		{
			// Start extraction process
			hThread = reinterpret_cast<HANDLE>(_beginthreadex(nullptr, 0, DecodeThread, this, 0, &thId));
			//hThread = CreateThread(NULL, 0, DecodeThread, this, 0, &thId);
		}
		return FALSE;

		case WM_THREAD_END:
		{
			// Finish the process
			WaitForSingleObject(hThread, INFINITE);
			CloseHandle(hThread);
			EndDialog(hWnd, IDOK);
		}
		return TRUE;
	}

	return FALSE;
}
