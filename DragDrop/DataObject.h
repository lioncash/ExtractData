#pragma once

#include "EnumFORMATETC.h"
#include "STGMEDIUM.h"

class CDataObject : public IDataObject
{
public:
	CDataObject();

	virtual HRESULT WINAPI QueryInterface(const IID& iid, LPVOID* ppv);
	virtual ULONG WINAPI AddRef();
	virtual ULONG WINAPI Release();

	virtual HRESULT WINAPI GetData(FORMATETC* pFormatetc, STGMEDIUM* pMedium);
	virtual HRESULT WINAPI GetDataHere(FORMATETC* pFormatetc, STGMEDIUM* pMedium);
	virtual HRESULT WINAPI QueryGetData(FORMATETC* pFormatetc);
	virtual HRESULT WINAPI GetCanonicalFormatEtc(FORMATETC* pFormatetcIn, FORMATETC* pFormatetcInOut);
	virtual HRESULT WINAPI SetData(FORMATETC* pFormatetc, STGMEDIUM* pMedium, BOOL bRelease);
	virtual HRESULT WINAPI EnumFormatEtc(DWORD dwDirection, IEnumFORMATETC** ppEnumFormatetc);
	virtual HRESULT WINAPI DAdvise(FORMATETC* pFormatetc, DWORD advf, IAdviseSink* pAdvSink, DWORD *pdwConnection);
	virtual HRESULT WINAPI DUnadvise(DWORD dwConnection);
	virtual HRESULT WINAPI EnumDAdvise(IEnumSTATDATA** ppEnumAdvise);

	BOOL allocate(int num);

protected:
	class CObject
	{
	public:
		FORMATETC m_fmt;
		STGMEDIUM m_medium;

	public:
		CObject()
		{
			m_medium.tymed = TYMED_NULL;
		}

		~CObject()
		{
			if (m_medium.tymed != TYMED_NULL)
				ReleaseStgMedium(&m_medium);
		}

		BOOL Set(FORMATETC* pf, STGMEDIUM* pm, BOOL bRelease)
		{
			m_fmt = *pf;

			if (bRelease)
			{
				m_medium = *pm;
				return TRUE;
			}

			return CSTGMEDIUM::Dup(&m_medium, pf, pm);
		}
	};

	LONG m_RefCount;
	std::vector<CObject> m_objects;
};
