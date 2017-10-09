#include "StdAfx.h"
#include "YCLibrary.h"

YCLibrary::YCLibrary()
{
}

YCLibrary::~YCLibrary()
{
	Free();
}

/// Load the specified module
///
/// @param file_path Load module name
///
bool YCLibrary::Load(LPCTSTR file_path)
{
	m_module = ::LoadLibrary(file_path);

	return m_module != nullptr;
}

/// Release the loaded module
void YCLibrary::Free()
{
	if (m_module == nullptr)
		return;

	::FreeLibrary(m_module);
	m_module = nullptr;
}

/// Gets the function address
///
/// @param proc_name Name of the function
///
FARPROC YCLibrary::GetProcAddress(LPCTSTR proc_name) const
{
	if (m_module == nullptr)
		return nullptr;

	// Done as a last resort because there is no GetProcAddressW
	const YCStringA string_proc_name = proc_name;

	return ::GetProcAddress(m_module, string_proc_name);
}
