#include "stdafx.h"
#include "../Common.h"
#include "../ExtractData.h"
#include "../Ctrl/ListView.h"
#include "../Ctrl/Button.h"
#include "../Ctrl/Font.h"
#include "../ArcFile.h"
#include "DataBase.h"

CDataBase::CDataBase()
{
	// データベースファイルをフルパスで取得
	TCHAR modulePath[MAX_PATH];
	GetModuleFileName(NULL, modulePath, MAX_PATH);
	PathRemoveFileSpec(modulePath);
	wsprintf(m_DBFilePath, _T("%s\\database.dat"), modulePath);
}

CDataBase::~CDataBase()
{

}

// データベースからファイルパスとファイル数を取得する関数
BOOL CDataBase::ReadFileInfo()
{/*
	CFile DBFile;
	if (DBFile.Open(m_DBFilePath, FILE_READ) == INVALID_HANDLE_VALUE)
		return FALSE;

	tstring buf;
	while (DBFile.ReadLine(buf) == TRUE) {
		if ((buf >= _T("A:")) && (buf <= _T("Z:"))) {
			FILEINFODB infFile = {0, buf, 0};

			// ファイル位置取得
			size_t offset = buf.length();
			file.start = DataBaseFile.tellg();
			file.start -= offset + 2;

			// ファイル内容取得
			while (getline(DataBaseFile, buf)) {
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

// 選択アイテムを削除する関数
int CDataBase::DelFileInfo()
{
	//HWND DataBaseList = m_DataBaseList;
	//std::vector<DBFI> *pDBFileInfoList = &m_DBFileInfoList;

	//// 何も選択していなければ削除しない
	//if (ListView_GetSelectedCount(DataBaseList) <= 0)
	//	return -1;

	//// UNDO取得
	//GetUndo();

	//// 選択されていないアイテムを取得
	//std::vector<DBFI> NoSelectedItemList;
	//int FocusedItem = -1, FirstSelectedItem = -1;
	//int nItemCount = pDBFileInfoList->size();
	//for (int i = 0; i < nItemCount; i++) {
	//	UINT uiState = ListView_GetItemState(DataBaseList, i, LVIS_SELECTED | LVIS_FOCUSED);
	//	// フォーカス状態のアイテム番号取得
	//	if ((FocusedItem == -1) && (uiState & LVIS_FOCUSED))
	//		FocusedItem = i;
	//	if ((FirstSelectedItem == -1) && (uiState & LVIS_SELECTED))
	//		FirstSelectedItem = i;
	//	else if (!(uiState & LVIS_SELECTED))
	//		NoSelectedItemList.push_back((*pDBFileInfoList)[i]);
	//}

	//// DBFileInfoListに選択されていないアイテムを格納することで、選択されたアイテムを削除することができる(eraseだと遅い)
	//pDBFileInfoList->clear();
	//*pDBFileInfoList = NoSelectedItemList;
	//NoSelectedItemList.clear();

	//SendMessage(DataBaseList, WM_SETREDRAW, FALSE, 0);

	//// リストビュー更新
	//ListView listview(m_hWnd);
	//listview.Update(DataBaseList, *pDBFileInfoList, FirstSelectedItem);

	//nItemCount = pDBFileInfoList->size();
	//// 連続して削除出来るようにアイテムを選択状態にする
	//if (FocusedItem < nItemCount) { // 何故か中括弧をつけないとコンパイルできない
	//	ListView_SetItemState(DataBaseList, FocusedItem, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
	//}
	//// 総項目数を超えていたら一番下のアイテムにフォーカス
	//else
	//	ListView_SetItemState(DataBaseList, nItemCount - 1, LVIS_FOCUSED, LVIS_FOCUSED);

	//SendMessage(DataBaseList, WM_SETREDRAW, TRUE, 0);

	//// REDO取得
	//GetRedo();

	return 0;
}

// 編集後の内容に従ってデータベースを保存
BOOL CDataBase::SaveFileInfo()
{
	//CFile DBFile;
	//if (DBFile.Open(m_DBFilePath, FILE_READ) == INVALID_HANDLE_VALUE)
	//	return FALSE

	//std::vector<DBFI>& pEntDB = m_entDB;

	//// データベースファイルの容量取得
	//QWORD FileSize = DBFile.GetFileSize();

	//// ファイルサイズが0だったら保存する必要がないので終了
	//if (FileSize == 0)
	//	return -1;

	//try {
	//	// 容量分のメモリを割り当てる
	//	char *str = new char[FileSize + 1];
	//	ZeroMemory(str, FileSize + 1);
	//	char *pstr = str;

	//	// strに保存するデータベースを読み込む
	//	size_t FileCount = pDBFileInfoList->size();
	//	for (size_t i = 0; i < FileCount; i++) {
	//		rDataBaseFile.seekg((*pDBFileInfoList)[i].start);

	//		std::string buf;
	//		while (getline(rDataBaseFile, buf)) {
	//			buf += "\n";
	//			size_t len = buf.length();
	//			for (size_t j = 0; j < len; j++)
	//				*pstr++ = buf[j];
	//			if (buf == "\n")
	//				break;
	//		}
	//	}

	//	// database.txtに書き込み
	//	std::ofstream wDataBaseFile(m_DataBaseName);
	//	wDataBaseFile << str;

	//	delete []str;
	//}
	//catch (std::bad_alloc) {
	//	TCHAR DataBaseName_tmp[MAX_PATH];
	//	lstrcpy(DataBaseName_tmp, m_DataBaseName);
	//	PathRenameExtension(DataBaseName_tmp, _T(".tmp"));
	//	CopyFile(m_DataBaseName, DataBaseName_tmp, FALSE);

	//	std::ifstream DataBaseFile_tmp(DataBaseName_tmp);
	//	std::ofstream DataBaseFile(m_DataBaseName);

	//	// database.txtに書き込み
	//	size_t FileCount = pDBFileInfoList->size();
	//	for (DWORD i = 0; i < FileCount; i++) {
	//		DataBaseFile_tmp.seekg((*pDBFileInfoList)[i].start);

	//		std::string buf;
	//		while (getline(DataBaseFile_tmp, buf)) {
	//			DataBaseFile << buf << '\n';
	//			if (buf == "")
	//				break;
	//		}
	//	}
	//}

	return 0;
}

// 選択したアイテム番号をリストに格納する関数
int CDataBase::GetSelItem(std::vector<int> *SelectedItemList)
{
	//HWND DataBaseList = m_DataBaseList;

	//if (ListView_GetSelectedCount(DataBaseList) <= 0)
	//	return -1;

	//int nItem = -1;
	//while (1) {
	//	nItem = ListView_GetNextItem(DataBaseList, nItem, LVNI_ALL | LVNI_SELECTED);
	//	if (nItem == -1)
	//		break;
	//	SelectedItemList->push_back(nItem);
	//}

	return 0;
}

// UndoRedoListをクリアする関数
void CDataBase::ClearUndoRedo()
{
	//std::vector<UndoRedo> *pUndoRedoList = &m_UndoRedoList;
	//if (!pUndoRedoList->empty()) {
	//	size_t size = pUndoRedoList->size();
	//	for (int i = 0; i < (int)size; i++) {
	//		(*pUndoRedoList)[i].DBFileInfoList.clear();
	//		(*pUndoRedoList)[i].SelectedItemList.clear();
	//	}
	//	pUndoRedoList->clear();
	//}
}

// UNDOを取得する関数
int CDataBase::GetUndo()
{
	//// 前回のUndoRedoList消去
	//ClearUndoRedo();

	//HWND DataBaseList = m_DataBaseList;
	//std::vector<UndoRedo> *pUndoRedoList = &m_UndoRedoList;

	//// 新たなUndo作成
	//UndoRedo newUndo;
	//newUndo.DBFileInfoList = m_DBFileInfoList;
	//GetSelItem(&newUndo.SelectedItemList);
	//newUndo.FocusedItem = ListView_GetNextItem(DataBaseList, -1, LVNI_ALL | LVNI_FOCUSED);
	//// UndoRedoListに追加
	//pUndoRedoList->push_back(newUndo);

	return 0;
}

// REDOを取得する関数
int CDataBase::GetRedo()
{
	//HWND DataBaseList = m_DataBaseList;
	//std::vector<UndoRedo> *pUndoRedoList = &m_UndoRedoList;

	//// 新たなRedo作成
	//UndoRedo newRedo;
	//newRedo.DBFileInfoList = m_DBFileInfoList;
	//GetSelItem(&newRedo.SelectedItemList);
	//newRedo.FocusedItem = ListView_GetNextItem(DataBaseList, -1, LVNI_ALL | LVNI_FOCUSED);
	//// UndoRedoListに追加
	//pUndoRedoList->push_back(newRedo);

	//m_Redo_flag = false;

	return 0;
}

// 取り消しを行う関数
int CDataBase::SetUndo()
{
	//HWND DataBaseList = m_DataBaseList;
	//std::vector<DBFI> *pDBFileInfoList = &m_DBFileInfoList;
	//std::vector<UndoRedo> *pUndoRedoList = &m_UndoRedoList;
	//bool Redo_flag = m_Redo_flag;

	//// UndoRedoListが空だったら何もしない
	//if (pUndoRedoList->empty())
	//	return -1;

	//// リストアイテム復元
	//SendMessage(DataBaseList, WM_SETREDRAW, FALSE, 0);
	//*pDBFileInfoList = (*pUndoRedoList)[Redo_flag].DBFileInfoList;
	//ListView listview(m_hWnd);
	//listview.Update(DataBaseList, *pDBFileInfoList, ListView_GetTopIndex(DataBaseList));

	//// 選択状態復元
	//size_t size = (*pUndoRedoList)[Redo_flag].SelectedItemList.size();
	//for (int i = 0; i < (int)size; i++)
	//	ListView_SetItemState(DataBaseList, (*pUndoRedoList)[Redo_flag].SelectedItemList[i], LVIS_SELECTED, LVIS_SELECTED);

	//// フォーカス復元
	//ListView_SetItemState(DataBaseList, (*pUndoRedoList)[Redo_flag].FocusedItem, LVIS_FOCUSED, LVIS_FOCUSED);
	//SendMessage(DataBaseList, WM_SETREDRAW, TRUE, 0);

	//Redo_flag ^= 1;
	//m_Redo_flag = Redo_flag;

	return 0;
}

void CDataBase::DoModal(HWND hWnd)
{
	HINSTANCE hInst = (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE);
	DialogBoxParam(hInst, _T("DATABASE"), hWnd, (DLGPROC)WndStaticProc, (LPARAM)this);
}

// データベース編集ダイアログ
LRESULT CDataBase::WndProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp)
{
	switch (msg) {
		case WM_INITDIALOG:
		{
			// データベースリスト作成
			m_listview.Create(hWnd, *m_pOption);
			// 各種ボタン作成
			m_DelButton.Create(hWnd, _T("削除"), ID_DB_BUTTON1);
			m_UndoButton.Create(hWnd, _T("元に戻す"), ID_DB_BUTTON2);
			m_AllSelButton.Create(hWnd, _T("すべて選択"), ID_DB_BUTTON3);
			m_EndButton.Create(hWnd, _T("編集終了"), ID_DB_BUTTON4);
			// 親ウィンドウの中央に表示する座標を求める
			RECT rc;
			GetWindowRect(GetParent(hWnd), (LPRECT)&rc);
			int width = 600;
			int height = 400;
			int left = rc.left + ((rc.right - rc.left) - width) / 2;
			int top = rc.top + ((rc.bottom - rc.top) - height) / 2;
			// ウィンドウ位置・サイズ設定
			//SaveLoadIni slini(hWnd);
			//slini.LoadWindowPlacement(idsDataBaseWindow, left, top, width, height);
			//slini.LoadListView(idsDataBaseList, &sort_param, 1);

			// データベースファイル読み込み
			ReadFileInfo();
			// ソートして表示
			m_listview.Sort();

			SetFocus(hWnd);
			return FALSE;
		}

		case WM_COMMAND:
		{
			switch (LOWORD(wp)) {
				case ID_DB_BUTTON1: // 削除
				//case IDM_DEL:
					DelFileInfo();
					m_listview.SetFocus();
					return FALSE;

				case ID_DB_BUTTON2: // 取り消し
				//case IDM_UNDO:
					SetUndo();
					m_listview.SetFocus();
					return FALSE;

				case ID_DB_BUTTON3: // すべて選択
				//case IDM_SELECTALL:
					m_listview.SetAllItemSelect();
					m_listview.SetFocus();
					return FALSE;

				case ID_DB_BUTTON4: // 編集終了
					SaveFileInfo();
				case IDCANCEL: // キャンセル
					m_EndButton.SetFocus(); // 落ちるのを防ぐ

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
			if (wp == ID_LISTVIEW2) { //IDチェック
				LPNMLISTVIEW pNM = (LPNMLISTVIEW)lp;
				switch (pNM->hdr.code) {
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

// 1度読み込んだことのあるファイルかどうかをデータベースファイルからチェックする関数
BOOL CDataBase::Check(CArcFile* pArc)
{
	//OPTION* pOption = pArc->GetOpt();

	//// データベース機能を使わない場合はFALSEを返し、ファイルから情報を取得する
	//if (pOption->bUseDB == FALSE)
	//	return FALSE;

	//CFileMap DBFile;
	//if (DBFile.Open(m_DBFilePath) == NULL)
	//	return FALSE;

	//// ヘッダチェック
	//if (memcmp(DBFile.GetFilePointer(), "EDDB", 4) != 0)
	//	return FALSE;

	//// アーカイブ数取得
	//DBFile.Seek(4, FILE_CURRENT);
	//LPDWORD pCtArc = (LPDWORD)DBFile.GetFilePointer();

	//// インデックスサイズ取得
	//DBFile.Seek(4, FILE_CURRENT);
	//LPDWORD pIndexSize = (LPDWORD)DBFile.GetFilePointer();

	//// インデックス取得
	//DBFile.Seek(4, FILE_CURRENT);
	//LPBYTE pIndex = (LPBYTE)DBFile.GetFilePointer();

	//// 前に一度読み込んだことがないかチェック
	//BOOL bOpen = FALSE;
	//DWORD IndexOffset = 0;
	//for (DWORD i = 0; i < *pCtArc; i++) {
	//	DWORD nArcPathLen = *(LPDWORD)&pIndex[0];
	//	if (memcmp(pArc->GetArcPath().c_str(), &pIndex[4], nArcPathLen) == 0) {
	//		bOpen = TRUE;
	//		break;
	//	}
	//	DWORD offset = strlen(pIndex) + 17;
	//	pIndex += offset;
	//	IndexOffset += offset;
	//}

	//// 読み込んだことがなければFALSEを返す
	//if (bOpen == FALSE)
	//	return FALSE;

	

	//tstring buf;
	//while (DBFile.ReadLine(buf) == TRUE) {
	//	// データベースに同じファイル名があったら同ファイルと判断し、データベースからファイル情報取得
	//	if (buf == pArc->GetArcPath()) {
	//		//std::vector<CFI> *pCFileInfoList = &g_CFileInfoList;
	//		size_t size_prev = pArc->GetFileInfo().size();
	//		tstring sFileExt(_T("."));
	//		sFileExt += pOption->szExtMode;

	//		while (DBFile.ReadLine(buf) == TRUE) {
	//			// 改行まで読んだら抜ける
	//			if (buf == _T(""))
	//				break;

	//			FILEINFO infFile;
	//			//TCHAR filename[_MAX_FNAME];
	//			//buf >> infFile.name >> infFile.sizeOrg >> infFile.start;
	//			std::istringstream is(buf);
	//			is >> infFile.name >> infFile.sizeOrg >> infFile.start;
	//			//sscanf(buf.c_str(), "%s %d %x", filename, &cfile.size, &cfile.start);

	//			// モードに合った拡張子のファイル情報だけ取得(標準モードのときはすべて取得)
	//			if ((lstrcmp(pOption->szExtMode, _T("標準")) == 0) || (lstrcmpi(PathFindExtension(infFile.name.c_str()), sFileExt.c_str()) == 0)) {
	//				infFile.sizeCmp = infFile.sizeOrg;
	//				infFile.end = infFile.start + infFile.sizeOrg;
	//				infFile.format = extract.SetFileFormat(cfile.name);
	//				pArc->SetFileInfo(infFile);
	//			}
	//			// MPG抽出モードでsfdファイルが読まれたら取得
	//			else if ((lstrcmp(pOption.ExtMode, _T("MPG")) == 0) && (lstrcmpi(PathFindExtension(infFile.name.c_str()), _T(".sfd")) == 0)) {
	//				infFile.sizeCmp = infFile.sizeOrg;
	//				infFile.end = infFile.start + infFile.sizeOrg;
	//				infFile.format = extract.SetFileFormat(cfile.name);
	//				pArc->SetFileInfo(infFile);
	//			}
	//		}

	//		// 増えていなかったらこのモードでは読み込んだことがないと判断
	//		if (size_prev == pArc->GetFileInfo().size())
	//			return FALSE;
	//		else
	//			return TRUE;
	//	}
	//}

	return FALSE;
}

// 読み込んだ情報をデータベースファイルに書き込む関数
BOOL CDataBase::Add(CArcFile* pArc)
{
	// データベース機能を使わない場合はデータベースファイルに書き込まない
	//if (pArc->GetOpt()->bUseDB == FALSE)
	//	return FALSE;

	//std::vector<FILEINFO>& pEnt = pArc->GetFileInfo();
	//size_t nStartEnt = pArc->GetStartEnt();
	//size_t nEndEnt = pEnt.size();
	//size_t ctFile = nEndEnt - nStartEnt;

	//// 最後まで読み込み、モードに合った拡張子のファイルが見つかったときデータベースファイルに書き込む
	//if (ctFile == 0)
	//	return FALSE;

	//CFileMap DBFile;
	//if (DBFile.Open(m_DBFilePath) == NULL)
	//	return FALSE;

	//// ヘッダチェック
	//if (memcmp(DBFile.GetFilePointer(), "EDDB", 4) != 0)
	//	return FALSE;

	//// アーカイブ数取得
	//DBFile.Seek(4, FILE_CURRENT);
	//LPDWORD pCtArc = (LPDWORD)DBFile.GetFilePointer();

	//// インデックスサイズ取得
	//DBFile.Seek(4, FILE_CURRENT);
	//LPDWORD pIndexSize = (LPDWORD)DBFile.GetFilePointer();

	//// インデックス取得
	//DBFile.Seek(4, FILE_CURRENT);
	//LPBYTE pIndex = (LPBYTE)DBFile.GetFilePointer();

	//// 前に一度読み込んだことがないかチェック
	//BOOL bOpen = FALSE;
	//DWORD IndexOffset = 0;
	//for (DWORD i = 0; i < *pCtArc; i++) {
	//	if (pArc->GetArcPath() == pIndex) {
	//		bOpen = TRUE;
	//		break;
	//	}
	//	DWORD offset = strlen(pIndex) + 17;
	//	pIndex += offset;
	//	IndexOffset += offset;
	//}

	//DBFile.Seek(IndexOffset, FILE_CURRENT);

	//if (bOpen == TRUE) {
	//	
	//}
	//else {
	//	// ヘッダ更新
	//	(*pCtArc)++;
	//	*pIndexSize += pArc->GetArcPath().length() + 13;

	//	// インデックス追加
	//	DBFile.Append(pArc->GetArcPath().c_str(), pArc->GetArcPath().length() + 1);
	//	LPDWORD pDataSize = (LPDWORD)DBFile.Append(&ctFile, 4);
	//	LPDWORD pOffset = DBFile.Append(NULL, 4);
	//	DBFile.Append(NULL, 4);

	//	// データ追加
	//	LPDWORD offset = DBFile.SeekEnd();
	//	for (size_t i = nStartEnt; i < nEndEnt; i++) {
	//		DBFile.Append(pEnt[i].name.c_str(), pEnt[i].name.length() + 1);
	//		DBFile.Append(pEnt[i].sizeOrg, 8);
	//		DBFile.Append(pEnt[i].start, 8);
	//	}

	//	*pDataSize = DBFile.GetFilePointer() - offset;
	//	*pOffset = *offset - *pIndexSize;
	//}

	return TRUE;
}