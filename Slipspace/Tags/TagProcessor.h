#pragma once

#include <DirectXTex.h>

#include "ModuleLoading/TagLoading/TagFramework.h"
#include "ModuleLoading/ModuleFramework.h"
#include "TagContainers.h"
#include "TagnameFetcher.h"

#include "TagStructs/bitm.h"
#include "../../Utilities/CTList.h"


// #include "../DirectXTex-main/DirectXTex-main/DirectXTex/DirectXTex.h" // we need to fix this



class ModuleManager {
public:

public:
    ModuleManager();

    void OpenModule(string filename);
    void CloseModule(string filename);
    Tag* GetTag(uint32_t tagID); // gets from already open tags
    Tag* OpenTag(uint32_t tagID); // opens from open modules
    void CloseTag(uint32_t tagID); // WHEN CLOSING TAG WE MUST CLEAR TAGID, SO GAMOBJECTS REALIZE THE TAG IS NON-EXISTANT // terrible idea

    Module* GetModule_AtIndex(uint32_t index);
    Module* GetModule_FromTag(Tag* tag);

    BitmapResource* BITM_GetTexture(Tag* tag, ID3D11Device* device, int target_resource = -1);
    void TagToModel(Tag* tag);
    HRESULT OpenTagResource(Tag* tag, uint32_t resource_index, char* resource_out_buffer, uint32_t buffer_size);
    bool IsTagResourceHd1(Tag* tag, uint32_t resource_index);
private:
    vector<Module*> loaded_modules;
    Oodle* unpacker; // so we dont reconfigure this for each module
    Tagnames TagnameProcessor;
public:
    uint32_t open_modules = 0;
    uint32_t total_tags = 0;
    vector<Tag*> loaded_tags;
};









