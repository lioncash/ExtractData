#pragma once

/// LoadLibrary API Wrapper Class
class YCLibrary
{
public:
	YCLibrary();
	virtual ~YCLibrary();

	bool Load(LPCTSTR file_path);
	void Free();

	FARPROC GetProcAddress(LPCTSTR proc_name) const;

protected:
	HMODULE m_module = nullptr;
};
