#include "stdafx.h"

//#include "DataBase/DataBase.h"
#include "Dialog/ExistsDialog.h"
#include "Dialog/SearchDialog.h"
#include "ExtractBase.h"
//#include "FileMap.h"
#include "Extract.h"

#include "Extract/Ahx.h"
#include "Extract/Axl.h"
#include "Extract/Alcot.h"
#include "Extract/AOS.h"
#include "Extract/Aselia.h"
#include "Extract/Baldr.h"
#include "Extract/Clannad.h"
#include "Extract/Circus.h"
#include "Extract/CircusPak.h"
//#include "Extract/Crx.h"
#include "Extract/Cyc.h"
#include "Extract/EntisGLS.h"
#include "Extract/Ethornell.h"
#include "Extract/Himauri.h"
#include "Extract/InnocentGrey.h"
#include "Extract/Izumo.h"
#include "Extract/Katakoi.h"
#include "Extract/LostChild.h"
#include "Extract/Majiro.h"
#include "Extract/MeltyBlood.h"
#include "Extract/Navel.h"
#include "Extract/Nitro.h"
#include "Extract/Nscr.h"
#include "Extract/Oyatu.h"
#include "Extract/Pajamas.h"
#include "Extract/QLIE.h"
#include "Extract/Retouch.h"
#include "Extract/Spitan.h"
#include "Extract/SummerDays.h"
#include "Extract/TaskForce.h"
#include "Extract/TH2.h"
#include "Extract/TrH.h"
#include "Extract/VA.h"
#include "Extract/Will.h"
#include "Extract/Windmill.h"
#include "Extract/Ykc.h"
#include "Extract/Yuris.h"
// cpz
#include "Extract/cpz/Cmv.h"
#include "Extract/cpz/Cpz.h"
// TCD
#include "Extract/TCD/TCD2.h"
#include "Extract/TCD/TCD3.h"
// paz
#include "Extract/paz/Paz.h"
#include "Extract/paz/Haruoto.h"    // Which is also the base class for the following two includes
#include "Extract/paz/AngelType.h"
#include "Extract/paz/WindRP.h"
#include "Extract/paz/HaruotoFD.h"  // It also serves as the base class of ef, which is included before
#include "Extract/paz/ef_ffd.h"
#include "Extract/paz/ef_trial.h"
#include "Extract/paz/ef_first.h"
#include "Extract/paz/ef_sfm.h"
#include "Extract/paz/ef_latter.h"
#include "Extract/paz/BSFre.h"
// 吉里吉里 (KiriKiri)
#include "Extract/Krkr/Krkr.h"
#include "Extract/Krkr/Braban.h"
#include "Extract/Krkr/Fate.h"
#include "Extract/Krkr/FateFD.h"
#include "Extract/Krkr/FlyingShine.h"
#include "Extract/Krkr/Hachukano.h"
#include "Extract/Krkr/Himesyo.h"
#include "Extract/Krkr/Natsupochi.h"
#include "Extract/Krkr/SisMiko.h"
#include "Extract/Krkr/Specialite.h"
#include "Extract/Krkr/Tengu.h"
#include "Extract/Krkr/TokiPaku.h"
#include "Extract/Krkr/Yotsunoha.h"
#include "Extract/Krkr/Tlg.h"
// Search
#include "Search/AhxSearch.h"
#include "Search/BmpSearch.h"
#include "Search/JpgSearch.h"
#include "Search/MidSearch.h"
#include "Search/MpgSearch.h"
#include "Search/OggSearch.h"
#include "Search/PngSearch.h"
#include "Search/SearchBase.h"
#include "Search/WavSearch.h"
#include "Search/WmvSearch.h"

#include "Image.h" // α processing (alpha processing)
#include "Sound/Ogg.h" // CRC correction

#include "Susie.h"

std::vector<CExtractBase*> CExtract::m_Class;
std::vector<CSearchBase*>  CExtract::m_SearchClass;
std::set<CExtractBase*>	   CExtract::m_DecodeClass;

CExtract::CExtract()
{
	SetClass();
	SetSearchClass();
}

