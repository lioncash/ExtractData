#include "StdAfx.h"
#include "UI/WinMain.h"

#include "Common.h"
#include "ExtractData.h"
#include "LastDir.h"
#include "res/ResExtractData.h"
#include "Susie.h"
#include "UI/Ctrl/StatusBar.h"
#include "UI/Dialog/FileDialog.h"
#include "UI/Dialog/FolderDialog.h"
#include "UI/Dialog/VersionInfo.h"
#include "UI/MainListView.h"
#include "UI/MainToolBar.h"
#include "UI/Option.h"
#include "UI/WindowBase.h"

#pragma comment(lib, "ShLwApi.Lib")
#pragma comment(lib, "ComCtl32.Lib")
#pragma comment(lib, "imm32.lib")

#ifdef  _DEBUG
#pragma comment(lib, "zlibd.lib")
#pragma comment(lib, "libpngd.lib")
#else
#pragma comment(lib, "zlib.lib")
#pragma comment(lib, "libpng16.lib")
#endif

// Main Function
int WINAPI _tWinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPTSTR lpsCmdLine, int nCmdShow)
{
#ifdef _DEBUG
	::_CrtSetDbgFlag( _CRTDBG_LEAK_CHECK_DF | _CRTDBG_ALLOC_MEM_DF );
#endif

	CWinMain main;
	return main.WinMain(hInst, hPrevInst, lpsCmdLine, nCmdShow);
}

#define WINDOWCLASS_NAME   _T("ExtractDataClass")
#define WINDOW_NAME        _T("ExtractData")

CWinMain::CWinMain()
{
	m_hWnd = nullptr;
	ZeroMemory(&m_wc, sizeof(WNDCLASSEX));

	m_hInst = nullptr;
	m_hPrevInst = nullptr;
	m_lpsCmdLine = nullptr;
	m_nCmdShow = 0;
}

CWinMain::~CWinMain()
{
}

