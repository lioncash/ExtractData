#include "StdAfx.h"
#include "UI/DataBase/DataBase.h"

#include "ArcFile.h"
#include "Common.h"
#include "ExtractData.h"
#include "UI/Ctrl/Button.h"
#include "UI/Ctrl/Font.h"
#include "UI/Ctrl/ListView.h"

CDataBase::CDataBase()
{
	// Retrieves the full path and database file
	TCHAR modulePath[MAX_PATH];
	GetModuleFileName(NULL, modulePath, MAX_PATH);
	PathRemoveFileSpec(modulePath);
	wsprintf(m_DBFilePath, _T("%s\\database.dat"), modulePath);
}

CDataBase::~CDataBase()
{
}

// Function to retrieve a number from the database file path and file
BOOL CDataBase::ReadFileInfo()
{
/*
	CFile DBFile;
	if (DBFile.Open(m_DBFilePath, FILE_READ) == INVALID_HANDLE_VALUE)
		return FALSE;

	tstring buf;
	while (DBFile.ReadLine(buf) == TRUE)
	{
		if ((buf >= _T("A:")) && (buf <= _T("Z:")))
		{
			FILEINFODB infFile = {0, buf, 0};

			// Get file position
			size_t offset = buf.length();
			file.start = DataBaseFile.tellg();
			file.start -= offset + 2;

			// Get file contents
			while (getline(DataBaseFile, buf))
			{
				if (buf == _T(""))
					break;
				file.count++;
			}

			DBFileInfoList->push_back(file);
		}
	}
*/
	return 0;
}

// Function to delete the selected item
int CDataBase::DelFileInfo()
{
/*
	HWND DataBaseList = m_DataBaseList;
	std::vector<DBFI> *pDBFileInfoList = &m_DBFileInfoList;

	// Do not delete anything unless it is selected
	if (ListView_GetSelectedCount(DataBaseList) <= 0)
		return -1;

	// Get UNDO
	GetUndo();

	// Retrieve the items that are not selected
	std::vector<DBFI> NoSelectedItemList;
	int FocusedItem = -1, FirstSelectedItem = -1;
	int nItemCount = pDBFileInfoList->size();
	for (int i = 0; i < nItemCount; i++)
	{
		UINT uiState = ListView_GetItemState(DataBaseList, i, LVIS_SELECTED | LVIS_FOCUSED);
		// Get the item number of the focus state
		if ((FocusedItem == -1) && (uiState & LVIS_FOCUSED))
			FocusedItem = i;
		if ((FirstSelectedItem == -1) && (uiState & LVIS_SELECTED))
			FirstSelectedItem = i;
		else if (!(uiState & LVIS_SELECTED))
			NoSelectedItemList.push_back((*pDBFileInfoList)[i]);
	}

	// By storing items that are not selected in the DBFileInfoList (slow to erase) it is possible to deleted the selected item
	pDBFileInfoList->clear();
	*pDBFileInfoList = NoSelectedItemList;
	NoSelectedItemList.clear();

	SendMessage(DataBaseList, WM_SETREDRAW, FALSE, 0);

	// Update listview
	ListView listview(m_hWnd);
	listview.Update(DataBaseList, *pDBFileInfoList, FirstSelectedItem);

	nItemCount = pDBFileInfoList->size();
	// State so that you can continually remove selected items
	if (FocusedItem < nItemCount) // Can not compile without a brace for some reason
	{
		ListView_SetItemState(DataBaseList, FocusedItem, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
	}
	// Item focus at the bottom has exceeded the total number of items
	else
		ListView_SetItemState(DataBaseList, nItemCount - 1, LVIS_FOCUSED, LVIS_FOCUSED);

	SendMessage(DataBaseList, WM_SETREDRAW, TRUE, 0);

	// Get REDO
	GetRedo();
*/

	return 0;
}

