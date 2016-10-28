#pragma once

class CFont
{
public:
	CFont();
	virtual ~CFont();

	HFONT Create();
	HFONT GetFont() const { return m_hFont; }

private:
	HFONT m_hFont = nullptr;
};
