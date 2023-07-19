#include "oodle.h"

 void Oodle::decompress(char* decompress_buffer, int DecompressedSize, char* CompressedData, int CompressedSize) {
    if (!IsDllLoaded) load_dll();

    int BytesDecompressed = g_OodleDecompressFunc(CompressedData, CompressedSize, decompress_buffer, DecompressedSize, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3);
    if (BytesDecompressed != DecompressedSize) 
        throw std::exception("uncompressed data the wrong size, could cause logic errors");
}
void Oodle::load_dll() {
    HINSTANCE mod = LoadLibrary(StringHelper::StringToWide("oo2core_8_win64.dll").c_str());
    if (mod == (HINSTANCE)0)
        throw new std::exception("oodle module failed to load");

    OodleLZ_Decompress_Func* g_OodleDecompressFunc = (OodleLZ_Decompress_Func*)GetProcAddress(mod, "OodleLZ_Decompress");
    IsDllLoaded = true;
}