void CExtract::SetClass()
{
	std::vector<CExtractBase*>& Class = m_Class;
	static CAhx ahx;                    Class.push_back(&ahx);
	static CAxl axl;                    Class.push_back(&axl);
	static CAlcot alcot;                Class.push_back(&alcot);
	static CAOS clAOS;                  Class.push_back(&clAOS);
	static CAselia aselia;              Class.push_back(&aselia);
	static CBaldr baldr;                Class.push_back(&baldr);
	static CCircus clCircus;            Class.push_back(&clCircus);
	static CCircusPak clCircusPak;      Class.push_back(&clCircusPak);
	static CClannad clannad;            Class.push_back(&clannad);
	static CCmv cmv;                    Class.push_back(&cmv);
	static CCpz cpz;                    Class.push_back(&cpz);
//  static CCrx crx;                    Class.push_back(&crx);
	static CCyc cyc;                    Class.push_back(&cyc);
	static CEntisGLS clEntisGLS;        Class.push_back(&clEntisGLS);
	static CEthornell ethornell;        Class.push_back(&ethornell);
	static CHimauri himauri;            Class.push_back(&himauri);
	static CInnocentGrey innocent;      Class.push_back(&innocent);
	static CIzumo izumo;                Class.push_back(&izumo);
	static CKatakoi katakoi;            Class.push_back(&katakoi);
	static CMajiro majiro;              Class.push_back(&majiro);
	static CMeltyBlood melblo;          Class.push_back(&melblo);
	static CNavel navel;                Class.push_back(&navel);
	static CNitro nitro;                Class.push_back(&nitro);
	static CNscr nscr;                  Class.push_back(&nscr);
	static COyatu clOyatu;              Class.push_back(&clOyatu);
	static CPajamas pajamas;            Class.push_back(&pajamas);
	static CQLIE clQLIE;                Class.push_back(&clQLIE);
	static CRetouch clRetouch;          Class.push_back(&clRetouch);
	static CSpitan spitan;              Class.push_back(&spitan);
	static CSummerDays summerdays;      Class.push_back(&summerdays);
	static CTaskForce clTaskForce;      Class.push_back(&clTaskForce);
	static CTH2 th2;                    Class.push_back(&th2);
	static CTrH trh;                    Class.push_back(&trh);
	static CVA va;                      Class.push_back(&va);
	static CWill will;                  Class.push_back(&will);
	static CWindmill windmill;          Class.push_back(&windmill);
	static CLostChild lostchild;        Class.push_back(&lostchild);
	static CYkc ykc;                    Class.push_back(&ykc);
	static CYuris yuris;                Class.push_back(&yuris);
	// paz
	static Cef_ffd ef_ffd;              Class.push_back(&ef_ffd);
	static Cef_trial ef_trial;          Class.push_back(&ef_trial);
	static Cef_first ef_first;          Class.push_back(&ef_first);
	static CEFsfm clEFsfm;              Class.push_back(&clEFsfm);
	static CEFLatter clEFLatter;        Class.push_back(&clEFLatter);
	static CHaruoto haruoto;            Class.push_back(&haruoto);
	static CHaruotoFD haruotoFD;        Class.push_back(&haruotoFD);
	static CAngelType clAngelType;      Class.push_back(&clAngelType);
	static CWindRP clWindRP;            Class.push_back(&clWindRP);
	static CBSFre clBSFre;              Class.push_back(&clBSFre);
	// TCD
	static CTCD2 clTCD2;                Class.push_back(&clTCD2);
	static CTCD3 clTCD3;                Class.push_back(&clTCD3);
	// 吉里吉里 (KiriKiri)
	static CBraban braban;              Class.push_back(&braban);
	static CFate fate;                  Class.push_back(&fate);
	static CFateFD fateFD;              Class.push_back(&fateFD);
	static CFlyingShine flying;         Class.push_back(&flying);
	static CHachukano hachukano;        Class.push_back(&hachukano);
	static CHimesyo himesyo;            Class.push_back(&himesyo);
	static CNatsupochi natsupochi;      Class.push_back(&natsupochi);
	static CSisMiko sismiko;            Class.push_back(&sismiko);
	static CSpecialite specialite;      Class.push_back(&specialite);
	static CTengu clTengu;              Class.push_back(&clTengu);
	static CTokiPaku tokipaku;          Class.push_back(&tokipaku);
	static CYotsunoha yotsunoha;        Class.push_back(&yotsunoha);
	static CKrkr krkr;                  Class.push_back(&krkr); // Kirikiri class can be added to the end I'd like to check first whether you must perform decryption
	static CTlg clTlg;                  Class.push_back(&clTlg);
	//
	static CImage image;                Class.push_back(&image);
	static COgg ogg;                    Class.push_back(&ogg);
}

