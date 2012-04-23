#pragma once

class CDropSource : public IDropSource {
private:
    LONG m_RefCount;

public:
    CDropSource();

    virtual HRESULT WINAPI QueryInterface(const IID& iid, LPVOID* ppv);
    virtual ULONG WINAPI AddRef();
    virtual ULONG WINAPI Release();

    virtual HRESULT WINAPI QueryContinueDrag(BOOL fEscapePressed, DWORD grfKeyState);
    virtual HRESULT WINAPI GiveFeedback(DWORD dwEffect);
};
