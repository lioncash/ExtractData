#include "StdAfx.h"
#include "UI/Option.h"

#include "Common.h"
#include "Reg.h"
#include "res/ResExtractData.h"
#include "Susie.h"
#include "UI/Dialog/FolderDialog.h"
#include "UI/Ctrl/Button.h"
#include "UI/Ctrl/CheckBox.h"
#include "UI/Ctrl/EditBox.h"
#include "UI/Ctrl/Label.h"
#include "UI/Ctrl/GroupBox.h"
#include "UI/Ctrl/RadioBtn.h"
#include "UI/Ctrl/UpDown.h"
#include "UI/SusieListView.h"
#include "Utils/ArrayUtils.h"

namespace
{
using ConfigurationDlgProc = int (WINAPI*)(HWND, int);

constexpr std::array<LPCTSTR, 9> search_files_labels{{
	_T("AHX"),
	_T("BMP"),
	_T("JPG"),
	_T("MID"),
	_T("MPG"),
	_T("OGG"),
	_T("PNG"),
	_T("WAV"),
	_T("WMV")
}};
}

SOption COption::m_option;
SOption COption::m_option_tmp;
CMainListView* COption::m_pListView;
CSearchToolBar* COption::m_pToolBar;
HWND COption::m_hDlg = nullptr;
HWND COption::m_hParentWnd = nullptr;

void COption::Init(CSearchToolBar& toolbar, CMainListView& listview)
{
	m_pToolBar = &toolbar;
	m_pListView = &listview;

	LoadIni();

	// Read Susie Plugin
	if (m_option.bSusieUse)
	{
		CSusie susie;
		susie.LoadSpi(m_option.SusieDir);
	}
}

void COption::LoadIni()
{
	YCIni ini(SBL_STR_INI_EXTRACTDATA);

	ini.SetSection(_T("Option"));

	// List background color
	ini.SetKey(_T("ListBkColor"));
	ini.ReadStr(m_option.szListBkColor, sizeof(m_option.szListBkColor), _T("FFFFFF"));
	m_option.ListBkColor = _tcstol(m_option.szListBkColor, nullptr, 16);

	// List text color
	ini.SetKey(_T("ListTextColor"));
	ini.ReadStr(m_option.szListTextColor, sizeof(m_option.szListTextColor), _T("000000"));
	m_option.ListTextColor = _tcstol(m_option.szListTextColor, nullptr, 16);

	// Set whether or not to increase the accuracy of searching for OGG files
	ini.SetKey(_T("HighSearchOgg"));
	ini.ReadDec(&m_option.bHighSearchOgg, TRUE);

	// Set whether or not to extract each folder
	ini.SetKey(_T("ExtFolder"));
	ini.ReadDec(&m_option.bCreateFolder, TRUE);

	// Set whether or not to fix the CRC of each OGG file on extraction
	ini.SetKey(_T("OggCRC"));
	ini.ReadDec(&m_option.bFixOgg, FALSE);

	// Set whether or not to enable simple decoding
	ini.SetKey(_T("EasyDecrypt"));
	ini.ReadDec(&m_option.bEasyDecrypt, FALSE);

	// Set whether or not to change the extension of the script
	ini.SetKey(_T("RenameScriptExt"));
	ini.ReadDec(&m_option.bRenameScriptExt, TRUE);

	// Set the ouput image format
	ini.SetKey(_T("DstBMP"));
	ini.ReadDec(&m_option.bDstBMP, TRUE);
	ini.SetKey(_T("DstPNG"));
	ini.ReadDec(&m_option.bDstPNG, FALSE);

	// Set the PNG compression level
	ini.SetKey(_T("CmplvPng"));
	ini.ReadDec(&m_option.CmplvPng, 1UL);

	// Set whether or not to do alpha blending
	ini.SetKey(_T("AlphaBlend"));
	ini.ReadDec(&m_option.bAlphaBlend, FALSE);

	// Set the background color of the alpha blending
	ini.SetKey(_T("BG_RGB"));
	ini.ReadStr(m_option.szBgRGB, sizeof(m_option.szBgRGB), _T("FFFFFF"));
	m_option.BgRGB = _tcstol(m_option.szBgRGB, nullptr, 16);

	// Destination
	ini.SetKey(_T("SaveMethodSel"));
	ini.ReadDec(&m_option.bSaveSel, TRUE);
	ini.SetKey(_T("SaveMethodSrc"));
	ini.ReadDec(&m_option.bSaveSrc, FALSE);
	ini.SetKey(_T("SaveMethodDir"));
	ini.ReadDec(&m_option.bSaveDir, FALSE);

	// Fixed output folder
	TCHAR desktop_path[_MAX_DIR];
	::SHGetSpecialFolderPath(nullptr, desktop_path, CSIDL_DESKTOPDIRECTORY, FALSE);

	ini.SetKey(_T("SaveDir"));
	ini.ReadStr(m_option.SaveDir, desktop_path);

	// Buffer size
	ini.SetKey(_T("BufSize"));
	ini.ReadDec(&m_option.BufSize, 64UL);

	// Temporary folder
	TCHAR tmp_dir[_MAX_DIR];
	TCHAR tmp_dir_long[_MAX_DIR];

	::GetTempPath(ArrayUtils::ArraySize(tmp_dir), tmp_dir);
	::GetLongPathName(tmp_dir, tmp_dir_long, ArrayUtils::ArraySize(tmp_dir_long));
	PathAppend(tmp_dir_long, _T("ExtractData"));

	ini.SetKey(_T("TmpDir"));
	ini.ReadStr(m_option.TmpDir, tmp_dir_long);

	// Susie Settings
	ini.SetSection(_T("Susie"));

	// Set whether or not to use Susie plugins
	ini.SetKey(_T("SusieUse"));
	ini.ReadDec(&m_option.bSusieUse, FALSE);

	// Susie Folder
	YCString susie_folder_path;
	CReg reg;
	reg.GetValue(susie_folder_path, _T("HKEY_CURRENT_USER\\Software\\Takechin\\Susie\\Plug-in"), _T("Path"));

	ini.SetKey(_T("SusieDir"));
	ini.ReadStr(m_option.SusieDir, susie_folder_path);

	// Set whether or not to give Susie plugins priority on decoding
	ini.SetKey(_T("SusieFirst"));
	ini.ReadDec(&m_option.bSusieFirst, FALSE);

	// Set of files to search through
	ini.SetSection(_T("Search"));

	for (const auto& search_file : search_files_labels)
	{
		BOOL search;

		ini.SetKey(search_file);
		ini.ReadDec(&search, TRUE);

		m_option.bSearch.push_back(search);
	}
}