// After editing, save the contents of the database
BOOL CDataBase::SaveFileInfo()
{
/*
	CFile DBFile;
	if (DBFile.Open(m_DBFilePath, FILE_READ) == INVALID_HANDLE_VALUE)
		return FALSE

	std::vector<DBFI>& pEntDB = m_entDB;

	// Obtain the capacity of the database file
	QWORD FileSize = DBFile.GetFileSize();

	// End because there is no need to save when the file size is 0
	if (FileSize == 0)
		return -1;

	try
	{
		// Allocate memory 
		char *str = new char[FileSize + 1];
		ZeroMemory(str, FileSize + 1);
		char *pstr = str;

		// Read the database string that you want to save to str
		size_t FileCount = pDBFileInfoList->size();
		for (size_t i = 0; i < FileCount; i++)
		{
			rDataBaseFile.seekg((*pDBFileInfoList)[i].start);

			std::string buf;
			while (getline(rDataBaseFile, buf))
			{
				buf += "\n";
				size_t len = buf.length();
				for (size_t j = 0; j < len; j++)
					*pstr++ = buf[j];
				if (buf == "\n")
					break;
			}
		}

		// Write database.txt
		std::ofstream wDataBaseFile(m_DataBaseName);
		wDataBaseFile << str;

		delete []str;
	}
	catch (std::bad_alloc)
	{
		TCHAR DataBaseName_tmp[MAX_PATH];
		lstrcpy(DataBaseName_tmp, m_DataBaseName);
		PathRenameExtension(DataBaseName_tmp, _T(".tmp"));
		CopyFile(m_DataBaseName, DataBaseName_tmp, FALSE);

		std::ifstream DataBaseFile_tmp(DataBaseName_tmp);
		std::ofstream DataBaseFile(m_DataBaseName);

		// Write database.txt
		size_t FileCount = pDBFileInfoList->size();
		for (DWORD i = 0; i < FileCount; i++)
		{
			DataBaseFile_tmp.seekg((*pDBFileInfoList)[i].start);

			std::string buf;
			while (getline(DataBaseFile_tmp, buf))
			{
				DataBaseFile << buf << '\n';
				if (buf == "")
					break;
			}
		}
	}
*/

	return 0;
}

// Function to store the selected items in a list
int CDataBase::GetSelItem(std::vector<int> *SelectedItemList)
{
/*
	HWND DataBaseList = m_DataBaseList;

	if (ListView_GetSelectedCount(DataBaseList) <= 0)
		return -1;

	int nItem = -1;
	while (1)
	{
		nItem = ListView_GetNextItem(DataBaseList, nItem, LVNI_ALL | LVNI_SELECTED);
		if (nItem == -1)
			break;
		SelectedItemList->push_back(nItem);
	}
*/

	return 0;
}

// Function to clear the UndoRedoList
void CDataBase::ClearUndoRedo()
{
/*
	std::vector<UndoRedo> *pUndoRedoList = &m_UndoRedoList;
	if (!pUndoRedoList->empty())
	{
		size_t size = pUndoRedoList->size();
		for (int i = 0; i < (int)size; i++)
		{
			(*pUndoRedoList)[i].DBFileInfoList.clear();
			(*pUndoRedoList)[i].SelectedItemList.clear();
		}
		pUndoRedoList->clear();
	}
*/
}

// Function that retrieves UNDO
int CDataBase::GetUndo()
{
/*
	// Erase previous UndoRedoList
	ClearUndoRedo();

	HWND DataBaseList = m_DataBaseList;
	std::vector<UndoRedo> *pUndoRedoList = &m_UndoRedoList;

	// Create a new undo
	UndoRedo newUndo;
	newUndo.DBFileInfoList = m_DBFileInfoList;
	GetSelItem(&newUndo.SelectedItemList);
	newUndo.FocusedItem = ListView_GetNextItem(DataBaseList, -1, LVNI_ALL | LVNI_FOCUSED);
	// Add to UndoRedoList
	pUndoRedoList->push_back(newUndo);
*/

	return 0;
}

// Function that gets REDO
int CDataBase::GetRedo()
{
/*
	HWND DataBaseList = m_DataBaseList;
	std::vector<UndoRedo> *pUndoRedoList = &m_UndoRedoList;

	// Create a new redo
	UndoRedo newRedo;
	newRedo.DBFileInfoList = m_DBFileInfoList;
	GetSelItem(&newRedo.SelectedItemList);
	newRedo.FocusedItem = ListView_GetNextItem(DataBaseList, -1, LVNI_ALL | LVNI_FOCUSED);
	// Add to UndoRedoList
	pUndoRedoList->push_back(newRedo);

	m_Redo_flag = false;
*/

	return 0;
}

