
#include	"stdafx.h"
#include	"res/ResExtractData.h"
#include	"Common.h"
#include	"Option.h"
#include	"MainToolBar.h"

///////////////////////////////////////////////////////////////////////////
//
// Main toolbar
//
///////////////////////////////////////////////////////////////////////////
HWND CMainToolBar::Create(HWND hWnd)
{
    TBBUTTON tbButton[] = {
        {0, IDM_OPEN, TBSTATE_ENABLED, TBSTYLE_DROPDOWN, 0, 0},
        {1, IDM_EXTRACT, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0},
        {2, IDM_EXTRACTALL, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0}
    };

    HWND hToolBar = CToolBar::Create(hWnd, tbButton, IDI_MAIN_TOOLBAR, 16, ARRAYSIZE(tbButton));

    TBBUTTON tbSpace = {0, 0, TBSTATE_ENABLED, TBSTYLE_SEP, 0, 0};
    SendMessage(hToolBar, TB_INSERTBUTTON, 0, (LPARAM)&tbSpace);
    SendMessage(hToolBar, TB_INSERTBUTTON, 4, (LPARAM)&tbSpace);

    return (hToolBar);
}

void CMainToolBar::CreateMenuHistory(int iItem)
{
    RECT rc;
    SendMessage(GetCtrlHandle(), TB_GETRECT, (WPARAM)iItem, (LPARAM)&rc);
    MapWindowPoints(GetCtrlHandle(), HWND_DESKTOP, (LPPOINT)&rc, 2);

    TPMPARAMS tpm;
    tpm.cbSize = sizeof(TPMPARAMS);
    tpm.rcExclude.top = rc.top;
    tpm.rcExclude.left = rc.left;
    tpm.rcExclude.bottom = rc.bottom;
    tpm.rcExclude.right = rc.right;

    HMENU hOpenMenu = CreateMenu();
    HMENU hOpenSubMenu = CreatePopupMenu();

    MENUITEMINFO mii;
    mii.cbSize = sizeof(MENUITEMINFO);
    mii.fMask = MIIM_TYPE | MIIM_SUBMENU;
    mii.fType = MFT_STRING;
    mii.hSubMenu = hOpenSubMenu;
    mii.dwTypeData = _T("dummy");
    InsertMenuItem(hOpenMenu, 0, TRUE, &mii);

    // When there is no history
    if (m_vcOpenHistoryList.empty())
    {
        mii.fMask = MIIM_TYPE | MIIM_STATE;
        mii.dwTypeData = _T("No History");
        mii.fState = MFS_DISABLED;
        InsertMenuItem(hOpenSubMenu, 0, TRUE, &mii);
    }
    // When there is history
    else
    {
        mii.fMask = MIIM_TYPE | MIIM_ID;

        for (int i = 0; i < (int)m_vcOpenHistoryList.size(); i++)
        {
            mii.wID = ID_TOOLBAR_OPEN_HISTORY + i;
            mii.dwTypeData = m_vcOpenHistoryList[i].GetBuffer( 0 );
            InsertMenuItem(hOpenSubMenu, i, TRUE, &mii);
        }
    }

    TrackPopupMenuEx(hOpenSubMenu, TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_VERTICAL, rc.left, rc.bottom, GetHandle(), &tpm);
    DestroyMenu(hOpenMenu);
}