void COption::SaveIni()
{
	YCIni ini(SBL_STR_INI_EXTRACTDATA);

	ini.SetSection(_T("Option"));

	// List background color
	ini.SetKey(_T("ListBkColor"));
	ini.WriteHex(m_option.ListBkColor, 6);

	// List text color
	ini.SetKey(_T("ListTextColor"));
	ini.WriteHex(m_option.ListTextColor, 6);

	// Increase the accuracy of an OGG search
	ini.SetKey(_T("HighSearchOgg"));
	ini.WriteDec(m_option.bHighSearchOgg);

	// Each folder to extract
	ini.SetKey(_T("ExtFolder"));
	ini.WriteDec(m_option.bCreateFolder);

	// Fix CRC of OGG files on extraction
	ini.SetKey(_T("OggCRC"));
	ini.WriteDec(m_option.bFixOgg);

	// To enable simple decoding
	ini.SetKey(_T("EasyDecrypt"));
	ini.WriteDec(m_option.bEasyDecrypt);

	// Set whether or not to change the extension of scripts
	// ini.SetKey(_T("RenameScriptExt"));
	// ini.WriteDec(m_option.bRenameScriptExt);

	// Output image format
	ini.SetKey(_T("DstBMP"));
	ini.WriteDec(m_option.bDstBMP);
	ini.SetKey(_T("DstPNG"));
	ini.WriteDec(m_option.bDstPNG);

	// PNG Compression level
	ini.SetKey(_T("CmplvPng"));
	ini.WriteDec(m_option.CmplvPng);

	// Alpha blending
	ini.SetKey(_T("AlphaBlend"));
	ini.WriteDec(m_option.bAlphaBlend);

	// Background color alpha blending
	ini.SetKey(_T("BG_RGB"));
	ini.WriteHex(m_option.BgRGB, 6);

	// Destination
	ini.SetKey(_T("SaveMethodSel"));
	ini.WriteDec(m_option.bSaveSel);
	ini.SetKey(_T("SaveMethodSrc"));
	ini.WriteDec(m_option.bSaveSrc);
	ini.SetKey(_T("SaveMethodDir"));
	ini.WriteDec(m_option.bSaveDir);

	// Fixed output folder
	ini.SetKey(_T("SaveDir"));
	ini.WriteStr(m_option.SaveDir);

	// Buffer size
	ini.SetKey(_T("BufSize"));
	ini.WriteDec(m_option.BufSize);

	// Temporary folder
	ini.SetKey(_T("TmpDir"));
	ini.WriteStr(m_option.TmpDir);

	// Susie Settings
	ini.SetSection(_T("Susie"));

	// Use Susie plugins
	ini.SetKey(_T("SusieUse"));
	ini.WriteDec(m_option.bSusieUse);

	// Susie Folder
	ini.SetKey(_T("SusieDir"));
	ini.WriteStr(m_option.SusieDir);

	// Give Susie plugins priority on decoding
	ini.SetKey(_T("SusieFirst"));
	ini.WriteDec(m_option.bSusieFirst);

	// Set of files to search
	ini.SetSection(_T("Search"));

	for (size_t i = 0; i < search_files_labels.size(); i++)
	{
		ini.SetKey(search_files_labels[i]);
		ini.WriteDec(m_option.bSearch[i]);
	}

	// Save Susie plugin settings ON/OFF
	if (m_option.bSusieUse)
	{
		CSusie susie;
		susie.SaveSpi();
	}
}