int CWinMain::WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPTSTR lpsCmdLine, int nCmdShow)
{
	// WinMain function receives an argument of the program, which is assigned to a member variable
	m_hInst = hInst;
	m_hPrevInst = hPrevInst;
	m_lpsCmdLine = lpsCmdLine;
	m_nCmdShow = nCmdShow;

	OleInitialize(nullptr);

	if (!InitApp())
		return FALSE;

	if (!InitInstance())
		return FALSE;

	HACCEL hAccel = LoadAccelerators(hInst, _T("MYACCEL"));
	if (!hAccel)
		return FALSE;

	MSG msg;
	int bRet;
	while ((bRet = GetMessage(&msg, nullptr, 0, 0)) != 0)
	{
		if (bRet == -1)
			break;

		if (!TranslateAccelerator(m_hWnd, hAccel, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	OleUninitialize();
	return static_cast<int>(msg.wParam);
}

// Window class registration
ATOM CWinMain::InitApp()
{
	WNDCLASSEX wc;
	wc.cbSize        = sizeof(WNDCLASSEX);
	wc.style         = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc   = static_cast<WNDPROC>(CWindowBase::WndStaticProc); // Procedure name
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = 0;
	wc.hInstance     = m_hInst;  // Instance
	wc.hIcon         = LoadIcon(m_hInst, MAKEINTRESOURCE(IDI_ICON_APP));
	wc.hCursor       = LoadCursor(nullptr, IDC_ARROW);
	wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_BTNFACE+1);
	wc.lpszMenuName  = _T("MAINMENU"); // Menu name
	wc.lpszClassName = WINDOWCLASS_NAME;
	wc.hIconSm       = LoadIcon(m_hInst, MAKEINTRESOURCE(IDI_ICON_APP));

	m_wc = wc;
	return (RegisterClassEx(&m_wc));
}

// Window creation
bool CWinMain::InitInstance()
{
	m_hWnd = CreateWindow(WINDOWCLASS_NAME,
		WINDOW_NAME,            // This name will be displayed in the title bar
		WS_OVERLAPPEDWINDOW,    // Type of window
		0,                      // X Coordinate
		0,                      // Y Coordinate
		0,                      // Width
		0,                      // Height
		nullptr,                // When creating the parent window handle, the parent is NULL
		nullptr,                // When creating the menu handle, the handle is NULL
		m_hInst,                // Instance handle
		static_cast<LPVOID>(this));
	if (!m_hWnd)
		return false;

	Init(idsMainWindow, 640, 480);
	UpdateWindow(m_hWnd);

	return true;
}

// Window Procedure
LRESULT CWinMain::WndProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp)
{
	static CMainToolBar MainToolBar;
	static CSearchToolBar SearchToolBar;
	static CMainListView listview;
	static CStatusBar statusbar;
	static CExtractData extract;
	static COption option;
	static CVersionInfo version;
	static CLastDir lastdir;
	static TCHAR ReadmeFileName[MAX_PATH], HistoryFileName[MAX_PATH], StateFileName[MAX_PATH];
	INITCOMMONCONTROLSEX ic;

	switch (msg)
	{
	case WM_CREATE:
		{
			// Allow D&D (Drag & Drop)
			DragAcceptFiles(hWnd, TRUE);

			option.Init(SearchToolBar, listview);

			// Initialization of the common installation configuration
			ic.dwSize = sizeof(INITCOMMONCONTROLSEX);
			ic.dwICC = ICC_WIN95_CLASSES;
			InitCommonControlsEx(&ic);

			// Create main toolbar
			MainToolBar.Create(hWnd);
			SearchToolBar.Create(hWnd);

			// Create list view
			listview.Create(hWnd, option.GetOpt());

			// Create status bar
			statusbar.Create(hWnd, option.GetOpt(), listview);

			// Get full path
			TCHAR ModulePath[MAX_PATH];
			GetModuleFileName(nullptr, ModulePath, MAX_PATH);
			PathRemoveFileSpec(ModulePath);

			// Retrieves full path of the location of Readme.txt
			lstrcpy(ReadmeFileName, ModulePath);
			PathAppend(ReadmeFileName, _T("Readme.txt"));

			// Retrieves full path of the location of History.txt
			lstrcpy(HistoryFileName, ModulePath);
			PathAppend(HistoryFileName, _T("History.txt"));

			// Retrieves full path of the location of State.txt
			lstrcpy(StateFileName, ModulePath);
			PathAppend(StateFileName, _T("State.txt"));

			extract.Init(hWnd, option.GetOpt(), listview);
			break;
		}

	case WM_DROPFILES:
		extract.OpenDrop(wp);
		MainToolBar.AddOpenHistory(extract.GetArcList());
		statusbar.SetCount();
		break;

	case WM_COMMAND:
		switch (LOWORD(wp))
		{
		case IDM_OPEN: // Open a file to load
			extract.Open(lastdir.GetOpen());
			MainToolBar.AddOpenHistory(extract.GetArcList());
			statusbar.SetCount();
			lastdir.SaveIni();
			break;

		case IDM_CLOSE: // Close the opened file
			extract.Close();
			statusbar.SetCount();
			break;

		// Open a file from history
		case ID_TOOLBAR_OPEN_HISTORY:
		case ID_TOOLBAR_OPEN_HISTORY+1:
		case ID_TOOLBAR_OPEN_HISTORY+2:
		case ID_TOOLBAR_OPEN_HISTORY+3:
		case ID_TOOLBAR_OPEN_HISTORY+4:
		case ID_TOOLBAR_OPEN_HISTORY+5:
		case ID_TOOLBAR_OPEN_HISTORY+6:
		case ID_TOOLBAR_OPEN_HISTORY+7:
		case ID_TOOLBAR_OPEN_HISTORY+8:
		case ID_TOOLBAR_OPEN_HISTORY+9:
			extract.OpenHistory(MainToolBar.GetHistory()[LOWORD(wp)-ID_TOOLBAR_OPEN_HISTORY]);
			MainToolBar.AddOpenHistory(extract.GetArcList());
			statusbar.SetCount();
			break;

		// Search button configuration file
		case IDM_AHX:
		case IDM_BMP:
		case IDM_JPG:
		case IDM_MID:
		case IDM_MPG:
		case IDM_OGG:
		case IDM_PNG:
		case IDM_WAV:
		case IDM_WMV:
			SearchToolBar.SetSearch(LOWORD(wp));
			break;

		// Quick settings
		case IDM_QUICKSET_STD_SEARCHOGG:
		case IDM_QUICKSET_EXTRACT_CREATEFOLDER:
		case IDM_QUICKSET_EXTRACT_FIXOGG:
		case IDM_QUICKSET_EXTRACT_EASYDECRYPT:
		case IDM_QUICKSET_EXTRACT_DSTPNG:
		case IDM_QUICKSET_EXTRACT_ALPHABLEND:
		case IDM_QUICKSET_SUSIE_USE:
		case IDM_QUICKSET_SUSIE_FIRST:
			SetQuickMenuItem(LOWORD(wp));
			break;

		case IDM_EXIT: // Exit
			SendMessage(hWnd, WM_CLOSE, 0, 0);
			break;

		case IDM_EXTRACT: // Extract the selected range
			if (listview.GetCountSel() > 0)
			{
				extract.SaveSel(lastdir.GetSave(), true);
			}
			lastdir.SaveIni();
			break;

		case IDM_EXTRACTALL: // Extract all
			if( listview.GetCount() > 0 )
			{
				extract.SaveAll(lastdir.GetSave(), true);
			}
			lastdir.SaveIni();
			break;

		case IDM_EXTRACT_NOTCONVERT: // Extracts the selection without converting it
			if (listview.GetCountSel() > 0)
			{
				extract.SaveSel(lastdir.GetSave(), false);
			}
			lastdir.SaveIni();
			break;

		case IDM_EXTRACTALL_NOTCONVERT: // Extracts all without conversion
			if (listview.GetCount() > 0)
			{
				extract.SaveAll(lastdir.GetSave(), false);
			}
			lastdir.SaveIni();
			break;

		case IDM_SELECTALL: // Select all
			listview.SetItemSelAll(LVIS_SELECTED);
			break;

		case IDM_OPTION: // Options
			option.DoModal(hWnd);
			break;

		case IDM_README: // Open Readme.txt
			ShellExecute(nullptr, _T("open"), ReadmeFileName, nullptr, nullptr, SW_SHOWNORMAL);
			break;

		case IDM_HISTORY: // Open History.txt
			ShellExecute(nullptr, _T("open"), HistoryFileName, nullptr, nullptr, SW_SHOWNORMAL);
			break;

		case IDM_STATE: // Open State.txt
			ShellExecute(nullptr, _T("open"), StateFileName, nullptr, nullptr, SW_SHOWNORMAL);
			break;

		case IDM_VERSION: // Version info
			version.DoModal(hWnd);
			break;
		}
		break;

	// Right-click Menu
	case WM_CONTEXTMENU:
		CreateMenu(lp);
		break;

	case WM_MOUSEWHEEL:
		{
			POINT pos;
			GetCursorPos(&pos);
			HWND pWnd = WindowFromPoint(pos);
			if (pWnd == listview.GetHandle())
				SendMessage(pWnd, WM_MOUSEWHEEL, wp, lp);
			break;
		}

	case WM_NOTIFY:
		{
			// Toolbar
			if (wp == ID_TOOLBAR)
			{
				LPNMTOOLBAR pNM = reinterpret_cast<LPNMTOOLBAR>(lp);
				switch (pNM->hdr.code)
				{
					// File History
					case TBN_DROPDOWN:
						MainToolBar.CreateMenuHistory(pNM->iItem);
						break;
				}
			}

			// List view
			if (wp == idsMainList)
			{
				LPNMLISTVIEW pNM = reinterpret_cast<LPNMLISTVIEW>(lp);
				switch (pNM->hdr.code)
				{
				// Sort
				case LVN_COLUMNCLICK:
					listview.Sort(pNM->iSubItem);
					break;
				// Show Tooltips
				case LVN_GETINFOTIP:
					listview.ShowTip(reinterpret_cast<LPNMLVGETINFOTIP>(lp));
					break;
				// View
				case LVN_GETDISPINFO:
					listview.Show(reinterpret_cast<NMLVDISPINFO*>(lp));
					break;
				// D&D (Drag & Drop)
				case LVN_BEGINDRAG:
					//extract.SaveDrop();
					LRESULT             lResult;
					listview.OnBeginDrag(reinterpret_cast<NMHDR*>(lp), &lResult);
					break;
				// Playback / Viewing by double-clicking
				case NM_DBLCLK:
					if (listview.GetCountSel() > 0)
					{
						extract.DecodeTmp();
						extract.OpenRelate();
					}
					break;
				}
			}
			break;
		}

	case WM_SIZE:
		MainToolBar.SetWindowPos(0, 0, 100, 26);
		SearchToolBar.SetWindowPos(100, 0, LOWORD(lp), 26);
		listview.SetWindowPos(0, 28, LOWORD(lp), HIWORD(lp) - 48);
		statusbar.SetWindowPos(LOWORD(lp));
		break;

	case WM_CLOSE:
		// Save
		SaveIni();
		listview.SaveIni();
		// Clean up
		extract.Close();
		DestroyWindow(hWnd);
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	default:
		return (DefWindowProc(hWnd, msg, wp, lp));
	}

	return 0;
}

void CWinMain::CreateMenu(LPARAM lp)
{
	POINT pt;
	pt.x = LOWORD(lp);
	pt.y = HIWORD(lp);

	HMENU rMenu = LoadMenu(m_hInst, _T("RMENU"));
	HMENU rSubMenu = GetSubMenu(rMenu, 0);

	SetQuickMenu(rMenu);

	TrackPopupMenu(rSubMenu, TPM_LEFTALIGN | TPM_TOPALIGN, pt.x, pt.y, 0, m_hWnd, nullptr);
	DestroyMenu(rMenu);
}

void CWinMain::SetQuickMenu(HMENU hMenu)
{
	COption clOption;
	SOption& stOption = clOption.GetOpt();

	struct SQuickSet 
	{
		LPBOOL pbOption;
		int nID;
	} stQuickSet[] = {
		{&stOption.bHighSearchOgg,  IDM_QUICKSET_STD_SEARCHOGG},
		{&stOption.bCreateFolder,   IDM_QUICKSET_EXTRACT_CREATEFOLDER},
		{&stOption.bFixOgg,         IDM_QUICKSET_EXTRACT_FIXOGG},
		{&stOption.bEasyDecrypt,    IDM_QUICKSET_EXTRACT_EASYDECRYPT},
		{&stOption.bDstPNG,         IDM_QUICKSET_EXTRACT_DSTPNG},
		{&stOption.bAlphaBlend,     IDM_QUICKSET_EXTRACT_ALPHABLEND},
		{&stOption.bSusieUse,       IDM_QUICKSET_SUSIE_USE},
		{&stOption.bSusieFirst,     IDM_QUICKSET_SUSIE_FIRST}
	};

	for (const auto& quickSet : stQuickSet)
	{
		UINT uCheck = MF_BYCOMMAND | (*quickSet.pbOption == TRUE) ? MF_CHECKED : MF_UNCHECKED;
		CheckMenuItem(hMenu, quickSet.nID, uCheck);
	}
}

void CWinMain::SetQuickMenuItem(int nID)
{
	COption clOption;
	SOption& stOption = clOption.GetOpt();

	struct SQuickSet
	{
		LPBOOL pbOption;
		int nID;
	} stQuickSet[] = {
		{&stOption.bHighSearchOgg,  IDM_QUICKSET_STD_SEARCHOGG},
		{&stOption.bCreateFolder,   IDM_QUICKSET_EXTRACT_CREATEFOLDER},
		{&stOption.bFixOgg,         IDM_QUICKSET_EXTRACT_FIXOGG},
		{&stOption.bEasyDecrypt,    IDM_QUICKSET_EXTRACT_EASYDECRYPT},
		{&stOption.bDstPNG,         IDM_QUICKSET_EXTRACT_DSTPNG},
		{&stOption.bAlphaBlend,     IDM_QUICKSET_EXTRACT_ALPHABLEND},
		{&stOption.bSusieUse,       IDM_QUICKSET_SUSIE_USE},
		{&stOption.bSusieFirst,     IDM_QUICKSET_SUSIE_FIRST}
	};

	for (auto& quickSet : stQuickSet)
	{
		if (quickSet.nID == nID)
			*quickSet.pbOption ^= 1;
	}

	if (nID == IDM_QUICKSET_EXTRACT_DSTPNG)
	{
		// Extract images as PNG or BMP depending on setting
		stOption.bDstBMP ^= 1;
	}
	else if (nID == IDM_QUICKSET_SUSIE_USE && stOption.bSusieUse == TRUE)
	{
		// Load Susie Plug-ins
		CSusie clSusie;
		clSusie.LoadSpi(stOption.SusieDir);
	}

	clOption.SaveIni();
}