void CMainToolBar::AddOpenHistory(std::vector<CArcFile*>& pclArcList)
{
    //for (int i = 0; i < (int)pclArcList.size(); i++) {
    for (std::vector<CArcFile*>::iterator itrArc = pclArcList.begin(); itrArc != pclArcList.end(); )
    {
        CArcFile* pclArc = *itrArc;
        // Turn off if there is any history
        for (std::vector<YCString>::iterator itrStr = m_vcOpenHistoryList.begin(); itrStr != m_vcOpenHistoryList.end(); itrStr++)
        {
            if (pclArc->GetArcPath() == *itrStr)
            {
                m_vcOpenHistoryList.erase(itrStr);
                break;
            }
        }
        // When history entries have hit their limit, remove the oldest entry to make room for the newer one
        if (m_vcOpenHistoryList.size() == 10)
            m_vcOpenHistoryList.pop_back();
        // Add to the history
        m_vcOpenHistoryList.insert(m_vcOpenHistoryList.begin(), pclArc->GetArcPath());

        // Close a file that isn't supported
        if (pclArc->GetState() == FALSE)
        {
            itrArc = pclArcList.erase(itrArc);
            delete pclArc;
        }
        else
        {
            itrArc++;
        }
    }

    // Save the history ini
    SaveIni();
}

void CMainToolBar::LoadIni()
{
    YCIni				clIni( SBL_STR_INI_EXTRACTDATA );

    clIni.SetSection( _T("OpenHistory") );

    for( unsigned int i = 0 ; i < 10 ; i++ )
    {
        TCHAR				szKey[256];

        _stprintf( szKey, _T("File%u"), i );

        clIni.SetKey( szKey );

        YCString			clsPathToArc;

        clIni.ReadStr( clsPathToArc, _T("") );

        if( clsPathToArc == _T("") )
        {
            break;
        }

        m_vcOpenHistoryList.push_back( clsPathToArc );
    }
}

void CMainToolBar::SaveIni()
{
    YCIni				clIni( SBL_STR_INI_EXTRACTDATA );

    clIni.SetSection( _T("OpenHistory") );

    for( size_t i = 0; i < m_vcOpenHistoryList.size() ; i++ )
    {
        TCHAR				szKey[256];

        _stprintf( szKey, _T("File%u"), i );

        clIni.SetKey( szKey );
        clIni.WriteStr( m_vcOpenHistoryList[i] );
    }
}

///////////////////////////////////////////////////////////////////////////
//
// Search file toolbar
//
///////////////////////////////////////////////////////////////////////////
HWND CSearchToolBar::Create(HWND hWnd)
{
    TBBUTTON tbButton[] = {
        {0, IDM_AHX, TBSTATE_ENABLED, TBSTYLE_CHECK, 0, 0},
        {1, IDM_BMP, TBSTATE_ENABLED, TBSTYLE_CHECK, 0, 0},
        {2, IDM_JPG, TBSTATE_ENABLED, TBSTYLE_CHECK, 0, 0},
        {3, IDM_MID, TBSTATE_ENABLED, TBSTYLE_CHECK, 0, 0},
        {4, IDM_MPG, TBSTATE_ENABLED, TBSTYLE_CHECK, 0, 0},
        {5, IDM_OGG, TBSTATE_ENABLED, TBSTYLE_CHECK, 0, 0},
        {6, IDM_PNG, TBSTATE_ENABLED, TBSTYLE_CHECK, 0, 0},
        {7, IDM_WAV, TBSTATE_ENABLED, TBSTYLE_CHECK, 0, 0},
        {8, IDM_WMV, TBSTATE_ENABLED, TBSTYLE_CHECK, 0, 0}
    };

    m_nBeginID = IDM_AHX;

    HWND hToolBar = CToolBar::Create(hWnd, tbButton, IDI_SEARCH_TOOLBAR, 25, ARRAYSIZE(tbButton));
    SetCheckSearch();

    return (hToolBar);
}

void CSearchToolBar::SetCheckSearch()
{
    HWND hToolBar = GetCtrlHandle();
    COption option;
    for (int i = 0, ID = m_nBeginID; i < (int)option.GetOpt().bSearch.size(); i++, ID++)
        SendMessage(hToolBar, TB_CHECKBUTTON, ID, option.GetOpt().bSearch[i]);
}

void CSearchToolBar::SetSearch(int nID)
{
    int nNumber = nID - m_nBeginID;

    // Change
    COption option;
    option.GetOpt().bSearch[nNumber] ^= 1;

    // Save ini
    option.SaveIni();
}
