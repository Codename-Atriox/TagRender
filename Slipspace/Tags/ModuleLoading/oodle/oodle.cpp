#include "oodle.h"

 void Oodle::decompress(char* decompress_buffer, int DecompressedSize, char* CompressedData, int CompressedSize) {
    if (!IsDllLoaded) load_dll();

    int BytesDecompressed = g_OodleDecompressFunc(CompressedData, CompressedSize, decompress_buffer, DecompressedSize, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3);
    if (BytesDecompressed < DecompressedSize) 
        throw std::exception("uncompressed data smaller than specified");
    if (BytesDecompressed > DecompressedSize)
        throw std::exception("uncompressed data greater than specified, memory error!!");
}
void Oodle::load_dll() {
    HINSTANCE mod = LoadLibrary(L"oo2core_8_win64.dll");
    if (!mod)
        throw std::exception("oodle module failed to load");

    g_OodleDecompressFunc = (OodleLZ_Decompress_Func*)GetProcAddress(mod, "OodleLZ_Decompress");
    IsDllLoaded = true;
}
