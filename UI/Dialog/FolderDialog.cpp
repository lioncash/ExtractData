#include "StdAfx.h"
#include "UI/Dialog/FolderDialog.h"

#include "Common.h"

bool CFolderDialog::DoModal(HWND window, LPCTSTR title, LPTSTR directory)
{
	Microsoft::WRL::ComPtr<IFileDialog> dialog;
	auto hr = CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(dialog.GetAddressOf()));
	if (FAILED(hr))
		return false;

	if (FAILED(dialog->SetTitle(YCStringW{title})))
		return false;

	DWORD options;
	if (FAILED(dialog->GetOptions(&options)))
		return false;

	if (FAILED(dialog->SetOptions(options | FOS_PICKFOLDERS)))
		return false;

	if (FAILED(dialog->Show(window)))
		return false;

	Microsoft::WRL::ComPtr<IShellItem> result;
	if (FAILED(dialog->GetResult(result.GetAddressOf())))
		return false;

	wchar_t* folder_path;
	if (FAILED(result->GetDisplayName(SIGDN_FILESYSPATH, &folder_path)))
		return false;

	YCStringA folder_path_as_char{folder_path};
	CoTaskMemFree(folder_path);

	std::memcpy(directory, folder_path_as_char.GetString(), folder_path_as_char.GetLength());
	directory[folder_path_as_char.GetLength()] = '\0';
	return true;
}
