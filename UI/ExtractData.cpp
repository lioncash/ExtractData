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

bool g_bThreadEnd;

CExtractData::CExtractData()
{
	GetModuleFileName(nullptr, m_tmp_file_list_path, MAX_PATH);
	PathRemoveFileSpec(m_tmp_file_list_path);
	PathAppend(m_tmp_file_list_path, _T("TmpFileList.txt"));
}

CExtractData::~CExtractData() = default;

void CExtractData::Init(HWND parent, SOption& option, CMainListView& listview)
{
	m_parent_window = parent;
	m_parent_inst = reinterpret_cast<HINSTANCE>(GetWindowLongPtr(parent, GWLP_HINSTANCE));
	m_options = &option;
	m_list_view = &listview;
}

void CExtractData::Open(LPTSTR open_dir)
{
	std::vector<TCHAR> file_names(MAX_PATH * 1000);
	CFileDialog file_dialog;

	if (file_dialog.DoModal(m_parent_window, file_names.data(), open_dir))
	{
		Mount(file_names.data());
	}
}

void CExtractData::OpenHistory(const YCString& file_path)
{
	std::vector<TCHAR> file_names(32000);
	lstrcpy(file_names.data(), file_path);
	Mount(file_names.data());
}

void CExtractData::OpenDrop(WPARAM wp)
{
	HDROP drop_handle = reinterpret_cast<HDROP>(wp);
	const UINT num_files = DragQueryFile(drop_handle, -1, nullptr, 0);
	std::vector<TCHAR> file_names(MAX_PATH * num_files);

	DragQueryFile(drop_handle, 0, file_names.data(), MAX_PATH);

	if (num_files >= 2)
	{
		// Gets the first file name
		TCHAR filename[MAX_PATH];
		lstrcpy(filename, PathFindFileName(file_names.data()));

		// Get the directory name
		PathRemoveFileSpec(file_names.data());

		// The following refers to the '\0'
		TCHAR* filename_ptr = &file_names[lstrlen(file_names.data()) + 1];

		// Attach filename following the '\0'
		memcpy(filename_ptr, filename, lstrlen(filename));
		filename_ptr += lstrlen(filename) + 1;

		for (int i = 1; i < static_cast<int>(num_files); i++)
		{
			DragQueryFile(drop_handle, i, filename, MAX_PATH);
			memcpy(filename_ptr, PathFindFileName(filename), lstrlen(PathFindFileName(filename)));
			filename_ptr += lstrlen(PathFindFileName(filename)) + 1;
		}
	}

	DragFinish(drop_handle);

	Mount(file_names.data());
}

void CExtractData::Close()
{
	// Close all open archive files
	m_archives.clear();

	// Close the decoding class
	CExtract::Close();

	// Delete all temporary files
	DeleteTmpFile();

	// Clear the list view
	m_list_view->Clear();
}

void CExtractData::Mount(LPCTSTR archive_names)
{
	Close(); // Close the last opened 
	m_archive_names = archive_names;
	m_input = true;
	DialogBoxParam(m_parent_inst, _T("PROGBAR"), m_parent_window, reinterpret_cast<DLGPROC>(WndStaticProc), reinterpret_cast<LPARAM>(this));
	m_list_view->Show();
}

