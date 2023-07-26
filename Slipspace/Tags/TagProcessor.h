#pragma once

#include "ModuleLoading/TagLoading/TagFramework.h"
#include "ModuleLoading/ModuleFramework.h"


#include "TagStructs/bitm.h"
#include <DirectXTex.h>
#include "CTList.h"
// #include "../DirectXTex-main/DirectXTex-main/DirectXTex/DirectXTex.h" // we need to fix this



class ModuleManager {
public:
    struct Tag {
        Tag(uint32_t _4CC, uint32_t _ID, char* _data, char* _clnup, std::string smod, uint32_t sind);
        ~Tag();
        uint32_t tag_FourCC;
        uint32_t tagID;
        char* tag_data;
        char* tag_cleanup_ptr;
        CTList<ID3D11ShaderResourceView> resources;
        // source data, so fetching resources doesn't require us refinding the tag each time
        std::string source_module;
        uint32_t source_tag_index;
        // contain a list of references, so we can wipe them upon tag deletion
        // contain the file header ptr, so we can tell which child tags to clear
        // contain a list of parent tags, so when cleaning parents of shared tags, we dont accidently delete anything we're still using
        // alternatively, a use counter, so we can track how many tags are still relying on this one
    };
public:
    ModuleManager();

    void OpenModule(string filename);
    void CloseModule(string filename);
    Tag* GetTag(uint32_t tagID); // gets from already open tags
    Tag* OpenTag(uint32_t tagID); // opens from open modules
    void CloseTag(uint32_t tagID); // WHEN CLOSING TAG WE MUST CLEAR TAGID, SO GAMOBJECTS REALIZE THE TAG IS NON-EXISTANT

    Module* GetModule_AtIndex(uint32_t index);

    ID3D11ShaderResourceView* BITM_GetTexture(Tag* tag, ID3D11Device* device);
    void TagToModel(Tag* tag);
    void OpenTagResource(Tag* tag, uint32_t resource_index, char* resource_out_buffer, uint32_t buffer_size);
private:
    vector<Module*>* loaded_modules = new vector<Module*>();
    Oodle* unpacker; // so we dont reconfigure this for each module
public:
    uint32_t open_modules = 0;
    uint32_t total_tags = 0;
    vector<Tag*>* loaded_tags = new vector<Tag*>();
};









