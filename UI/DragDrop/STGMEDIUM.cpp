#include "StdAfx.h"
#include "UI/DragDrop/STGMEDIUM.h"

BOOL CSTGMEDIUM::Dup(STGMEDIUM* pdest, const FORMATETC* pFormatetc, const STGMEDIUM* pMedium)
{
	HANDLE hVoid = NULL;

	switch (pMedium->tymed)
	{
		case TYMED_HGLOBAL:
			hVoid = OleDuplicateData(pMedium->hGlobal, pFormatetc->cfFormat, (UINT)NULL);
			pdest->hGlobal = (HGLOBAL)hVoid;
			break;

		case TYMED_GDI:
			hVoid = OleDuplicateData(pMedium->hBitmap, pFormatetc->cfFormat, (UINT)NULL);
			pdest->hBitmap = (HBITMAP)hVoid;
			break;

		case TYMED_MFPICT:
			hVoid = OleDuplicateData(pMedium->hMetaFilePict, pFormatetc->cfFormat, (UINT)NULL);
			pdest->hMetaFilePict = (HMETAFILEPICT)hVoid;
			break;

		case TYMED_ENHMF:
			hVoid = OleDuplicateData(pMedium->hEnhMetaFile, pFormatetc->cfFormat, (UINT)NULL);
			pdest->hEnhMetaFile = (HENHMETAFILE)hVoid;
			break;

		case TYMED_FILE:
			hVoid = OleDuplicateData(pMedium->lpszFileName, pFormatetc->cfFormat, (UINT)NULL);
			pdest->lpszFileName = (LPOLESTR)hVoid;
			break;
	}

	if (hVoid == NULL)
		return FALSE;

	pdest->tymed = pMedium->tymed;
	pdest->pUnkForRelease = pMedium->pUnkForRelease;
	if (pMedium->pUnkForRelease != NULL)
		pMedium->pUnkForRelease->AddRef();

	return TRUE;
}