UINT WINAPI CExtractData::MountThread(LPVOID param)
{
	auto* obj = static_cast<CExtractData*>(param);

	try
	{
		SOption* options = obj->m_options;
		LPCTSTR all_archive_names = obj->m_archive_names;
		std::vector<SFileInfo>& file_info = obj->m_list_view->GetFileInfo();

		std::vector<YCString> archive_names;
		archive_names.emplace_back(all_archive_names);
		all_archive_names += lstrlen(all_archive_names) + 1; // Because of ZeroMemory, we have to add one for the null terminator char.

		// If you select more than one
		if (lstrcmp(all_archive_names, _T("")) != 0)
		{
			// Get directory name
			TCHAR tmp_dir[_MAX_DIR];
			lstrcpy(tmp_dir, archive_names[0]);
			PathAddBackslash(tmp_dir);
			const YCString dir(tmp_dir);
			archive_names.clear();

			// Directory Name + Filename in the list.
			while (lstrcmp(all_archive_names, _T("")) != 0)
			{
				const YCString archive_name(all_archive_names);
				archive_names.push_back(dir + archive_name);
				all_archive_names += lstrlen(all_archive_names) + 1;
			}

			std::sort(archive_names.begin(), archive_names.end());
		}

		// Entire file size
		u64 total_archive_size = 0;
		for (auto itr = archive_names.begin(); itr != archive_names.end(); )
		{
			// Open the archive file
			auto archive = std::make_unique<CArcFile>();

			if (archive->Open(*itr))
			{
				// Add archive filesize
				total_archive_size += archive->GetArcSize();
				obj->m_archives.push_back(std::move(archive));
				++itr;
			}
			else
			{
				// Unable to open archive. Remove its name from the list.
				itr = archive_names.erase(itr);
			}
		}

		// Initialize the progress bar
		CProgBar prog;
		prog.Init(obj->m_window, total_archive_size);

		// Reading
		u32 archive_id = 0;
		for (auto& archive : obj->m_archives)
		{
			archive->SetArcID(archive_id);
			archive->SetEnt(file_info);
			archive->SetProg(prog);
			archive->SetOpt(options);

			// View the archive filename
			prog.SetArcName(archive->GetArcName());

			// Corresponding file
			if (CExtract::Mount(archive.get()))
			{
				archive_id++;
				archive->SetState(true);
			}
		}

		// Progress towards 100%
		prog.UpdatePercent();
	}
	catch (const std::bad_alloc&)
	{
		// Out of memory
		CError error;
		error.bad_alloc(obj->m_window);
	}
	catch (...)
	{
		// or if canceled
	}

	PostMessage(obj->m_window, WM_THREAD_END, 0, 0);

	_endthreadex(0);

	return 0;
}

void CExtractData::Save(ExtractMode extract_mode, LPTSTR save_dir, bool convert)
{
	if (m_options->bSaveSel)
	{
		// Specifies output destination
		CFolderInputDialog folder_input_dialog;

		if (folder_input_dialog.DoModal(m_parent_window, save_dir) == IDOK)
		{
			Decode(extract_mode, save_dir, convert);
		}
	}
	else if (m_options->bSaveSrc)
	{
		// Output to input destination
		Decode(extract_mode, nullptr, convert);
	}
	else if (m_options->bSaveDir)
	{
		// Output to a fixed destination
		Decode(extract_mode, m_options->SaveDir, convert);
	}
}

void CExtractData::SaveSel(LPTSTR save_dir, bool convert)
{
	Save(ExtractMode::Select, save_dir, convert);
}