// Do the undo function
int CDataBase::SetUndo()
{
/*
	HWND DataBaseList = m_DataBaseList;
	std::vector<DBFI> *pDBFileInfoList = &m_DBFileInfoList;
	std::vector<UndoRedo> *pUndoRedoList = &m_UndoRedoList;
	bool Redo_flag = m_Redo_flag;

	// If nothing, empty list
	if (pUndoRedoList->empty())
		return -1;

	// Restore a list item
	SendMessage(DataBaseList, WM_SETREDRAW, FALSE, 0);
	*pDBFileInfoList = (*pUndoRedoList)[Redo_flag].DBFileInfoList;
	ListView listview(m_hWnd);
	listview.Update(DataBaseList, *pDBFileInfoList, ListView_GetTopIndex(DataBaseList));

	// Restore the selected state
	size_t size = (*pUndoRedoList)[Redo_flag].SelectedItemList.size();
	for (int i = 0; i < (int)size; i++)
		ListView_SetItemState(DataBaseList, (*pUndoRedoList)[Redo_flag].SelectedItemList[i], LVIS_SELECTED, LVIS_SELECTED);

	// Restore focus
	ListView_SetItemState(DataBaseList, (*pUndoRedoList)[Redo_flag].FocusedItem, LVIS_FOCUSED, LVIS_FOCUSED);
	SendMessage(DataBaseList, WM_SETREDRAW, TRUE, 0);

	Redo_flag ^= 1;
	m_Redo_flag = Redo_flag;
*/

	return 0;
}

void CDataBase::DoModal(HWND hWnd)
{
	HINSTANCE hInst = (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE);
	DialogBoxParam(hInst, _T("DATABASE"), hWnd, (DLGPROC)WndStaticProc, (LPARAM)this);
}

// Edit database dialog
LRESULT CDataBase::WndProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp)
{
	switch (msg)
	{
		case WM_INITDIALOG:
		{
			// Create database list
			m_listview.Create(hWnd, *m_pOption);
			// Create various buttons
			m_DelButton.Create(hWnd, _T("Edit"), ID_DB_BUTTON1);
			m_UndoButton.Create(hWnd, _T("Undo"), ID_DB_BUTTON2);
			m_AllSelButton.Create(hWnd, _T("Select all"), ID_DB_BUTTON3);
			m_EndButton.Create(hWnd, _T("Finish"), ID_DB_BUTTON4);
			// Seek the coordinates displayed in the center of the parent window
			RECT rc;
			GetWindowRect(GetParent(hWnd), (LPRECT)&rc);
			int width = 600;
			int height = 400;
			int left = rc.left + ((rc.right - rc.left) - width) / 2;
			int top = rc.top + ((rc.bottom - rc.top) - height) / 2;
			// Window position and size settings
			//SaveLoadIni slini(hWnd);
			//slini.LoadWindowPlacement(idsDataBaseWindow, left, top, width, height);
			//slini.LoadListView(idsDataBaseList, &sort_param, 1);

			// Read database file
			ReadFileInfo();
			// Sort
			m_listview.Sort();

			SetFocus(hWnd);
			return FALSE;
		}

		case WM_COMMAND:
		{
			switch (LOWORD(wp)) {
				case ID_DB_BUTTON1: // Edit
				//case IDM_DEL:
					DelFileInfo();
					m_listview.SetFocus();
					return FALSE;

				case ID_DB_BUTTON2: // Undo
				//case IDM_UNDO:
					SetUndo();
					m_listview.SetFocus();
					return FALSE;

				case ID_DB_BUTTON3: // Select all
				//case IDM_SELECTALL:
					m_listview.SetAllItemSelect();
					m_listview.SetFocus();
					return FALSE;

				case ID_DB_BUTTON4: // Finish
					SaveFileInfo();
				case IDCANCEL: // Cancel
					m_EndButton.SetFocus(); // Prevent lack of focus

					//SaveLoadIni slini(hWnd);
					//slini.SaveWindowPlacement(idsDataBaseWindow);
					//slini.SaveListView(idsDataBaseList, m_DataBaseList, sort_param);

					EndDialog(hWnd, IDOK);
					return TRUE;
			}
			return FALSE;
		}

		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			BeginPaint(hWnd, &ps);
			m_listview.SetFocus();
			EndPaint(hWnd, &ps);
			return FALSE;
		}

		case WM_NOTIFY:
		{
			if (wp == ID_LISTVIEW2) // ID check
			{
				LPNMLISTVIEW pNM = (LPNMLISTVIEW)lp;
				switch (pNM->hdr.code)
				{
					case LVN_COLUMNCLICK:
						m_listview.Sort(pNM->iSubItem);
						return FALSE;

					case LVN_GETDISPINFO:
					{
						NMLVDISPINFO* pDispInfo = (NMLVDISPINFO*)lp;
						if (pDispInfo->item.mask & LVIF_TEXT)
							m_listview.Show(pDispInfo);
						return FALSE;
					}
				}
			}
			return FALSE;
		}

		case WM_SIZE:
			m_listview.SetWindowPos(20, 20, LOWORD(lp) - 130, HIWORD(lp) - 40);
			m_DelButton.SetWindowPos(LOWORD(lp) - 95, 70, 80, 20);
			m_UndoButton.SetWindowPos(LOWORD(lp) - 95, 110, 80, 20);
			m_AllSelButton.SetWindowPos(LOWORD(lp) - 95, 150, 80, 20);
			m_EndButton.SetWindowPos(LOWORD(lp) - 95, 190, 80, 20);
			return FALSE;
	}

	return FALSE;
}

