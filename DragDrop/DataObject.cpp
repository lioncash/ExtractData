#include "stdafx.h"
#include "../stdafx.h"
#include "DataObject.h"

CDataObject::CDataObject()
{
	m_RefCount = 1;
}

HRESULT WINAPI CDataObject::QueryInterface(const IID& iid, LPVOID* ppv)
{
	if ((iid == IID_IDataObject) || (iid == IID_IUnknown))
	{
		*ppv = (LPVOID)this;
		AddRef();
		return S_OK;
	}

	*ppv = 0;
	return E_NOINTERFACE;
}

ULONG WINAPI CDataObject::AddRef()
{
	InterlockedIncrement(&m_RefCount);
	return (ULONG)m_RefCount;
}

ULONG WINAPI CDataObject::Release()
{
	if (InterlockedDecrement(&m_RefCount) == 0)
		delete this;

	return (ULONG)m_RefCount;
}

HRESULT WINAPI CDataObject::GetData(FORMATETC* pFormatetc, STGMEDIUM* pMedium)
{
	if ((pFormatetc == NULL) || (pMedium == NULL))
		return E_INVALIDARG;

	if (!(DVASPECT_CONTENT & pFormatetc->dwAspect))
		return DV_E_DVASPECT;

	std::vector<CObject>& objects = m_objects;
	for (int i = 0; i < (int)objects.size(); i++)
	{
		if ((objects[i].m_fmt.cfFormat == pFormatetc->cfFormat) && ((objects[i].m_fmt.tymed & pFormatetc->tymed) != 0))
		{
			if (CSTGMEDIUM::Dup(pMedium, &objects[i].m_fmt, &objects[i].m_medium) == FALSE)
				return E_OUTOFMEMORY;
				
			return S_OK;
		}
	}

	return DV_E_FORMATETC;
}

HRESULT WINAPI CDataObject::GetDataHere(FORMATETC* pFormatetc, STGMEDIUM* pMedium)
{
	return E_NOTIMPL;
}

HRESULT WINAPI CDataObject::QueryGetData(FORMATETC* pFormatetc)
{
	if (pFormatetc == NULL)
		return E_INVALIDARG;

	if (!(DVASPECT_CONTENT & pFormatetc->dwAspect))
		return DV_E_DVASPECT;

	std::vector<CObject>& objects = m_objects;
	for (size_t i = 0; i < objects.size(); i++)
	{
		if ((objects[i].m_fmt.cfFormat == pFormatetc->cfFormat) && ((objects[i].m_fmt.tymed & pFormatetc->tymed) != 0))
			return S_OK;
	}

	return DV_E_FORMATETC;
}

HRESULT WINAPI CDataObject::EnumFormatEtc(DWORD dwDirection, IEnumFORMATETC** ppEnumFormatetc)
{
	if (ppEnumFormatetc == NULL)
		return E_INVALIDARG;

	*ppEnumFormatetc = NULL;
	switch (dwDirection)
	{
		case DATADIR_GET:
		{
			CEnumFORMATETC* pfmt = new CEnumFORMATETC();
			if (pfmt == NULL)
				return E_OUTOFMEMORY;

			for (size_t i = 0; i < m_objects.size(); i++)
				pfmt->SetFormat(&m_objects[i].m_fmt);

			*ppEnumFormatetc = pfmt;
			break;
		}

		default:
			return E_NOTIMPL;
	}

	return S_OK;
}

HRESULT WINAPI CDataObject::SetData(FORMATETC* pFormatetc, STGMEDIUM* pMedium, BOOL bRelease)
{
	if (pFormatetc == NULL || pMedium == NULL)
		return E_INVALIDARG;

	CObject object;
	if (object.Set(pFormatetc, pMedium, bRelease) == FALSE)
		return E_OUTOFMEMORY;
	m_objects.push_back(object);

	return S_OK;
}

HRESULT WINAPI CDataObject::GetCanonicalFormatEtc(FORMATETC* pFormatetcIn, FORMATETC* pFormatetcOut)
{
	return E_NOTIMPL;
}

HRESULT WINAPI CDataObject::DAdvise(FORMATETC* pFormatetc, DWORD advf, IAdviseSink* pAdvSink, DWORD* pdwConnection)
{
	return OLE_E_ADVISENOTSUPPORTED;
}

HRESULT WINAPI CDataObject::DUnadvise(DWORD dwConnection)
{
	return OLE_E_ADVISENOTSUPPORTED;
}

HRESULT WINAPI CDataObject::EnumDAdvise(IEnumSTATDATA** ppEnumAdvise)
{
	return OLE_E_ADVISENOTSUPPORTED;
}