void CExtract::SetSearchClass()
{
	std::vector<CSearchBase*>& Class = m_SearchClass;
	static CAhxSearch ahx; Class.push_back(&ahx);
	static CBmpSearch bmp; Class.push_back(&bmp);
	static CJpgSearch jpg; Class.push_back(&jpg);
	static CMidSearch mid; Class.push_back(&mid);
	static CMpgSearch mpg; Class.push_back(&mpg);
	static COggSearch ogg; Class.push_back(&ogg);
	static CPngSearch png; Class.push_back(&png);
	static CWavSearch wav; Class.push_back(&wav);
	static CWmvSearch wmv; Class.push_back(&wmv);
}

//////////////////////////////////////////////////////////////////////////////////////////
// Mounting

bool CExtract::Mount(CArcFile* pclArc)
{
	// Read the archive header

	pclArc->ReadHed();
	pclArc->SeekHed();

	if (pclArc->GetOpt()->bSusieFirst)
	{
		// Mount the archive using the Susie plug-in

		if (pclArc->GetOpt()->bSusieUse)
		{
			CSusie clSusie;

			if (clSusie.Mount(pclArc))
			{
				pclArc->SetMountSusie();
				return true;
			}
		}

		// Mount the archive file
		for (auto* decoder : m_Class)
		{
			// Set the class to use to decode the archive
			if (decoder->Mount(pclArc))
			{
				m_DecodeClass.insert(decoder);
				return true;
			}
		}
	}
	else
	{
		// Mount the archive file
		for (auto* decoder : m_Class)
		{
			// Set the class to use to decode the archive
			if (decoder->Mount(pclArc))
			{
				m_DecodeClass.insert(decoder);
				return true;
			}
		}

		// Mount the archive file using the Susie plug-in
		if (pclArc->GetOpt()->bSusieUse)
		{
			CSusie clSusie;

			if (clSusie.Mount(pclArc))
			{
				pclArc->SetMountSusie();
				return true;
			}
		}
	}

	// File search

	CSearchDialog clSearchDlg;

	int nReturn = clSearchDlg.DoModal(pclArc->GetProg()->GetHandle(), pclArc->GetArcPath());

	if (nReturn == IDYES)
	{
		return Search(pclArc);
	}
	else
	{
		pclArc->GetProg()->UpdatePercent(pclArc->GetArcSize());
	}

	return false;
}