void COption::DoModal(HWND hWnd)
{
	m_hDlg = CreateProp(hWnd);
}

HWND COption::CreateProp(HWND hWnd)
{
	m_hParentWnd = hWnd;
	HINSTANCE hInst = reinterpret_cast<HINSTANCE>(GetWindowLongPtr(hWnd, GWLP_HINSTANCE));

	PROPSHEETPAGE psp;
	HPROPSHEETPAGE hpsp[3];
	psp.dwSize = sizeof(PROPSHEETPAGE);
	psp.dwFlags = PSP_USETITLE;
	psp.hInstance = hInst;
	psp.lParam = reinterpret_cast<LPARAM>(this);
	psp.pszTemplate = _T("OPTION");

	psp.pszTitle = _T("Basic Settings");
	psp.pfnDlgProc = reinterpret_cast<DLGPROC>(StdProc);
	hpsp[0] = CreatePropertySheetPage(&psp);
	psp.pszTitle = _T("Extraction Settings");
	psp.pfnDlgProc = reinterpret_cast<DLGPROC>(ExtractProc);
	hpsp[1] = CreatePropertySheetPage(&psp);
	psp.pszTitle = _T("Susie Plugins");
	psp.pfnDlgProc = reinterpret_cast<DLGPROC>(SusieProc);
	hpsp[2] = CreatePropertySheetPage(&psp);

	PROPSHEETHEADER psh;
	memset(&psh, 0, sizeof(PROPSHEETHEADER));
	psh.dwSize = sizeof(PROPSHEETHEADER);
	psh.dwFlags = PSH_USECALLBACK;
	psh.hInstance = hInst;
	psh.hwndParent = hWnd;
	psh.nPages = 3;
	psh.phpage = hpsp;
	psh.pszCaption = _T("ExtractData");
	psh.pfnCallback = static_cast<PFNPROPSHEETCALLBACK>(PropSheetProc);

	return reinterpret_cast<HWND>(PropertySheet(&psh));
}

SOption& COption::GetOpt()
{
	return m_option;
}

HWND COption::GetParent() const
{
	return m_hParentWnd;
}

HWND COption::GetHandle() const
{
	return m_hDlg;
}

int COption::PropSheetProc(HWND hWnd, UINT msg, LPARAM lParam)
{
	switch (msg)
	{
		case PSCB_INITIALIZED:
		{
			m_option_tmp = m_option;
			CSusie susie;
			susie.Init();
			const LONG style = GetWindowLongPtr(hWnd, GWL_EXSTYLE);
			SetWindowLongPtr(hWnd, GWL_EXSTYLE, style & ~WS_EX_CONTEXTHELP);
			break;
		}
	}

	return FALSE;
}

