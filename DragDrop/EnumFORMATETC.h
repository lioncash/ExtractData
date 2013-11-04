#pragma once

class CEnumFORMATETC : public IEnumFORMATETC
{
friend class CEnumFORMATETC;

public:
	CEnumFORMATETC();

	virtual HRESULT WINAPI QueryInterface(const IID& iid, LPVOID* ppv);
	virtual ULONG WINAPI AddRef();
	virtual ULONG WINAPI Release();

	virtual HRESULT WINAPI Next(ULONG celt, FORMATETC* rgelt, ULONG* pceltFetched);
	virtual HRESULT WINAPI Skip(ULONG celt);
	virtual HRESULT WINAPI Reset();
	virtual HRESULT WINAPI Clone(IEnumFORMATETC** ppEnum);

	void SetFormat(FORMATETC* fmt);

private:
	LONG m_RefCount;

protected:
	std::vector<FORMATETC> m_fmt;
	int m_current;
};