void CExtractData::SaveAll(LPTSTR save_dir, bool convert)
{
	Save(ExtractMode::All, save_dir, convert);
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

void CExtractData::Decode(ExtractMode extract_mode, LPCTSTR save_dir, bool convert)
{
	CExistsDialog exists_dialog;
	exists_dialog.GetOverWrite() = 0x01; // Perform Overwrite Confirmation
	m_extract_mode = extract_mode;
	m_save_dir = save_dir;
	m_convert = convert;
	m_input = false;
	DialogBoxParam(m_parent_inst, _T("PROGBAR"), m_parent_window, reinterpret_cast<DLGPROC>(WndStaticProc), reinterpret_cast<LPARAM>(this));
}

void CExtractData::DecodeTmp()
{
	CExistsDialog exists_dialog;
	exists_dialog.GetOverWrite() = 0x00; // Overwrite confirmation is not perform
	m_extract_mode = ExtractMode::Select;
	m_save_dir = m_options->TmpDir;
	m_convert = true;
	m_input = false;
	DialogBoxParam(m_parent_inst, _T("PROGBAR"), m_parent_window, reinterpret_cast<DLGPROC>(WndStaticProc), reinterpret_cast<LPARAM>(this));
}

UINT WINAPI CExtractData::DecodeThread(LPVOID param)
{
	auto* obj = static_cast<CExtractData*>(param);
	CArcFile* archive = nullptr;

	try
	{
		const SOption* options  = obj->m_options;
		const LPCTSTR obj_save_dir = obj->m_save_dir;
		const bool convert = obj->m_convert;

		// Determine entire filesize
		std::vector<int> selects;
		u64 total_file_size = 0;
		const auto& archives = obj->m_archives;
		if (obj->m_extract_mode == ExtractMode::Select)
		{
			int item = -1;
			while ((item = obj->m_list_view->GetNextItem(item)) != -1)
			{
				selects.push_back(item);
				total_file_size += archives[0]->GetFileInfo(item)->size_org;
			}
		}
		else
		{
			const size_t num_items = archives[0]->GetFileInfo().size();
			for (int item = 0; item < static_cast<int>(num_items); item++)
			{
				selects.push_back(item);
				total_file_size += archives[0]->GetFileInfo(item)->size_org;
			}
		}

		// Initialize progressbar
		CProgBar prog;
		prog.Init(obj->m_window, total_file_size);

		for (size_t i = 0; i < selects.size(); i++)
		{
			const SFileInfo* info = archives[0]->GetFileInfo(selects[i]);
			archive = archives[info->arc_id].get();
			archive->SetProg(prog);

			// Create destination folder name from the destination filename input
			if (obj_save_dir == nullptr && options->bSaveSrc)
			{
				// Get input
				TCHAR save_dir[MAX_PATH];
				lstrcpy(save_dir, archive->GetArcPath());

				// Delete the filename
				PathRemoveFileSpec(save_dir);

				if (archive->GetCtEnt() == 1)
				{
					// Archive folder creation stores files one by one
					TCHAR arc_dir_name[_MAX_DIR];
					lstrcpy(arc_dir_name, PathFindFileName(save_dir));
					PathAppend(save_dir, _T("_"));
					lstrcat(save_dir, arc_dir_name);
				}
				else
				{
					// Create a folder with the name of the archive files if there are more than two files present
					PathAppend(save_dir, _T("_"));
					lstrcat(save_dir, archive->GetArcName());
				}
				archive->SetSaveDir(save_dir);
			}
			else
			{
				archive->SetSaveDir(obj_save_dir);
			}

			archive->SetFileInfo(selects[i]);

			// View filename
			prog.SetFileName(info->name);

			// Extraction
			CExtract::Decode(archive, convert);

			// Close the file (For when I forget to close it)
			archive->CloseFile();
		}
	}
	catch (const std::bad_alloc&)
	{
		// Out of memory error
		CError error;
		error.bad_alloc(obj->m_window);
	}
	catch (...)
	{
		// Or if we cancel the operation
		//CError error;
		//error.Message(obj->m_hWnd, "");
		// Corresponds to the case where the file was still not closed, even with the exception thrown.
		//if (archive != nullptr)
		//	archive->CloseFile();
	}

	// Corresponds to the case where the file was still not closed, even with the exception thrown.
	if (archive != nullptr)
		archive->CloseFile();

	// Send message to terminate the thread
	PostMessage(obj->m_window, WM_THREAD_END, 0, 0);

	_endthreadex(0);

	return 0;
}

void CExtractData::OpenRelate()
{
	int item = -1;

	while ((item = m_list_view->GetNextItem(item)) != -1)
	{
		std::set<YCString>& tmp_file_paths = m_list_view->GetFileInfo()[item].tmp_file_paths;

		if (!tmp_file_paths.empty())
		{
			const YCString& tmp_file_path = *tmp_file_paths.begin();

			// Open from file association

			if (::ShellExecute(nullptr, nullptr, tmp_file_path, nullptr, nullptr, SW_SHOWNORMAL) == reinterpret_cast<HINSTANCE>(SE_ERR_NOASSOC))
			{
				// If there is no association with the file, issue a 
				// dialog to select an application which can open the file
				const YCString params = _T("shell32.dll, OpenAs_RunDLL ") + tmp_file_path;

				::ShellExecute(nullptr, nullptr, _T("rundll32.exe"), params, nullptr, TRUE);
			}

			// Added to the temporary file list
			for (const YCString& str : tmp_file_paths)
			{
				if (str != _T(""))
				{
					m_tmp_file_paths.insert(str);
				}
			}

			tmp_file_paths.clear();
		}
	}
}

void CExtractData::DeleteTmpFile()
{
	// Add the last remaining temporary files
	LoadTmpFileList();

	for (auto iter = m_tmp_file_paths.begin(); iter != m_tmp_file_paths.end(); )
	{
		TCHAR tmp[MAX_PATH];
		lstrcpy(tmp, *iter);

		// Delete temporary files
		if (PathFileExists(tmp))
		{
			// File exists

			if (!DeleteFile(tmp))
			{
				// Fails to remove it
				++iter;
				continue;
			}
		}

		while (lstrcmp(tmp, m_options->TmpDir) != 0)
		{
			// Delete folder
			if (!PathRemoveFileSpec(tmp))
			{
				break;
			}

			RemoveDirectory(tmp);
		}

		iter = m_tmp_file_paths.erase(iter);
	}

	// Save the list of remaining temp files
	SaveTmpFileList();
}

void CExtractData::LoadTmpFileList()
{
	CFile tmp_file;
	if (!tmp_file.OpenForRead(m_tmp_file_list_path))
	{
		// Failed to open import file
		return;
	}

	while (true)
	{
		char buf[MAX_PATH];

		if (tmp_file.ReadLine(buf, sizeof(buf), true) == 0)
			break;

		m_tmp_file_paths.insert(buf);
	}
}

void CExtractData::SaveTmpFileList()
{
	if (m_tmp_file_paths.empty())
	{
		// Was able to remove all
		DeleteFile(m_tmp_file_list_path);
		return;
	}

	CFile tmp_file;
	if (!tmp_file.OpenForWrite(m_tmp_file_list_path))
	{
		// Failed to open file for writing
		return;
	}

	for (const YCString& str : m_tmp_file_paths)
	{
		tmp_file.WriteLine(str);
		tmp_file.Write("\r\n", 2);
	}

	m_tmp_file_paths.clear();
}

LRESULT CExtractData::WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
	static HANDLE thread;
	UINT thread_id;

	switch (msg)
	{
		case WM_INITDIALOG:
		{
			CWindowBase::Init();

			g_bThreadEnd = false;

			SetFocus(GetDlgItem(hwnd, IDC_CANCEL));
			if (m_input)
			{
				// Reading
				SetWindowText(hwnd, _T("Reading the file..."));
				PostMessage(hwnd, WM_INPUT_FILE, 0, 0);
			}
			else
			{
				// Extraction
				SetWindowText(hwnd, _T("Extracting..."));
				PostMessage(hwnd, WM_OUTPUT_FILE, 0, 0);
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
				g_bThreadEnd = true;
				return FALSE;
			}
		}
		return FALSE;

		case WM_INPUT_FILE:
		{
			// Information Acquisition Start
			thread = reinterpret_cast<HANDLE>(_beginthreadex(nullptr, 0, MountThread, this, 0, &thread_id));
		}
		return FALSE;

		case WM_OUTPUT_FILE:
		{
			// Start extraction process
			thread = reinterpret_cast<HANDLE>(_beginthreadex(nullptr, 0, DecodeThread, this, 0, &thread_id));
		}
		return FALSE;

		case WM_THREAD_END:
		{
			// Finish the process
			WaitForSingleObject(thread, INFINITE);
			CloseHandle(thread);
			EndDialog(hwnd, IDOK);
		}
		return TRUE;
	}

	return FALSE;
}
