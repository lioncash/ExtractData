
#include	"stdafx.h"
#include	"../Common.h"
#include	"../res/ResExtractData.h"
#include	"ListView.h"

SORTPARAM* CListView::m_pSort;

CListView::CListView()
{
	m_hWnd = NULL;
	m_hList = NULL;
	m_hInst = NULL;
	m_hImage = NULL;
	m_pOption = NULL;
}

CListView::~CListView()
{
	Close();
}

void CListView::Init(HWND hWnd, SOption& option)
{
	m_hWnd = hWnd;
	m_hInst = (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE);
	m_pOption = &option;
}

HWND CListView::Create(UINT uID, std::vector<LVCOLUMN>& lvcols, int x, int y, int cx, int cy)
{
	m_uID = uID;

	// Create listview
	HWND hList = CreateWindowEx(WS_EX_CLIENTEDGE,
		WC_LISTVIEW, _T(""),
		WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SHOWSELALWAYS | LVS_OWNERDATA,
		x, y, cx, cy,
		m_hWnd,
		(HMENU)uID,
		m_hInst,
		NULL);
	m_hList = hList;

	// Set style
	DWORD dwStyle = ListView_GetExtendedListViewStyle(hList);
	dwStyle |= LVS_EX_FULLROWSELECT | LVS_EX_HEADERDRAGDROP | LVS_EX_INFOTIP;
	ListView_SetExtendedListViewStyle(hList, dwStyle);

	// Make line spacing 16px
	m_hImage = ImageList_Create(1, 16, ILC_COLOR, 0, 0);
	ListView_SetImageList(hList, m_hImage, LVSIL_STATE);

	YCIni				clIni( SBL_STR_INI_EXTRACTDATA) ;

	clIni.SetSection( uID );

	// Add a column header

	for( size_t i = 0 ; i < lvcols.size() ; i++ )
	{
		TCHAR				szColumn[256];

		_stprintf( szColumn, _T("column%d"), i );

		clIni.SetKey( szColumn );
		clIni.ReadDec( &lvcols[i].cx );

		lvcols[i].iSubItem = i;

		ListView_InsertColumn( hList, i, &lvcols[i] );
	}

	return	m_hList;
}

void CListView::SetBkColor()
{
	//ListView_SetBkColor(m_hList, m_pOption->ListBkColor);
	//ListView_SetTextBkColor(m_hList, m_pOption->ListBkColor);
	ListView_SetBkColor(m_hList, RGB((m_pOption->ListBkColor >> 16) & 0xFF, (m_pOption->ListBkColor >> 8) & 0xFF, m_pOption->ListBkColor & 0xFF));
	ListView_SetTextBkColor(m_hList, RGB((m_pOption->ListBkColor >> 16) & 0xFF, (m_pOption->ListBkColor >> 8) & 0xFF, m_pOption->ListBkColor & 0xFF));
}

void CListView::SetTextColor()
{
	//ListView_SetTextColor(m_hList, m_pOption->ListTextColor);
	ListView_SetTextColor(m_hList, RGB((m_pOption->ListTextColor >> 16) & 0xFF, (m_pOption->ListTextColor >> 8) & 0xFF, m_pOption->ListTextColor & 0xFF));
}

void CListView::Sort(int column)
{
	m_sort.column = column;
	m_sort.direct ^= 1;
	m_pSort = &m_sort;
	OnSort();
	InvalidateRect(m_hList, NULL, FALSE);
}

void CListView::Enable(BOOL flag)
{
	EnableWindow(m_hList, flag);
}

void CListView::SaveIni()
{
	YCIni				clIni( SBL_STR_INI_EXTRACTDATA );

	clIni.SetSection( m_uID );

	// Save column width

	int					nColumns = Header_GetItemCount( ListView_GetHeader( m_hList ) );

	for( int i = 0 ; i < nColumns ; i++ )
	{
		TCHAR				szColumn[256];

		_stprintf( szColumn, _T("Column%d"), i );

		clIni.SetKey( szColumn );
		clIni.WriteDec( ListView_GetColumnWidth( m_hList, i ) );
	}
}

void CListView::Close()
{
	if (m_hImage) {
		ImageList_Destroy(m_hImage);
		m_hImage = NULL;
	}
}

void CListView::Update()
{
	RECT rc;
	GetClientRect(m_hList, &rc);
	InvalidateRect(m_hList, &rc, FALSE);
}
