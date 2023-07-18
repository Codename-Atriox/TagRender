#include "oodle.h"

char* Oodle::decompress(int& DecompressedSize, char* CompressedData, int CompressedSize) {
    if (!IsDllLoaded) load_dll();

    char* Output_data = new char[DecompressedSize];
    int BytesDecompressed = g_OodleDecompressFunc(CompressedData, CompressedSize, Output_data, DecompressedSize, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3);
    if (BytesDecompressed != DecompressedSize) {
        DecompressedSize = BytesDecompressed; // this is so we know
        throw std::exception("uncompressed data the wrong size, could cause logic errors");
    }
    return Output_data;
}
void Oodle::load_dll() {
    std::wstring dll_name = StringHelper::StringToWide("oo2core_8_win64.dll");
    HINSTANCE mod = LoadLibrary(dll_name.c_str());
    OodleLZ_Decompress_Func* g_OodleDecompressFunc = (OodleLZ_Decompress_Func*)GetProcAddress(mod, "OodleLZ_Decompress");
    IsDllLoaded = true;
}
