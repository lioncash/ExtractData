#pragma once

class CFont
{
public:
	CFont();
	virtual ~CFont();

	HFONT Create();
	HFONT GetFont() const;

private:
	HFONT m_hFont = nullptr;
};
