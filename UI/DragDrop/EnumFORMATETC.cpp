#include "StdAfx.h"
#include "UI/DragDrop/EnumFORMATETC.h"

CEnumFORMATETC::CEnumFORMATETC()
{
	m_RefCount = 1;
	m_current = 0;
};

HRESULT WINAPI CEnumFORMATETC::QueryInterface(const IID& iid, LPVOID* ppv)
{
	if (iid == IID_IEnumFORMATETC || iid == IID_IUnknown)
	{
		*ppv = (LPVOID)this;
		AddRef();
		return S_OK;
	}

	*ppv = 0;
	return E_NOINTERFACE;
}

ULONG WINAPI CEnumFORMATETC::AddRef()
{
	InterlockedIncrement(&m_RefCount);
	return (ULONG)m_RefCount;
}

ULONG WINAPI CEnumFORMATETC::Release()
{
	if (InterlockedDecrement(&m_RefCount) == 0)
		delete this;

	return (ULONG)m_RefCount;
}

HRESULT WINAPI CEnumFORMATETC::Next(ULONG celt, FORMATETC* rgelt, ULONG* pceltFetched)
{
	if (pceltFetched != NULL)
		*pceltFetched = 0;

	std::vector<FORMATETC>& fmt = m_fmt;
	if ((celt <= 0) || (rgelt == NULL) || (m_current >= fmt.size()))
		return S_FALSE;

	// pceltFetched can only be null when celt is 1
	if ((pceltFetched == NULL) && (celt != 1))
		return S_FALSE;

	ULONG n = celt;
	while ((m_current < fmt.size()) && (n > 0))
	{
		*rgelt++ = fmt[m_current];
		m_current++;
		n--;
	}

	if (pceltFetched != NULL)
		*pceltFetched = celt - n;

	return (n == 0) ? S_OK : S_FALSE;
}

HRESULT WINAPI CEnumFORMATETC::Skip(ULONG celt)
{
	size_t ctFmt = m_fmt.size();

	while ((m_current < ctFmt) && (celt > 0))
	{
		m_current++;
		celt--;
	}

	return (celt == 0) ? S_OK : S_FALSE;
}

HRESULT WINAPI CEnumFORMATETC::Reset()
{
	m_current = 0;
	return S_OK;
}

HRESULT WINAPI CEnumFORMATETC::Clone(IEnumFORMATETC** ppEnum)
{
	if (ppEnum == NULL)
		return E_POINTER;

	CEnumFORMATETC* pfmt = new CEnumFORMATETC;
	if (pfmt == NULL)
		return E_OUTOFMEMORY;

	pfmt->m_fmt = m_fmt;

	pfmt->m_current = m_current;
	*ppEnum = pfmt;

	return S_OK;
}

void CEnumFORMATETC::SetFormat(FORMATETC* fmt)
{
	m_fmt.push_back(*fmt);
}
