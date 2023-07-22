#pragma once

#include "ModuleLoading/TagLoading/TagFramework.h"
#include "TagStructs/bitm.h"
#include "ModuleLoading/ModuleFramework.h"




class ModuleManager {
public:
    struct Tag {
        Tag(uint32_t _4CC, uint32_t _ID, char* _data, char* _clnup, char** _r, uint32_t _rcnt);
        ~Tag();
        uint32_t tag_FourCC;
        uint32_t tagID;
        char* tag_data;
        char* tag_cleanup_ptr;
        char** resources;
        uint32_t resource_count;
        // contain a list of references, so we can wipe them upon tag deletion
        // contain the file header ptr, so we can tell which child tags to clear
        // contain a list of parent tags, so when cleaning parents of shared tags, we dont accidently delete anything we're still using
    };
public:
    void OpenModule(string filename);
    void CloseModule(string filename);
    Tag* GetTag(uint32_t tagID);
    Tag* OpenTag(uint32_t tagID);
    void CloseTag(uint32_t tagID); // WHEN CLOSING TAG WE MUST CLEAR TAGID, SO GAMOBJECTS REALIZE THE TAG IS NON-EXISTANT

    Module* GetModule_AtIndex(uint32_t index);

    void TagToTexture();
    void TagToModel();
private:
    vector<Module*>* loaded_modules = new vector<Module*>();
public:
    uint32_t open_modules = 0;
    uint32_t total_tags = 0;
    vector<Tag>* loaded_tags = new vector<Tag>();
};









/*
// note: 4 byte "DDS_" magic comes before these
struct DDS_HEADER
{
    uint32_t        size; // = 124 maybe 86?
    uint32_t        flags;
    uint32_t        height;
    uint32_t        width;
    uint32_t        pitchOrLinearSize;
    uint32_t        depth; // only if DDS_HEADER_FLAGS_VOLUME is set in flags
    uint32_t        mipMapCount;
    uint32_t        reserved1[11];
    uint32_t        size; // = 32
    uint32_t        flags;
    uint32_t        fourCC;
    uint32_t        RGBBitCount;
    uint32_t        RBitMask;
    uint32_t        GBitMask;
    uint32_t        BBitMask;
    uint32_t        ABitMask;
    uint32_t        caps;
    uint32_t        caps2;
    uint32_t        caps3;
    uint32_t        caps4;
    uint32_t        reserved2;
};

struct DDS_HEADER_DXT10
{
    DXGI_FORMAT     dxgiFormat;
    uint32_t        resourceDimension;
    uint32_t        miscFlag; // see D3D11_RESOURCE_MISC_FLAG
    uint32_t        arraySize;
    uint32_t        miscFlags2; // see DDS_MISC_FLAGS2
};
*/