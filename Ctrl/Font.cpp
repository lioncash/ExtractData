#include "stdafx.h"
#include "Font.h"

CFont::CFont()
{
	m_hFont = NULL;
}

CFont::~CFont()
{
	if (m_hFont) DeleteObject(m_hFont);
}

// Function to set the font
HFONT CFont::Create()
{
	if (m_hFont == NULL)
	{
		m_hFont = CreateFont(
			13, // 0 if default font height (in logical units)
			0, // Width, 0 if height is 0
			0, // 10 times the angle of the display screen and the X-Axis
			0, // Angle of each character
			0, // Thickness
			0, // Italics
			0, // Underline
			0, // Strikethrough
			DEFAULT_CHARSET, // Font Settings
			0, // Approximation accuracy of the specified character and print character
			0, // Accuracy when the character is outside the clipping region
			0, // Output quality
			0, // Character spacing
			// _T("MS UI Gothic") // The name of the typeface -- NOTE: This is the original typeface before I began translating the app 
			_T("Segoe UI") // The name of the typeface  -- NOTE: Changed to Segoe UI to make it better for English locale
			// TODO: Possibly make the font able to be set from within the application itself
		);
	}

	return (m_hFont);
}