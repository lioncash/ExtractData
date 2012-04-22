#pragma once

class CFont {
private:
	HFONT m_hFont;

public:
	CFont();
	virtual ~CFont();

	HFONT Create();
	HFONT GetFont() { return (m_hFont); }
};
