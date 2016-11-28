#include "StdAfx.h"
#include "UI/DragDrop/DropSource.h"

CDropSource::CDropSource()
{
	m_RefCount = 1;
}

HRESULT WINAPI CDropSource::QueryInterface(const IID& iid, LPVOID* ppv)
{
	if ((iid == IID_IDropSource) || (iid == IID_IUnknown))
	{
		*ppv = (LPVOID)this;
		AddRef();
		return S_OK;
	}

	*ppv = 0;
	return E_NOINTERFACE;
}


ULONG WINAPI CDropSource::AddRef()
{
	InterlockedIncrement(&m_RefCount);
	return (ULONG)m_RefCount;
}


ULONG WINAPI CDropSource::Release()
{
	if (InterlockedDecrement(&m_RefCount) == 0)
		delete this;

	return (ULONG)m_RefCount;
}

HRESULT WINAPI CDropSource::QueryContinueDrag(BOOL bEscapePressed, DWORD grfKeyState)
{
	/* Decide whether or not to contine the drag */

	/* When the mouse button is pressed or if the ESC has been pressed, both will discontinue the drag */
	if (bEscapePressed || (MK_LBUTTON | MK_RBUTTON) == (grfKeyState & (MK_LBUTTON | MK_RBUTTON)))
	{
		return DRAGDROP_S_CANCEL;
	}

	/* When the mouse button is releasedd, then drop */
	if ((grfKeyState & (MK_LBUTTON | MK_RBUTTON)) == 0)
	{
		return DRAGDROP_S_DROP;
	}

	return S_OK;
}

HRESULT WINAPI CDropSource::GiveFeedback(DWORD dwEffect)
{
	/* Changing the mouse cursor to a special display is carried out here */

	// Use standard mouse cursor
	return DRAGDROP_S_USEDEFAULTCURSORS;
}
