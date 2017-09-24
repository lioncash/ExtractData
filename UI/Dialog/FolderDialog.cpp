#include "StdAfx.h"
#include "UI/Dialog/FolderDialog.h"

#include "Common.h"

BOOL CFolderDialog::DoModal(HWND window, LPCTSTR title, LPTSTR directory)
{
	Microsoft::WRL::ComPtr<IFileDialog> dialog;
	auto hr = CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(dialog.GetAddressOf()));
	if (FAILED(hr))
		return FALSE;

	if (FAILED(dialog->SetTitle(YCStringW{title})))
		return FALSE;

	DWORD options;
	if (FAILED(dialog->GetOptions(&options)))
		return FALSE;

	if (FAILED(dialog->SetOptions(options | FOS_PICKFOLDERS)))
		return FALSE;

	if (FAILED(dialog->Show(window)))
		return FALSE;

	Microsoft::WRL::ComPtr<IShellItem> result;
	if (FAILED(dialog->GetResult(result.GetAddressOf())))
		return FALSE;

	wchar_t* folder_path;
	if (FAILED(result->GetDisplayName(SIGDN_FILESYSPATH, &folder_path)))
		return FALSE;

	YCStringA folder_path_as_char{folder_path};
	CoTaskMemFree(folder_path);

	std::memcpy(directory, folder_path_as_char.GetString(), folder_path_as_char.GetLength());
	directory[folder_path_as_char.GetLength()] = '\0';
	return TRUE;
}
