#pragma once
#include <stdint.h>
#include <wtypes.h>
#include <stdexcept>
#include "../../../../StringHelper.h" // wow thats a lot

class Oodle {
public:
    enum class OodleCompressionLevel : uint64_t{
        None,
        SuperFast,
        VeryFast,
        Fast,
        Normal,
        Optimal1,
        Optimal2,
        Optimal3,
        Optimal4,
        Optimal5
    };
    enum class OodleFormat : uint32_t{
        LZH,
        LZHLW,
        LZNIB,
        None,
        LZB16,
        LZBLW,
        LZA,
        LZNA,
        Kraken,
        Mermaid,
        BitKnit,
        Selkie,
        Akkorokamui
    };
    char* decompress(int& DecompressedSize, char* CompressedData, int CompressedSize);
private:
    typedef int __stdcall OodleLZ_Decompress_Func(char* buffer, int bufferSize, char* outputBuffer, int outputBufferSize, int a, int b, int c, void* d, void* e, void* f, void* g, void* h, void* i, int threadModule);

    HINSTANCE mod;
    OodleLZ_Decompress_Func* g_OodleDecompressFunc;

    bool IsDllLoaded = false;
    void load_dll();
};