// Function to check whether the file from the database file may be read or not
BOOL CDataBase::Check(CArcFile* pArc)
{
/*
	OPTION* pOption = pArc->GetOpt();

	// If you do not want to use the database function, it returns FALSE
	if (pOption->bUseDB == FALSE)
		return FALSE;

	CFileMap DBFile;
	if (DBFile.Open(m_DBFilePath) == NULL)
		return FALSE;

	// Check header
	if (memcmp(DBFile.GetFilePointer(), "EDDB", 4) != 0)
		return FALSE;

	// Get archive count
	DBFile.Seek(4, FILE_CURRENT);
	LPDWORD pCtArc = (LPDWORD)DBFile.GetFilePointer();

	// Get index size
	DBFile.Seek(4, FILE_CURRENT);
	LPDWORD pIndexSize = (LPDWORD)DBFile.GetFilePointer();

	// Get index
	DBFile.Seek(4, FILE_CURRENT);
	LPBYTE pIndex = (LPBYTE)DBFile.GetFilePointer();

	// Check whether or not the file has already been read
	BOOL bOpen = FALSE;
	DWORD IndexOffset = 0;
	for (DWORD i = 0; i < *pCtArc; i++)
	{
		DWORD nArcPathLen = *(LPDWORD)&pIndex[0];
		if (memcmp(pArc->GetArcPath().c_str(), &pIndex[4], nArcPathLen) == 0)
		{
			bOpen = TRUE;
			break;
		}
		DWORD offset = strlen(pIndex) + 17;
		pIndex += offset;
		IndexOffset += offset;
	}

	// Returns false if the file has not been read
	if (bOpen == FALSE)
		return FALSE;

	tstring buf;
	while (DBFile.ReadLine(buf) == TRUE)
	{
		// Determines if the same file with the same name is in the database
		if (buf == pArc->GetArcPath())
		{
			//std::vector<CFI> *pCFileInfoList = &g_CFileInfoList;
			size_t size_prev = pArc->GetFileInfo().size();
			tstring sFileExt(_T("."));
			sFileExt += pOption->szExtMode;

			while (DBFile.ReadLine(buf) == TRUE)
			{
				// After reading up to the newline escape
				if (buf == _T(""))
					break;

				FILEINFO infFile;
				//TCHAR filename[_MAX_FNAME];
				//buf >> infFile.name >> infFile.sizeOrg >> infFile.start;
				std::istringstream is(buf);
				is >> infFile.name >> infFile.sizeOrg >> infFile.start;
				//sscanf(buf.c_str(), "%s %d %x", filename, &cfile.size, &cfile.start);

				// Only get file information that matches the mode of extension (When in standard mode, get everything)
				if ((lstrcmp(pOption->szExtMode, _T("Standard")) == 0) || (lstrcmpi(PathFindExtension(infFile.name.c_str()), sFileExt.c_str()) == 0))
				{
					infFile.sizeCmp = infFile.sizeOrg;
					infFile.end = infFile.start + infFile.sizeOrg;
					infFile.format = extract.SetFileFormat(cfile.name);
					pArc->SetFileInfo(infFile);
				}
				// Once the file is in get mode. sfd extraction mode
				else if ((lstrcmp(pOption.ExtMode, _T("MPG")) == 0) && (lstrcmpi(PathFindExtension(infFile.name.c_str()), _T(".sfd")) == 0))
				{
					infFile.sizeCmp = infFile.sizeOrg;
					infFile.end = infFile.start + infFile.sizeOrg;
					infFile.format = extract.SetFileFormat(cfile.name);
					pArc->SetFileInfo(infFile);
				}
			}

			// If in this mode, determine if the size has changed
			if (size_prev == pArc->GetFileInfo().size())
				return FALSE;
			else
				return TRUE;
		}
	}
*/

	return FALSE;
}

