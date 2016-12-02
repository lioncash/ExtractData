#pragma once

//----------------------------------------------------------------------------------------
//-- Local Memory Class ------------------------------------------------------------------
//----------------------------------------------------------------------------------------

class YCLocalMemory
{
public:
	YCLocalMemory();
	virtual ~YCLocalMemory();

	bool Alloc(UINT uFlags, UINT uBytes);
	bool Free();

	void* Lock();
	bool Unlock();

	UINT GetSize() const;

	HLOCAL& GetHandle();
	void* GetPtr() const;

protected:
	HLOCAL m_hMemory = nullptr;
	void* m_pvMemory = nullptr;
};
