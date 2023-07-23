#pragma once
#include "NFD.h"

bool NativeFileDialogue::NFD_OpenDialog(std::string& outPath) {
    HRESULT result = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    if (!SUCCEEDED(result))
        return false;

    // Create dialog
    IFileOpenDialog* fileOpenDialog(NULL);
    result = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL, IID_IFileOpenDialog, reinterpret_cast<void**>(&fileOpenDialog));
    if (!SUCCEEDED(result)) {
        CoUninitialize();
        return false;
    }

    // Show the dialog.
    result = fileOpenDialog->Show(NULL);
    if (!SUCCEEDED(result)){
        fileOpenDialog->Release();
        CoUninitialize();
        return false;
    }

    // Get the file name
    IShellItem* shellItem(NULL);
    result = fileOpenDialog->GetResult(&shellItem);
    if (!SUCCEEDED(result)) {
        fileOpenDialog->Release();
        CoUninitialize();
        return false;
    }

    // get the path of the filename or something
    wchar_t* filePath(NULL);
    result = shellItem->GetDisplayName(SIGDN_FILESYSPATH, &filePath);
    if (!SUCCEEDED(result)) {
        shellItem->Release();
        fileOpenDialog->Release();
        CoUninitialize();
        return false;
    }

    std::wstring path(filePath);
    std::string c(path.begin(), path.end());
    outPath = c;
        
    CoTaskMemFree(filePath);
    shellItem->Release();
    fileOpenDialog->Release();
    CoUninitialize();
    return true;
}








