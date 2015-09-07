#pragma once

class CFont
{
public:
	CFont();
	virtual ~CFont();

	HFONT Create();
	HFONT GetFont() { return m_hFont; }

private:
	HFONT m_hFont;
};