LRESULT COption::StdProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp)
{
	static SOption* pOption = &m_option_tmp;

	// Listview settings
	static CGroupBox ListGroup;
	static CLabel ListLabelBk, ListLabelText;
	static CEditBox ListEditBk, ListEditText;
	// Search settings
	static std::array<CCheckBox, search_files_labels.size()> SearchCheck;
	static const size_t SearchCheckNum = search_files_labels.size();
	static CButton SearchBtn[2];
	static CGroupBox SearchGroup;
	// Search accuracy
	static CGroupBox HighSearchGroup;
	static CCheckBox HighSearchCheckOgg;

	switch (msg)
	{
		case WM_INITDIALOG:
		{
			CWindowBase::Init(::GetParent(hWnd));

			UINT ID = 10000;
			const int x = 10;
			const int xx = 15;
			int y = 0;

			// Listview settings
			ListGroup.Create(hWnd, _T("List Settings"), ID++, x, y += 20, 510, 75);
			ListLabelBk.Create(hWnd, _T("Background Color"), ID++, x + xx, y += 24, 100, 20);
			ListEditBk.Create(hWnd, pOption->szListBkColor, ID++, x + xx + 100, y - 4, 70, 22);
			ListEditBk.SetLimit(6);
			ListLabelText.Create(hWnd, _T("Text Color"), ID++, x + xx, y += 24, 100, 20);
			ListEditText.Create(hWnd, pOption->szListTextColor, ID++, x + xx + 100, y - 4, 70, 22);
			ListEditText.SetLimit(6);

			// Search Settings
			SearchGroup.Create(hWnd, _T("Files to be searched"), ID++, x, y += 40, 510, 100);
			//y += 20;
			for (size_t i = 0, xxx = 0; i < SearchCheckNum; i++, xxx += 55)
			{
				if ((i % 8) == 0)
				{
					 xxx = 0, y += 20;
				}

				SearchCheck[i].Create(hWnd, search_files_labels[i], ID++, x + xx + static_cast<int>(xxx), y, 50, 20);
				SearchCheck[i].SetCheck(pOption->bSearch[i]);
			}

			SearchBtn[0].Create(hWnd, _T("Select all"), ID++, 350, y += 30, 80, 20);
			SearchBtn[1].Create(hWnd, _T("Deselect all"),   ID++, 430, y, 80, 20);

			// Search Accuracy Settings
			HighSearchGroup.Create(hWnd, _T("Search Accuracy"), ID++, x, y += 40, 510, 50);
			HighSearchCheckOgg.Create(hWnd, _T("Increase the accuracy of OGG searches"), ID++, x + xx, y += 20, 220, 20);
			HighSearchCheckOgg.SetCheck(pOption->bHighSearchOgg);

			break;
		}

		case WM_COMMAND:
			// Check the file search box
			if (LOWORD(wp) >= SearchCheck[0].GetID() && LOWORD(wp) <= SearchCheck[SearchCheckNum-1].GetID())
			{
				const int number = LOWORD(wp) - SearchCheck[0].GetID();
				pOption->bSearch[number] ^= 1;
				SearchCheck[number].SetCheck(pOption->bSearch[number]);
				PropSheet_Changed(::GetParent(hWnd), hWnd);
				break;
			}

			// Select all
			if (LOWORD(wp) == SearchBtn[0].GetID())
			{
				for (size_t i = 0; i < SearchCheckNum; i++)
				{
					pOption->bSearch[i] = TRUE;
					SearchCheck[i].SetCheck(pOption->bSearch[i]);
				}
				PropSheet_Changed(::GetParent(hWnd), hWnd);
				break;
			}

			// Deselect
			if (LOWORD(wp) == SearchBtn[1].GetID())
			{
				for (size_t i = 0; i < SearchCheckNum; i++)
				{
					pOption->bSearch[i] = FALSE;
					SearchCheck[i].SetCheck(pOption->bSearch[i]);
				}
				PropSheet_Changed(::GetParent(hWnd), hWnd);
				break;
			}

			if (LOWORD(wp) == HighSearchCheckOgg.GetID())
			{
				PropSheet_Changed(::GetParent(hWnd), hWnd);
				break;
			}

			// The contents of the editbox have been changed
			if (HIWORD(wp) == EN_CHANGE)
			{
				PropSheet_Changed(::GetParent(hWnd), hWnd);
				break;
			}
			break;

		case WM_NOTIFY:
		{
			const auto* const hdr = reinterpret_cast<LPNMHDR>(lp);
			switch (hdr->code)
			{
				// Apply/OK, Tabbing
				case PSN_APPLY:
				case PSN_KILLACTIVE:
					ListEditBk.GetText(&pOption->ListBkColor, TRUE);
					_stprintf(pOption->szListBkColor, _T("%06x"), pOption->ListBkColor);
					ListEditText.GetText(&pOption->ListTextColor, TRUE);
					_stprintf(pOption->szListTextColor, _T("%06x"), pOption->ListTextColor);

					pOption->bHighSearchOgg = HighSearchCheckOgg.GetCheck();
					// OK/Apply
					if (hdr->code == PSN_APPLY)
						Apply();
					return TRUE;
			}
			break;
		}
	}

	return FALSE;
}