// Function that writes information to the database
BOOL CDataBase::Add(CArcFile* pArc)
{
/*
	// If you do not want to use the database
	if (pArc->GetOpt()->bUseDB == FALSE)
		return FALSE;

	std::vector<FILEINFO>& pEnt = pArc->GetFileInfo();
	size_t nStartEnt = pArc->GetStartEnt();
	size_t nEndEnt = pEnt.size();
	size_t ctFile = nEndEnt - nStartEnt;

	// Write to the database file when read to the end, the file extension that matches the mode is found
	if (ctFile == 0)
		return FALSE;

	CFileMap DBFile;
	if (DBFile.Open(m_DBFilePath) == NULL)
		return FALSE;

	// Check header
	if (memcmp(DBFile.GetFilePointer(), "EDDB", 4) != 0)
		return FALSE;

	// Get archive count
	DBFile.Seek(4, FILE_CURRENT);
	LPDWORD pCtArc = (LPDWORD)DBFile.GetFilePointer();

	// Get index size
	DBFile.Seek(4, FILE_CURRENT);
	LPDWORD pIndexSize = (LPDWORD)DBFile.GetFilePointer();

	// Get index
	DBFile.Seek(4, FILE_CURRENT);
	LPBYTE pIndex = (LPBYTE)DBFile.GetFilePointer();

	// Check whether or not the file has been read before
	BOOL bOpen = FALSE;
	DWORD IndexOffset = 0;
	for (DWORD i = 0; i < *pCtArc; i++)
	{
		if (pArc->GetArcPath() == pIndex)
		{
			bOpen = TRUE;
			break;
		}
		DWORD offset = strlen(pIndex) + 17;
		pIndex += offset;
		IndexOffset += offset;
	}

	DBFile.Seek(IndexOffset, FILE_CURRENT);

	if (bOpen == TRUE)
	{
	
	}
	else
	{
		// Update header
		(*pCtArc)++;
		*pIndexSize += pArc->GetArcPath().length() + 13;

		// Add index
		DBFile.Append(pArc->GetArcPath().c_str(), pArc->GetArcPath().length() + 1);
		LPDWORD pDataSize = (LPDWORD)DBFile.Append(&ctFile, 4);
		LPDWORD pOffset = DBFile.Append(NULL, 4);
		DBFile.Append(NULL, 4);

		// Add data
		LPDWORD offset = DBFile.SeekEnd();
		for (size_t i = nStartEnt; i < nEndEnt; i++)
		{
			DBFile.Append(pEnt[i].name.c_str(), pEnt[i].name.length() + 1);
			DBFile.Append(pEnt[i].sizeOrg, 8);
			DBFile.Append(pEnt[i].start, 8);
		}

		*pDataSize = DBFile.GetFilePointer() - offset;
		*pOffset = *offset - *pIndexSize;
	}
*/

	return TRUE;
}