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

BOOL YCLibrary::Load(
	LPCTSTR				pszPathToFile					// Load module name
	)
{
	m_hModule = ::LoadLibrary( pszPathToFile );

	return	(m_hModule != NULL);
}

//////////////////////////////////////////////////////////////////////////////////////////
//	Release the loaded module

void	YCLibrary::Free()
{
	if( m_hModule != NULL )
	{
		::FreeLibrary( m_hModule );
		m_hModule = NULL;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////
//	Gets the function address

FARPROC	YCLibrary::GetProcAddress(
	LPCTSTR				pszProcName						// Name of the function
	)
{
	if( m_hModule == NULL )
	{
		return NULL;
	}

	YCStringA clsProcName = pszProcName;	// Done as a last resort because there is no GetProcAddressW

	return ::GetProcAddress( m_hModule, clsProcName );
}