LRESULT COption::ExtractProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp)
{
	static CFolderDialog FolderDlg;
	static SOption* pOption = &m_option_tmp;

	// Extraction Settings

	static constexpr std::array<LPCTSTR, 3> ExtractCheckText{{
		_T("Extract each folder"),
		_T("Fix the CRC of OGG files upon extraction"),
		_T("Enable simple decoding")
	}};

	static const std::array<BOOL*, 4> ExtractCheckFlag{{
		&pOption->bCreateFolder, &pOption->bFixOgg, &pOption->bEasyDecrypt, &pOption->bRenameScriptExt
	}};

	static std::array<CCheckBox, ExtractCheckText.size()> ExtractCheck;
	static CCheckBox ExtractCheckAlpha;
	static CLabel ExtractLabelImage, ExtractLabelPng, ExtractLabelAlpha, ExtractLabelSave, ExtractLabelBuf, ExtractLabelTmp;
	static CRadioBtn ExtractRadioImage, ExtractRadioSave;
	static CUpDown ExtractUpDownPng;
	static CEditBox ExtractEditSusie, ExtractEditPng, ExtractEditAlpha, ExtractEditSave, ExtractEditBuf, ExtractEditTmp;
	static CButton ExtractBtnSusie, ExtractBtnSave, ExtractBtnTmp;
	static CGroupBox ExtractGroupImage, ExtractGroupSave;

	switch (msg)
	{
		case WM_INITDIALOG:
		{
			UINT ID = 10000;
			const int x = 10;
			const int xx = 15;
			int y = 0;

			// Extraction Settings

			for (size_t i = 0; i < ExtractCheckText.size(); i++)
			{
				ExtractCheck[i].Create(hWnd, ExtractCheckText[i], ID++, x, y += 20, 230, 20);
				ExtractCheck[i].SetCheck(*ExtractCheckFlag[i]);
			}

			//

			int y_image = y;
			ExtractGroupImage.Create(hWnd, _T("Output image format"), ID++, x, y_image += 34, 240, 110);
			ExtractRadioImage.Close();
			ExtractRadioImage.Create(hWnd, _T("BMP"), ID++, x + xx, y_image += 18, 50, 20);
			ExtractRadioImage.Create(hWnd, _T("PNG"), ID++, x + xx, y_image += 20, 50, 20);
			ExtractRadioImage.SetCheck(0, pOption->bDstBMP);
			ExtractRadioImage.SetCheck(1, pOption->bDstPNG);
			ExtractLabelPng.Create(hWnd, _T("Compression Level"), ID++, x + xx + 50, y_image + 3, 100, 20);
			ExtractEditPng.Create(hWnd, _T(""), ID++, x + xx + 150, y_image, 40, 22);
			ExtractEditPng.SetLimit(1);
			ExtractUpDownPng.Create(hWnd, ExtractEditPng.GetCtrlHandle(), pOption->CmplvPng, ID++, 9, 0);

			//

			ExtractCheckAlpha.Create(hWnd, _T("Enable alpha blending"), ID++, x + xx, y_image += 22, 140, 20);
			ExtractCheckAlpha.SetCheck(pOption->bAlphaBlend);
			ExtractLabelAlpha.Create(hWnd, _T("Background color"), ID++, x + xx * 2 + 4, y_image += 24, 100, 20);
			ExtractEditAlpha.Create(hWnd, pOption->szBgRGB, ID++, x + xx * 2 + 100, y_image - 4, 100, 22);
			ExtractEditAlpha.SetLimit(6);
			ExtractEditAlpha.Enable(pOption->bAlphaBlend);

			//

			const int x_save = x + 200;
			int y_save = y;

			ExtractGroupSave.Create(hWnd, _T("Destination"), ID++, x_save + 50, y_save += 34, 290, 110);
			ExtractRadioSave.Close();
			ExtractRadioSave.Create(hWnd, _T("Specify each time"), ID++, x_save + xx + 50, y_save += 18, 220, 20);
			ExtractRadioSave.Create(hWnd, _T("Same folder as input source"), ID++, x_save + xx + 50, y_save += 20, 200, 20);
			ExtractRadioSave.Create(hWnd, _T("The following folder"), ID++, x_save + xx + 50, y_save += 20, 200, 20);
			ExtractRadioSave.SetCheck(0, pOption->bSaveSel);
			ExtractRadioSave.SetCheck(1, pOption->bSaveSrc);
			ExtractRadioSave.SetCheck(2, pOption->bSaveDir);
			ExtractEditSave.Create(hWnd, pOption->SaveDir, ID++, x_save + xx * 2 + 40, y_save += 20, 200, 22);
			ExtractEditSave.Enable(pOption->bSaveDir);
			ExtractBtnSave.Create(hWnd, _T("Browse"), ID++, x_save + xx * 2 + 250, y_save + 1, 50, 20);
			ExtractBtnSave.Enable(pOption->bSaveDir);

			//

			y = (y_image > y_save) ? y_image : y_save;
			ExtractLabelBuf.Create(hWnd, _T("Buffer Size(KB)"), ID++, x, y += 44, 100, 20);
			ExtractEditBuf.Create(hWnd, pOption->BufSize, ID++, x + 100, y - 4, 110, 22);

			//

			ExtractLabelTmp.Create(hWnd, _T("Temporary Folder"), ID++, x, y += 24, 100, 20);
			ExtractEditTmp.Create(hWnd, pOption->TmpDir, ID++, x + 100, y - 4, 200, 22);
			ExtractBtnTmp.Create(hWnd, _T("Browse"), ID++, x + 310, y - 3, 50, 20);

			break;
		}

		case WM_COMMAND:
			// Checkbox
			if (LOWORD(wp) >= ExtractCheck.front().GetID() && LOWORD(wp) <= ExtractCheck.back().GetID())
			{
				PropSheet_Changed(::GetParent(hWnd), hWnd);
				break;
			}

			// Alpha blend check box
			if (LOWORD(wp) == ExtractCheckAlpha.GetID())
			{
				ExtractEditAlpha.Enable(ExtractCheckAlpha.GetCheck());
				PropSheet_Changed(::GetParent(hWnd), hWnd);
				break;
			}

			//Output image format radio button
			if (LOWORD(wp) >= ExtractRadioImage.GetID(0) && LOWORD(wp) <= ExtractRadioImage.GetID(1))
			{
				PropSheet_Changed(::GetParent(hWnd), hWnd);
				break;
			}

			// Destination radio button
			if (LOWORD(wp) >= ExtractRadioSave.GetID(0) && LOWORD(wp) <= ExtractRadioSave.GetID(2))
			{
				ExtractEditSave.Enable(ExtractRadioSave.GetCheck(2));
				ExtractBtnSave.Enable(ExtractRadioSave.GetCheck(2));
				PropSheet_Changed(::GetParent(hWnd), hWnd);
				break;
			}

			// Output folder browse
			if (LOWORD(wp) == ExtractBtnSave.GetID())
			{
				TCHAR szSaveDir[_MAX_DIR];
				ExtractEditSave.GetText(szSaveDir, sizeof(szSaveDir));
				if (FolderDlg.DoModal(hWnd, _T("Select the output folder"), szSaveDir) == TRUE)
					ExtractEditSave.SetText(szSaveDir);
				break;
			}

			// Temporary folder browse
			if (LOWORD(wp) == ExtractBtnTmp.GetID())
			{
				TCHAR szTmpDir[_MAX_DIR];
				ExtractEditTmp.GetText(szTmpDir, sizeof(szTmpDir));
				if (FolderDlg.DoModal(hWnd, _T("Select a temporary folder"), szTmpDir) == TRUE)
					ExtractEditTmp.SetText(szTmpDir);
				break;
			}

			// Contents of the edit box have been changed
			if (HIWORD(wp) == EN_CHANGE)
			{
				PropSheet_Changed(::GetParent(hWnd), hWnd);
				break;
			}
			break;

		case WM_NOTIFY:
		{
			const auto* const hdr = reinterpret_cast<LPNMHDR>(lp);
			switch (hdr->code)
			{
				// OK/Apply, Tabbing
				case PSN_APPLY:
				case PSN_KILLACTIVE:
					// Extraction Settings
					for (size_t i = 0; i < ExtractCheck.size(); i++)
						*ExtractCheckFlag[i] = ExtractCheck[i].GetCheck();
					//
					pOption->bDstBMP = ExtractRadioImage.GetCheck(0);
					pOption->bDstPNG = ExtractRadioImage.GetCheck(1);
					ExtractEditPng.GetText(&pOption->CmplvPng, FALSE);
					//
					pOption->bAlphaBlend = ExtractCheckAlpha.GetCheck();
					ExtractEditAlpha.GetText(&pOption->BgRGB, TRUE);
					_stprintf(pOption->szBgRGB, _T("%06x"), pOption->BgRGB);
					//
					pOption->bSaveSel = ExtractRadioSave.GetCheck(0);
					pOption->bSaveSrc = ExtractRadioSave.GetCheck(1);
					pOption->bSaveDir = ExtractRadioSave.GetCheck(2);
					ExtractEditSave.GetText(pOption->SaveDir);
					//
					ExtractEditBuf.GetText(&pOption->BufSize, FALSE);
					//
					ExtractEditTmp.GetText(pOption->TmpDir);
					// OK/Apply
					if (hdr->code == PSN_APPLY)
						Apply();
					return TRUE;
			}
			break;
		}
	}

	return FALSE;
}

