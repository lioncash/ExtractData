#include "StdAfx.h"

#include "ExtractBase.h"
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
#include "Extract/krkr/Krkr.h"
#include "Extract/krkr/Braban.h"
#include "Extract/krkr/Fate.h"
#include "Extract/krkr/FateFD.h"
#include "Extract/krkr/FlyingShine.h"
#include "Extract/krkr/Hachukano.h"
#include "Extract/krkr/Himesyo.h"
#include "Extract/krkr/Natsupochi.h"
#include "Extract/krkr/SisMiko.h"
#include "Extract/krkr/Specialite.h"
#include "Extract/krkr/Tengu.h"
#include "Extract/krkr/TokiPaku.h"
#include "Extract/krkr/Yotsunoha.h"
#include "Extract/krkr/Tlg.h"
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

#include "UI/Dialog/ExistsDialog.h"
#include "UI/Dialog/SearchDialog.h"
#include "UI/ProgressBar.h"

#include "ArcFile.h"
#include "Image.h" // α processing (alpha processing)
#include "Sound/Ogg.h" // CRC correction

#include "Susie.h"

std::vector<CExtractBase*> CExtract::m_class;
std::vector<CSearchBase*>  CExtract::m_search_class;
std::set<CExtractBase*>	   CExtract::m_decode_class;

CExtract::CExtract()
{
	SetClass();
	SetSearchClass();
}

void CExtract::SetClass()
{
	std::vector<CExtractBase*>& Class = m_class;
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
	std::vector<CSearchBase*>& Class = m_search_class;
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

bool CExtract::Mount(CArcFile* archive)
{
	// Read the archive header

	archive->ReadHeader();
	archive->SeekHed();

	if (archive->GetOpt()->bSusieFirst)
	{
		// Mount the archive using the Susie plug-in
		if (archive->GetOpt()->bSusieUse)
		{
			CSusie susie;

			if (susie.Mount(archive))
			{
				archive->SetMountSusie();
				return true;
			}
		}

		// Mount the archive file
		for (auto* decoder : m_class)
		{
			// Set the class to use to decode the archive
			if (decoder->Mount(archive))
			{
				m_decode_class.insert(decoder);
				return true;
			}
		}
	}
	else
	{
		// Mount the archive file
		for (auto* decoder : m_class)
		{
			// Set the class to use to decode the archive
			if (decoder->Mount(archive))
			{
				m_decode_class.insert(decoder);
				return true;
			}
		}

		// Mount the archive file using the Susie plug-in
		if (archive->GetOpt()->bSusieUse)
		{
			CSusie susie;

			if (susie.Mount(archive))
			{
				archive->SetMountSusie();
				return true;
			}
		}
	}

	// File search
	CSearchDialog search_dialog;
	const INT_PTR return_code = search_dialog.DoModal(archive->GetProg()->GetHandle(), archive->GetArcPath());

	if (return_code == IDYES)
		return Search(archive);

	archive->GetProg()->UpdatePercent(archive->GetArcSize());
	return false;
}

bool CExtract::Search(CArcFile* archive)
{
	const SOption* const option = archive->GetOpt();

	std::vector<CSearchBase*>& SearchClass = m_search_class;
	std::vector<CSearchBase*> Class;
	u32 max_header_size = 0;
	for (size_t i = 0; i < option->bSearch.size(); i++)
	{
		// Add class to use only
		if (option->bSearch[i])
		{
			SearchClass[i]->Init(option);
			Class.push_back(SearchClass[i]);

			// Keep looking for the maximum length of the header in the class to use
			const u32 header_size = SearchClass[i]->GetHeaderSize();
			if (header_size > max_header_size)
				max_header_size = header_size;
		}
	}

	CProgBar* progress_bar = archive->GetProg();
	const size_t num_classes = Class.size();

	// End if no files were set to be searched for
	if (num_classes == 0)
	{
		progress_bar->UpdatePercent(archive->GetArcSize());
		return false;
	}

	std::map<int, int> offsets;

	while (true)
	{
		bool ret = false;

		// Read SEARCH_BUFFER_SIZE segments
		u8 buf[CSearchBase::SEARCH_BUFFER_SIZE];
		const u32 read_size = archive->Read(buf, CSearchBase::SEARCH_BUFFER_SIZE);

		// Search ends when the reading amount is smaller than the header size
		if (read_size < max_header_size)
		{
			progress_bar->UpdatePercent(read_size);
			break;
		}

		const u32 search_size = read_size - max_header_size;

		for (int i = 0; i < (int)num_classes; i++)
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
				archive->Seek(-((int)read_size - entry.first), FILE_CURRENT);
				progress_bar->UpdatePercent(entry.first);

				// Mount if the offset is less
				Class[entry.second]->Mount(archive);

				// TODO: I want to do this without the break.
				break;
			}

			offsets.clear();
		}
		else
		{
			progress_bar->UpdatePercent(search_size);
			// Prevent interruption of data
			archive->Seek(-((int)max_header_size - 1), FILE_CURRENT);
		}
	}

	if (archive->GetCtEnt() == 0)
		return false;

	return true;
}

/**
 * Decode
 *
 * @param archive  Archive
 * @param convert Conversion request
 */
bool CExtract::Decode(CArcFile* archive, bool convert)
{
	bool retval = false;
	const SFileInfo* file_info = archive->GetOpenFileInfo();

	try
	{
		// For processing split files
		archive->SetArcsID(file_info->arcs_id);

		// Set to the position of the file to extract
		archive->SeekHed(file_info->start);

		// Simple initialization of the decryption key
		archive->InitDecrypt();

		// Decoding process
		if (archive->GetMountSusie())
		{
			// Decode with the Susie plug-in
			CSusie susie;
			retval = susie.Decode(archive);
		}
		else
		{
			// Decoded by built-in features
			if (convert)
			{
				// Conversion request
				for (auto* decoder : m_decode_class)
				{
					retval = decoder->Decode(archive);
					if (retval)
						break;
				}

				if (!retval)
				{
					retval = archive->Decode();
				}
			}
			else
			{
				// No conversion request
				for (auto* decoders : m_decode_class)
				{
					retval = decoders->Extract(archive);
					if (retval)
						break;
				}

				if (!retval)
				{
					retval = archive->Extract();
				}
			}
		}
	}
	catch (const CExistsDialog&)
	{
		// If selected 'No' in the overwrite confirmation
		archive->GetProg()->UpdatePercent(file_info->size_org);
		retval = true;
	}

	return retval;
}

void CExtract::Close()
{
	// Reset the class used to decode
	m_decode_class.clear();
}
