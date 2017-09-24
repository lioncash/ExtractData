#include "StdAfx.h"
#include "UI/WinMain.h"

#include "Common.h"
#include "ExtractData.h"
#include "LastDir.h"
#include "res/ResExtractData.h"
#include "Susie.h"
#include "UI/Ctrl/StatusBar.h"
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
#pragma comment(lib, "libpng16d.lib")
#else
#pragma comment(lib, "zlib.lib")
#pragma comment(lib, "libpng16.lib")
#endif

// Main Function
int WINAPI _tWinMain(_In_ HINSTANCE inst, _In_opt_ HINSTANCE prev_inst, _In_ LPTSTR cmd_line, _In_ int cmd_show)
{
#ifdef _DEBUG
	::_CrtSetDbgFlag( _CRTDBG_LEAK_CHECK_DF | _CRTDBG_ALLOC_MEM_DF );
#endif

	CWinMain main;
	return main.WinMain(inst, prev_inst, cmd_line, cmd_show);
}

#define WINDOWCLASS_NAME   _T("ExtractDataClass")
#define WINDOW_NAME        _T("ExtractData")

CWinMain::CWinMain() = default;
CWinMain::~CWinMain() = default;

int CWinMain::WinMain(HINSTANCE inst, HINSTANCE prev_inst, LPTSTR cmd_line, int cmd_show)
{
	// WinMain function receives an argument of the program, which is assigned to a member variable
	m_inst = inst;
	m_prev_inst = prev_inst;
	m_cmd_line = cmd_line;
	m_cmd_show = cmd_show;

	OleInitialize(nullptr);

	if (!InitApp())
		return FALSE;

	if (!InitInstance())
		return FALSE;

	HACCEL accel = LoadAccelerators(inst, _T("MYACCEL"));
	if (!accel)
		return FALSE;

	MSG msg;
	int bRet;
	while ((bRet = GetMessage(&msg, nullptr, 0, 0)) != 0)
	{
		if (bRet == -1)
			break;

		if (!TranslateAccelerator(m_window, accel, &msg))
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
	wc.lpfnWndProc   = WndStaticProc; // Procedure name
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = 0;
	wc.hInstance     = m_inst;  // Instance
	wc.hIcon         = LoadIcon(m_inst, MAKEINTRESOURCE(IDI_ICON_APP));
	wc.hCursor       = LoadCursor(nullptr, IDC_ARROW);
	wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_BTNFACE+1);
	wc.lpszMenuName  = _T("MAINMENU"); // Menu name
	wc.lpszClassName = WINDOWCLASS_NAME;
	wc.hIconSm       = LoadIcon(m_inst, MAKEINTRESOURCE(IDI_ICON_APP));

	m_wc = wc;
	return (RegisterClassEx(&m_wc));
}

// Window creation
bool CWinMain::InitInstance()
{
	m_window = CreateWindow(WINDOWCLASS_NAME,
		WINDOW_NAME,            // This name will be displayed in the title bar
		WS_OVERLAPPEDWINDOW,    // Type of window
		0,                      // X Coordinate
		0,                      // Y Coordinate
		0,                      // Width
		0,                      // Height
		nullptr,                // When creating the parent window handle, the parent is NULL
		nullptr,                // When creating the menu handle, the handle is NULL
		m_inst,                // Instance handle
		static_cast<LPVOID>(this));
	if (!m_window)
		return false;

	Init(idsMainWindow, 640, 480);
	UpdateWindow(m_window);

	return true;
}

