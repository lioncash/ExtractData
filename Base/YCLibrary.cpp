#include "stdafx.h"
#include "YCLibrary.h"

//////////////////////////////////////////////////////////////////////////////////////////
// Constructor

YCLibrary::YCLibrary()
{
	m_hModule = NULL;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Destructor

YCLibrary::~YCLibrary()
{
	Free();
}

//////////////////////////////////////////////////////////////////////////////////////////
// Load the specified module
//
// Parameters:
//   - pszPathToFile - Load module name

bool YCLibrary::Load(LPCTSTR pszPathToFile)
{
	m_hModule = ::LoadLibrary( pszPathToFile );

	return (m_hModule != NULL);
}

//////////////////////////////////////////////////////////////////////////////////////////
// Release the loaded module

void YCLibrary::Free()
{
	if (m_hModule != NULL)
	{
		::FreeLibrary(m_hModule);
		m_hModule = NULL;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////
// Gets the function address
//
// Parameters:
//   - pszProcName - Name of the function

FARPROC YCLibrary::GetProcAddress(LPCTSTR pszProcName)
{
	if (m_hModule == NULL)
	{
		return NULL;
	}

	YCStringA clsProcName = pszProcName; // Done as a last resort because there is no GetProcAddressW

	return ::GetProcAddress(m_hModule, clsProcName);
}