bool CExtract::Search(CArcFile* pclArc)
{
	SOption* pOption = pclArc->GetOpt();

/*
	// Check the database
	CDataBase db;
	if (db.Check(pclArc) == TRUE)
		return;
	//std::vector<CSearchBase*> Class = m_SearchClass;
	for (int i = 0; i < (int)pOption->bSearch.size(); i++)
	{
		pOption->bSearch[i] = FALSE;
	}
	pOption->bSearch[3] = TRUE;
*/

	std::vector<CSearchBase*>& SearchClass = m_SearchClass;
	std::vector<CSearchBase*> Class;
	u32 max_header_size = 0;
	for (size_t i = 0; i < pOption->bSearch.size(); i++)
	{
		// Add class to use only
		if (pOption->bSearch[i] == TRUE)
		{
			SearchClass[i]->Init(pOption);
			Class.push_back(SearchClass[i]);

			// Keep looking for the maximum length of the header in the class to use
			const u32 header_size = SearchClass[i]->GetHedSize();
			if (header_size > max_header_size)
				max_header_size = header_size;
		}
	}

	CProgBar* pProg = pclArc->GetProg();
	size_t ctClass = Class.size();

	// End if no files were set to be searched for
	if (ctClass == 0)
	{
		pProg->UpdatePercent(pclArc->GetArcSize());
		return false;
	}
/*
	CFileMap FileMap;
	LPBYTE buf = (LPBYTE)FileMap.Open(pclArc->GetArcHandle(), FILE_READ);
	if (buf == NULL)
		return;

	DWORD BufSize = 65536;
	QWORD ArcSize = pclArc->GetArcSize() - maxHedSize;
	for (INT64 i = 0; i < (INT64)ArcSize; i += BufSize)
	{
		for (int j = 0; j < (int)ctClass; j++)
		{
			for (int k = 0; k < (int)BufSize; k++)
			{
				if (Class[j]->CmpHed(&buf[k]) == TRUE)
					break;
			}

			//if (Class[j]->CmpMem(buf, Class[j]->GetHed(), Class[j]->GetHedSize()) == TRUE)
			//{
				//Class[j]->Mount(pclArc);
				//break;
			//}
		}

		//if ((i & 65535) == 0)
			pProg->UpdatePercent(BufSize);

		//buf++;
			buf += BufSize;
		//Class[0]->CmpMem(buf, "\x89\x50\x4E\x47\x0D\x0A\x1A\x0A", 8);
		//Class[0]->CmpMem(buf, "\x00\x00\x01\xBA\x21\x00\x01\x00", 8);
		//Class[0]->CmpMem(buf, "\x30\x26\xB2\x75\x8E\x66\xCF\x11\xA6\xD9\x00\xAA\x00\x62\xCE\x6C", 16);
		//if (memcmp(buf, "\x89\x50\x4E\x47\x0D\x0A\x1A\x0A", 8) == 0);
		//if (memcmp(buf, "\x00\x00\x01\xBA\x21\x00\x01\x00", 8) == 0);
		//if (memcmp(buf, "\x30\x26\xB2\x75\x8E\x66\xCF\x11\xA6\xD9\x00\xAA\x00\x62\xCE\x6C", 16) == 0);
		//if ((i & 65535) == 0)
		//  pProg->UpdatePercent(65536);
		//buf++;
	}
*/
	//QWORD ArcSize = pclArc->GetArcSize();
	//HANDLE hFileMap = CreateFileMapping(pclArc->GetArcHandle(), NULL, PAGE_READONLY, 0, ArcSize, NULL);
	//LPBYTE buf = (LPBYTE)MapViewOfFile(hFileMap, FILE_MAP_READ, 0, 0, 0);

//  for (INT64 i = 0; i < (INT64)ArcSize; i++) {
		//if (Class[0]->Search(pclArc, buf, dwReadSize) == TRUE) {
		//Class[0]->CmpMem(buf, "\x89\x50\x4E\x47\x0D\x0A\x1A\x0A", 8);
		//Class[0]->CmpMem(buf, "\x00\x00\x01\xBA\x21\x00\x01\x00", 8);
		//Class[0]->CmpMem(buf, "\x30\x26\xB2\x75\x8E\x66\xCF\x11\xA6\xD9\x00\xAA\x00\x62\xCE\x6C", 16);
		//if (memcmp(buf, "\x89\x50\x4E\x47\x0D\x0A\x1A\x0A", 8) == 0);
		//if (memcmp(buf, "\x00\x00\x01\xBA\x21\x00\x01\x00", 8) == 0);
		//if (memcmp(buf, "\x30\x26\xB2\x75\x8E\x66\xCF\x11\xA6\xD9\x00\xAA\x00\x62\xCE\x6C", 16) == 0);
		//if ((i & 65535) == 0)
		//  pProg->UpdatePercent(65536);
		//buf++;
//  }

/*
	for (INT64 i = 0; i < (INT64)ArcSize; i++)
	{
		BOOL ret = FALSE;
		for (int i = 0; i < (int)ctClass; i++)
		{
			if (Class[i]->Search(pclArc, buf, dwReadSize) == TRUE)
			{
				Class[i]->Mount(pclArc, buf);
				ret = TRUE;
				break;
			}
		}

		if (ret == FALSE)
		{
			pProg->UpdatePercent(dwReadSize);
			// Prevent interruption of data
			pclArc->Seek(-((int)maxHedSize-1), FILE_CURRENT);
		}
	}
*/
//  UnmapViewOfFile(buf);
//  CloseHandle(hFileMap);
	std::map<int, int> offsets;

	while (true)
	{
		bool ret = false;

		// Read SEARCH_BUFFER_SIZE segments
		u8 buf[CSearchBase::SEARCH_BUFFER_SIZE];
		const u32 read_size = pclArc->Read(buf, CSearchBase::SEARCH_BUFFER_SIZE);

		// Search ends when the reading amount is smaller than the header size
		if (read_size < max_header_size)
		{
			pProg->UpdatePercent(read_size);
			break;
		}

		const u32 search_size = read_size - max_header_size;

		for (int i = 0; i < (int)ctClass; i++)
		{
			if (Class[i]->Search(buf, search_size))
			{
				offsets.emplace(static_cast<int>(Class[i]->GetOffset()), i);
				ret = true;
			}
		}

		if (ret)
		{
			for (const auto& entry : offsets)
			{
				// dwReadSize - offset, back to the first position found, has moved to the header file
				pclArc->Seek(-((int)read_size - entry.first), FILE_CURRENT);
				pProg->UpdatePercent(entry.first);

				// Mount if the offset is less
				Class[entry.second]->Mount(pclArc);

				// TODO: I want to do this without the break.
				break;
			}

			offsets.clear();
		}
		else
		{
			pProg->UpdatePercent(search_size);
			// Prevent interruption of data
			pclArc->Seek(-((int)max_header_size - 1), FILE_CURRENT);
		}
	}

/*
	while (1)
	{
		BOOL ret = FALSE;
		BYTE buf[BUFSIZE];
		DWORD dwReadSize = pclArc->Read(buf, BUFSIZE) - maxHedSize;

		if (dwReadSize == -1)
			break;

		for (int i = 0; i < (int)dwReadSize; i++)
		{
			for (int j = 0; j < (int)ctClass; j++)
			{
				if (Class[j]->CmpHed(&buf[i]) == TRUE)
				{
					pclArc->Seek(-(BUFSIZE-i), FILE_CURRENT);
					pProg->UpdatePercent(i);
					Class[j]->Mount(pclArc);
					ret = TRUE;
					break;
				}
			}

			// Resume when the reading is called from Mount()
			if (ret == TRUE)
				break;
		}

		if (ret == FALSE)
			pProg->UpdatePercent(dwReadSize);

		// Prevent interruption of data
		pclArc->Seek(-((int)maxHedSize-1), FILE_CURRENT);
	}
	*/

	if (pclArc->GetCtEnt() == 0)
		return false;

	return true;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Decode

/**
 * @param pclArc  Archive
 * @param convert Conversion request
 */
bool CExtract::Decode(CArcFile* pclArc, bool convert)
{
	bool       retval = false;
	SFileInfo* pstFileInfo = pclArc->GetOpenFileInfo();

	try
	{
		// For processing split files
		pclArc->SetArcsID( pstFileInfo->arcsID );

		// Set to the position of the file to extract
		pclArc->SeekHed( pstFileInfo->start );

		// Simple initialization of the decryption key
		pclArc->InitDecrypt();

		// Decoding process
		if (pclArc->GetMountSusie())
		{
			// Decode with the Susie plug-in

			CSusie clSusie;
			retval = clSusie.Decode(pclArc);
		}
		else
		{
			// Decoded by built-in features
			if (convert)
			{
				// Conversion request
				for (auto* decoder : m_DecodeClass)
				{
					retval = decoder->Decode(pclArc);
					if (retval)
						break;
				}

				if (!retval)
				{
					retval = pclArc->Decode();
				}
			}
			else
			{
				// No conversion request
				for (auto* decoders : m_DecodeClass)
				{
					retval = decoders->Extract(pclArc);
					if (retval)
						break;
				}

				if (!retval)
				{
					retval = pclArc->Extract();
				}
			}
		}

/*
		// Decode

		if( bConvert )
		{
			// Conversion request

			if( pclArc->GetMountSusie() )
			{
				// Decode with Susie plug-in

				CSusie susie;

				if( susie.Decode( pclArc ) )
				{
					return;
				}
			}
			else
			{
				// Decode the files in the archive

				std::set<CExtractBase*>& Class = m_DecodeClass;

				for( std::set<CExtractBase*>::iterator itr = Class.begin() ; itr != Class.end() ; itr++ )
				{
					if( (*itr)->Decode( pclArc ) )
					{
						return;
					}
				}
			}

			pclArc->Decode();
		}
		else
		{
			// No conversion requests 

			pclArc->OpenFile();

			if( pclArc->GetArcExten() == _T(".xp3") )
			{
				// 吉里吉里 (KiriKiri)

				DWORD dwBufferSize = pclArc->GetBufSize();
				DWORD dwBufferSizeBase = dwBufferSize;

				YCMemory<BYTE> clmbtBuffer( dwBufferSize );

				for( size_t i = 0 ; i < pstFileInfo->starts.size() ; i++ )
				{
					dwBufferSize = dwBufferSizeBase;

					pclArc->SeekHed( pstFileInfo->starts[i] );

					DWORD dwDstSize = pstFileInfo->sizesOrg[i];

					for( DWORD dwWroteSizes = 0 ; dwWroteSizes != dwDstSize ; dwWroteSizes += dwBufferSize )
					{
						// Adjust buffer size

						pclArc->SetBufSize( &dwBufferSize, dwWroteSizes, dwDstSize );

						pclArc->Read( &clmbtBuffer[0], dwBufferSize );
						pclArc->WriteFile( &clmbtBuffer[0], dwBufferSize );
					}
				}
			}
			else
			{
				pclArc->ReadWrite();
			}

			pclArc->CloseFile();
		}*/
	}
	catch (CExistsDialog)
	{
		// If selected 'No' in the overwrite confirmation

		pclArc->GetProg()->UpdatePercent(pstFileInfo->sizeOrg);

		retval = true;
	}

	return retval;
}

void CExtract::Close()
{
	// Reset the class used to decode
	m_DecodeClass.clear();
}