// Window Procedure
LRESULT CWinMain::WndProc(HWND window, UINT msg, WPARAM wp, LPARAM lp)
{
	static CMainToolBar main_tool_bar;
	static CSearchToolBar search_tool_bar;
	static CMainListView list_view;
	static CStatusBar status_bar;
	static CExtractData extract;
	static COption option;
	static CVersionInfo version;
	static CLastDir last_dir;
	static TCHAR readme_file_name[MAX_PATH];
	static TCHAR history_file_name[MAX_PATH];
	static TCHAR state_file_name[MAX_PATH];
	INITCOMMONCONTROLSEX ic;

	switch (msg)
	{
	case WM_CREATE:
		{
			// Allow D&D (Drag & Drop)
			DragAcceptFiles(window, TRUE);

			option.Init(search_tool_bar, list_view);

			// Initialization of the common installation configuration
			ic.dwSize = sizeof(INITCOMMONCONTROLSEX);
			ic.dwICC = ICC_WIN95_CLASSES;
			InitCommonControlsEx(&ic);

			// Create main toolbar
			main_tool_bar.Create(window);
			search_tool_bar.Create(window);

			// Create list view
			list_view.Create(window, option.GetOpt());

			// Create status bar
			status_bar.Create(window, option.GetOpt(), list_view);

			// Get full path
			TCHAR ModulePath[MAX_PATH];
			GetModuleFileName(nullptr, ModulePath, MAX_PATH);
			PathRemoveFileSpec(ModulePath);

			// Retrieves full path of the location of Readme.txt
			lstrcpy(readme_file_name, ModulePath);
			PathAppend(readme_file_name, _T("Readme.txt"));

			// Retrieves full path of the location of History.txt
			lstrcpy(history_file_name, ModulePath);
			PathAppend(history_file_name, _T("History.txt"));

			// Retrieves full path of the location of State.txt
			lstrcpy(state_file_name, ModulePath);
			PathAppend(state_file_name, _T("State.txt"));

			extract.Init(window, option.GetOpt(), list_view);
			break;
		}

	case WM_DROPFILES:
		extract.OpenDrop(wp);
		main_tool_bar.AddOpenHistory(extract.GetArcList());
		status_bar.SetCount();
		break;

	case WM_COMMAND:
		switch (LOWORD(wp))
		{
		case IDM_OPEN: // Open a file to load
			extract.Open(last_dir.GetOpen());
			main_tool_bar.AddOpenHistory(extract.GetArcList());
			status_bar.SetCount();
			last_dir.SaveIni();
			break;

		case IDM_CLOSE: // Close the opened file
			extract.Close();
			status_bar.SetCount();
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
			extract.OpenHistory(main_tool_bar.GetHistory()[LOWORD(wp)-ID_TOOLBAR_OPEN_HISTORY]);
			main_tool_bar.AddOpenHistory(extract.GetArcList());
			status_bar.SetCount();
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
			search_tool_bar.SetSearch(LOWORD(wp));
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
			SendMessage(window, WM_CLOSE, 0, 0);
			break;

		case IDM_EXTRACT: // Extract the selected range
			if (list_view.GetCountSel() > 0)
			{
				extract.SaveSel(last_dir.GetSave(), true);
			}
			last_dir.SaveIni();
			break;

		case IDM_EXTRACTALL: // Extract all
			if( list_view.GetCount() > 0 )
			{
				extract.SaveAll(last_dir.GetSave(), true);
			}
			last_dir.SaveIni();
			break;

		case IDM_EXTRACT_NOTCONVERT: // Extracts the selection without converting it
			if (list_view.GetCountSel() > 0)
			{
				extract.SaveSel(last_dir.GetSave(), false);
			}
			last_dir.SaveIni();
			break;

		case IDM_EXTRACTALL_NOTCONVERT: // Extracts all without conversion
			if (list_view.GetCount() > 0)
			{
				extract.SaveAll(last_dir.GetSave(), false);
			}
			last_dir.SaveIni();
			break;

		case IDM_SELECTALL: // Select all
			list_view.SetItemSelAll(LVIS_SELECTED);
			break;

		case IDM_OPTION: // Options
			option.DoModal(window);
			break;

		case IDM_README: // Open Readme.txt
			ShellExecute(nullptr, _T("open"), readme_file_name, nullptr, nullptr, SW_SHOWNORMAL);
			break;

		case IDM_HISTORY: // Open History.txt
			ShellExecute(nullptr, _T("open"), history_file_name, nullptr, nullptr, SW_SHOWNORMAL);
			break;

		case IDM_STATE: // Open State.txt
			ShellExecute(nullptr, _T("open"), state_file_name, nullptr, nullptr, SW_SHOWNORMAL);
			break;

		case IDM_VERSION: // Version info
			version.DoModal(window);
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
			HWND point_window = WindowFromPoint(pos);
			if (point_window == list_view.GetHandle())
				SendMessage(point_window, WM_MOUSEWHEEL, wp, lp);
			break;
		}

	case WM_NOTIFY:
		{
			// Toolbar
			if (wp == ID_TOOLBAR)
			{
				LPNMTOOLBAR nm = reinterpret_cast<LPNMTOOLBAR>(lp);
				switch (nm->hdr.code)
				{
					// File History
					case TBN_DROPDOWN:
						main_tool_bar.CreateMenuHistory(nm->iItem);
						break;
				}
			}

			// List view
			if (wp == idsMainList)
			{
				LPNMLISTVIEW nm = reinterpret_cast<LPNMLISTVIEW>(lp);
				switch (nm->hdr.code)
				{
				// Sort
				case LVN_COLUMNCLICK:
					list_view.Sort(nm->iSubItem);
					break;
				// Show Tooltips
				case LVN_GETINFOTIP:
					list_view.ShowTip(reinterpret_cast<LPNMLVGETINFOTIP>(lp));
					break;
				// View
				case LVN_GETDISPINFO:
					list_view.Show(reinterpret_cast<NMLVDISPINFO*>(lp));
					break;
				// D&D (Drag & Drop)
				case LVN_BEGINDRAG:
				{
					//extract.SaveDrop();
					LRESULT result;
					list_view.OnBeginDrag(reinterpret_cast<NMHDR*>(lp), &result);
					break;
				}
				// Playback / Viewing by double-clicking
				case NM_DBLCLK:
					if (list_view.GetCountSel() > 0)
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
		main_tool_bar.SetWindowPos(0, 0, 100, 26);
		search_tool_bar.SetWindowPos(100, 0, LOWORD(lp), 26);
		list_view.SetWindowPos(0, 28, LOWORD(lp), HIWORD(lp) - 48);
		status_bar.SetWindowPos(LOWORD(lp));
		break;

	case WM_CLOSE:
		// Save
		SaveIni();
		list_view.SaveIni();
		// Clean up
		extract.Close();
		DestroyWindow(window);
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	default:
		return DefWindowProc(window, msg, wp, lp);
	}

	return 0;
}

void CWinMain::CreateMenu(LPARAM lp)
{
	POINT pt;
	pt.x = LOWORD(lp);
	pt.y = HIWORD(lp);

	HMENU menu = LoadMenu(m_inst, _T("RMENU"));
	HMENU sub_menu = GetSubMenu(menu, 0);

	SetQuickMenu(menu);

	TrackPopupMenu(sub_menu, TPM_LEFTALIGN | TPM_TOPALIGN, pt.x, pt.y, 0, m_window, nullptr);
	DestroyMenu(menu);
}

void CWinMain::SetQuickMenu(HMENU menu)
{
	COption option_dialog;
	const SOption& options = option_dialog.GetOpt();

	struct QuickSet
	{
		const BOOL* option;
		int id;
	} quick_set[] = {
		{&options.bHighSearchOgg,  IDM_QUICKSET_STD_SEARCHOGG},
		{&options.bCreateFolder,   IDM_QUICKSET_EXTRACT_CREATEFOLDER},
		{&options.bFixOgg,         IDM_QUICKSET_EXTRACT_FIXOGG},
		{&options.bEasyDecrypt,    IDM_QUICKSET_EXTRACT_EASYDECRYPT},
		{&options.bDstPNG,         IDM_QUICKSET_EXTRACT_DSTPNG},
		{&options.bAlphaBlend,     IDM_QUICKSET_EXTRACT_ALPHABLEND},
		{&options.bSusieUse,       IDM_QUICKSET_SUSIE_USE},
		{&options.bSusieFirst,     IDM_QUICKSET_SUSIE_FIRST}
	};

	for (const auto& entry : quick_set)
	{
		const u32 check = MF_BYCOMMAND | (*entry.option == TRUE ? MF_CHECKED : MF_UNCHECKED);
		CheckMenuItem(menu, entry.id, check);
	}
}

void CWinMain::SetQuickMenuItem(int id)
{
	COption option_dialog;
	SOption& options = option_dialog.GetOpt();

	struct QuickSet
	{
		LPBOOL option;
		int id;
	} quick_set[] = {
		{&options.bHighSearchOgg,  IDM_QUICKSET_STD_SEARCHOGG},
		{&options.bCreateFolder,   IDM_QUICKSET_EXTRACT_CREATEFOLDER},
		{&options.bFixOgg,         IDM_QUICKSET_EXTRACT_FIXOGG},
		{&options.bEasyDecrypt,    IDM_QUICKSET_EXTRACT_EASYDECRYPT},
		{&options.bDstPNG,         IDM_QUICKSET_EXTRACT_DSTPNG},
		{&options.bAlphaBlend,     IDM_QUICKSET_EXTRACT_ALPHABLEND},
		{&options.bSusieUse,       IDM_QUICKSET_SUSIE_USE},
		{&options.bSusieFirst,     IDM_QUICKSET_SUSIE_FIRST}
	};

	for (auto& entry : quick_set)
	{
		if (entry.id == id)
			*entry.option ^= 1;
	}

	if (id == IDM_QUICKSET_EXTRACT_DSTPNG)
	{
		// Extract images as PNG or BMP depending on setting
		options.bDstBMP ^= 1;
	}
	else if (id == IDM_QUICKSET_SUSIE_USE && options.bSusieUse == TRUE)
	{
		// Load Susie Plug-ins
		CSusie susie;
		susie.LoadSpi(options.SusieDir);
	}

	option_dialog.SaveIni();
}
