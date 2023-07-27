#pragma once
#include <stdint.h>
#include <vector>
namespace ModuleStructs{
    const int32_t module_header_size = 0x50;
    const int32_t module_file_size = 0x58;
    const int32_t block_header_size = 0x14;

    #pragma pack(push, 1)
    struct module_header {
        int32_t     Head;           //  used to determine if this file is actually a module, should be "mohd"
        int32_t     Version;        //  48 flight1, 51 flight2 & retail, season 8: 53
        uint64_t    ModuleId;       //  randomized between modules, algo unknown
        int32_t     FileCount;      //  the total number of tags contained by the module

        int32_t     ManifestCount;       //  'FFFFFFFF' "Number of tags in the load manifest (0 if the module doesn't have one, see the "Load Manifest" section below)"
        int32_t     Manifest_Unk_0x18;   //  'FFFFFFFF' on blank modules, 0 on non blanks, assumedly this the index of the manifest file in the module files array
        int32_t     Manifest_Unk_0x1C;   //  'FFFFFFFF'
    
        int32_t     ResourceIndex;   //  "Index of the first resource entry (numFiles - numResources)"
        int32_t     StringsSize;     //  total size (in bytes) of the strings table
        int32_t     ResourceCount;   //  number of resource files
        int32_t     BlockCount;      //  number of data blocks

        uint64_t    BuildVersion;    // this should be the same between each module
        uint64_t    Checksum;        // "Murmur3_x64_128 of the header (set this field to 0 first), file list, resource list, and block list"

        int32_t     Unk_0x040;       //  0
        int32_t     Unk_0x044;       //  0
        int32_t     Unk_0x048;       //  2
        int32_t     Unk_0x04C;       //  0
    };

    // // // // FLAGS // // // // 
    // (these are probably flipped as i was figuring this out straight from the module files)
    // 0000-0001 <- Uses Compression
    // 0000-0010 <- has blocks, which means to read the data across several data blocks, otherwise read straight from data offset
    // 0000-0100 <- is a raw file, meaning it has no tag header
    const char flag_UseCompression = 0b00000001;
    const char flag_UseBlocks      = 0b00000010;
    const char flag_UseRawfile     = 0b00000100;
    struct module_file {
        char        ClassGroup;     //  
        char        Flags;          // refer to flag bits below this struct
        uint16_t    BlockCount;     // "The number of blocks that make up the file. Only valid if the HasBlocks flag is set"
        uint32_t    BlockIndex;     // "The index of the first block in the file. Only valid if the HasBlocks flag is set"
        uint32_t    ResourceIndex;  // "Index of the first resource in the module's resource list that this file owns"

        uint32_t    ClassId;        // this is the tag group, should be a string right?

        uint32_t    DataOffset;     //how is this not a long???? surely theres no way a uint32_t could actually offset far enough in most files
        uint32_t    Unk_0x14;       // we will now need to double check each file to make sure if this number is ever anything // its used in the very big files

        int32_t     TotalCompressedSize;    // "The total size of compressed data."
        int32_t     TotalUncompressedSize;  // "The total size of the data after it is uncompressed. If this is 0, then the file is empty."

        int32_t     GlobalTagId;   // this is the murmur3 hash; autogenerate from tag path
    
        int32_t     UncompressedHeaderSize;
        int32_t     UncompressedTagDataSize;
        int32_t     UncompressedResourceDataSize;
        int32_t     UncompressedActualResourceDataSize;   // used with bitmaps, and likely other tags idk

        char        HeaderAlignment;             // Power of 2 to align the header buffer to (e.g. 4 = align to a multiple of 16 bytes).
        char        TagDataAlightment;           // Power of 2 to align the tag data buffer to.
        char        ResourceDataAligment;        // Power of 2 to align the resource data buffer to.
        char        ActualResourceDataAligment;  // Power of 2 to align the actual resource data buffer to.

        uint32_t    NameOffset;       // 
        int32_t     ParentIndex;      // "Used with resources to point back to the parent file. -1 = none"
        uint64_t    AssetChecksum;    // "Murmur3_x64_128 hash of (what appears to be) the original file that this file was built from. This is not always the same thing as the file stored in the module. Only verified if the HasBlocks flag is not set."
        uint64_t    AssetId;          // "The asset ID (-1 if not a tag)." maybe other files reference this through its id?

        uint32_t    ResourceCount;  // "Number of resources this file owns"
        int32_t     Unk_0x54;       // so far has just been 0, may relate to hd files?
    };




    struct block_header { // sizeof = 0x14
        int32_t    CompressedOffset;
        int32_t    CompressedSize;
        int32_t    UncompressedOffset;
        int32_t    UncompressedSize;
        int32_t    Compressed;
    };
    #pragma pack(pop)
};