LRESULT COption::SusieProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp)
{
	static CFolderDialog FolderDlg;
	static SOption* pOption = &m_option_tmp;

	static CCheckBox SusieCheckUse, SusieCheckFirst;
	static CLabel SusieLabelDir;
	static CEditBox SusieEditDir;
	static CButton SusieBtnDir, SusieBtnUpdate, SusieBtn[2];
	static CSusieListView SusieListView;
	static CSusie susie;

	switch (msg)
	{
		case WM_INITDIALOG:
		{
			UINT ID = 10000;
			const int x = 10;
			const int xx = 15;
			int y = 0;

			SusieCheckUse.Create(hWnd, _T("Use Susie Plugins"), ID++, x + 15, y += 20, 200, 20);
			SusieCheckUse.SetCheck(pOption->bSusieUse);

			SusieLabelDir.Create(hWnd, _T("Susie Folder"), ID++, x + xx, y += 24, 75, 20);
			SusieEditDir.Create(hWnd, pOption->SusieDir, ID++, x + xx + 75, y - 4, 200, 22);
			SusieEditDir.Enable(pOption->bSusieUse);
			SusieBtnDir.Create(hWnd, _T("Browse"), ID++, x + xx + 280, y - 3, 50, 20);
			SusieBtnDir.Enable(pOption->bSusieUse);

			SusieCheckFirst.Create(hWnd, _T("Give Susie plugins priority when decoding"), ID++, x + xx, y += 20, 250, 20);
			SusieCheckFirst.SetCheck(pOption->bSusieFirst);
			SusieCheckFirst.Enable(pOption->bSusieUse);

			SusieListView.Create(hWnd, *pOption, x + xx, y += 30, 500, 190);
			SusieListView.Close();
			SusieListView.Enable(pOption->bSusieUse);
			SusieListView.Show();

			SusieBtnUpdate.Create(hWnd, _T("Update"), ID++, x + 290, y += 200, 50, 20);
			SusieBtnUpdate.Enable(pOption->bSusieUse);
			SusieBtn[0].Create(hWnd, _T("All ON"), ID++, x + 350, y, 80, 20);
			SusieBtn[0].Enable(pOption->bSusieUse);
			SusieBtn[1].Create(hWnd, _T("All OFF"), ID++, x + 430, y, 80, 20);
			SusieBtn[1].Enable(pOption->bSusieUse);

			break;
		}

		case WM_COMMAND:
			// Use Susie plugins
			if (LOWORD(wp) == SusieCheckUse.GetID())
			{
				const BOOL flag = SusieCheckUse.GetCheck();
				SusieEditDir.Enable(flag);
				SusieBtnDir.Enable(flag);
				SusieCheckFirst.Enable(flag);
				SusieListView.Enable(flag);
				SusieBtnUpdate.Enable(flag);
				SusieBtn[0].Enable(flag);
				SusieBtn[1].Enable(flag);
				PropSheet_Changed(::GetParent(hWnd), hWnd);
				// Click here to show / hide the list of checkboxes, de-selected state
				pOption->bSusieUse = flag;
				SusieListView.SetItemSelAll(0);
				break;
			}

			// Susie Folder Browse
			if (LOWORD(wp) == SusieBtnDir.GetID())
			{
				TCHAR szSusieDir[_MAX_DIR];
				SusieEditDir.GetText(szSusieDir, sizeof(szSusieDir));
				if (FolderDlg.DoModal(hWnd, _T("Select the Susie folder"), szSusieDir) == TRUE)
					SusieEditDir.SetText(szSusieDir);
			}

			// Give Susie plugins priority on decoding
			if (LOWORD(wp) == SusieCheckFirst.GetID())
			{
				PropSheet_Changed(::GetParent(hWnd), hWnd);
				break;
			}

			// Update
			if (LOWORD(wp) == SusieBtnUpdate.GetID())
			{
				susie.LoadSpi(pOption->SusieDir);
				susie.Init();
				SusieListView.Show();
				SusieListView.Update();
				break;
			}

			// All ON
			if (LOWORD(wp) == SusieBtn[0].GetID())
			{
				SusieListView.SetCheckAll(true);
				PropSheet_Changed(::GetParent(hWnd), hWnd);
				break;
			}

			// All OFF
			if (LOWORD(wp) == SusieBtn[1].GetID())
			{
				SusieListView.SetCheckAll(false);
				PropSheet_Changed(::GetParent(hWnd), hWnd);
				break;
			}

			// Settings
			if (LOWORD(wp) == IDM_SUSIE_SET)
			{
				const SSusieInfo* const susie_info = SusieListView.GetFocusSusieInfo();

				// Get ConfigurationDlg()
				const auto ConfigurationDlg = reinterpret_cast<ConfigurationDlgProc>(susie_info->plugin.GetProcAddress(_T("ConfigurationDlg")));

				if (ConfigurationDlg == nullptr)
					break;

				// Call settings
				ConfigurationDlg(hWnd, 1);

				break;
			}

			// Contents of the editbox have changed
			if (HIWORD(wp) == EN_CHANGE)
			{
				PropSheet_Changed(::GetParent(hWnd), hWnd);
				break;
			}
			break;

		case WM_NOTIFY:
		{
			const auto* const hdr = reinterpret_cast<LPNMHDR>(lp);

			switch (hdr->code)
			{
				// OK/Apply, Tabbing
				case PSN_APPLY:
				case PSN_KILLACTIVE:
					pOption->bSusieUse = SusieCheckUse.GetCheck();
					pOption->bSusieFirst = SusieCheckFirst.GetCheck();
					SusieEditDir.GetText(pOption->SusieDir);
					SusieListView.SaveIni();
					// OK/Apply
					if (hdr->code == PSN_APPLY)
					{
						const BOOL update = (m_option.SusieDir == pOption->SusieDir) ? FALSE : TRUE;
						Apply();
						// Re-acquire plugin folder only when Susie has been changed
						if (pOption->bSusieUse == TRUE && update == TRUE)
						{
							susie.LoadSpi(pOption->SusieDir);
							susie.Init();
							SusieListView.Show();
							SusieListView.Update();
						}
					}
					return TRUE;
				// Check processing
				case NM_CLICK:
				case NM_DBLCLK:
					if (hdr->idFrom == idsSusieList)
					{
						if (SusieListView.SetCheck())
							PropSheet_Changed(::GetParent(hWnd), hWnd);
						break;
					}
			}

			// List view
			if (wp == idsSusieList)
			{
				const auto* const plv = reinterpret_cast<LPNMLISTVIEW>(lp);
				switch (plv->hdr.code)
				{
					// Custom draw
					case NM_CUSTOMDRAW:
						return SusieListView.CustomDraw(reinterpret_cast<LPNMLVCUSTOMDRAW>(lp));
					// Show tool tip
					case LVN_GETINFOTIP:
						SusieListView.ShowTip(reinterpret_cast<LPNMLVGETINFOTIP>(lp));
						break;
					// View
					case LVN_GETDISPINFO:
						SusieListView.Show(reinterpret_cast<NMLVDISPINFO*>(lp));
						break;
				}
			}

			break;
		}

		case WM_MOUSEWHEEL:
		{
			POINT pos;
			GetCursorPos(&pos);
			HWND pWnd = WindowFromPoint(pos);
			if (pWnd == SusieListView.GetHandle())
				SendMessage(pWnd, WM_MOUSEWHEEL, wp, lp);
			break;
		}

		// Right-click menu (Context menu)
		case WM_CONTEXTMENU:
		{
			if (wp == reinterpret_cast<WPARAM>(SusieListView.GetHandle()))
				SusieListView.CreateMenu(lp);
			break;
		}
	}

	return FALSE;
}

void COption::Apply()
{
	// Reflect changes
	CSusie susie;
	susie.Apply();

	// Save ini
	m_option = m_option_tmp;
	SaveIni();

	m_pListView->SetBkColor();
	m_pListView->SetTextColor();
	m_pListView->Update();

	m_pToolBar->SetCheckSearch();
}
