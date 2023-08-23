#pragma once
#include <stdint.h>
#include <string>
#include <vector>
#include "../../Utilities/CTList.h"
#include "../../StringHelper.h"



class BitmapResource {
public:
    ~BitmapResource() {
        delete image_view; // not sure if we can do this or not, we'll have to find out
    }
    int32_t resource_index;
    uint32_t Width;
    uint32_t Height;
    bool hd1;
    DirectX::ScratchImage scratch_image;
    ID3D11ShaderResourceView* image_view = nullptr;
};
class ModelResource {
public:
    vector<vector<char>> resource_buffers;
};
struct Tag {
    const static uint32_t bitm = 1651078253;
    const static uint32_t rtgo = 1920231279;
    const static uint32_t mode = 1836016741;
    const static uint32_t sbsp = 1935831920;
    const static uint32_t levl = 1818588780;

    Tag(std::string name, uint32_t _4CC, uint32_t _ID, char* _data, char* _clnup, std::string smod, uint32_t sind) {
        tagname_ext = StringHelper::GetFileNameFromPath(name);
        tagname = StringHelper::GetFileWithoutExtension(tagname_ext);
        tagpath = name;
        tag_FourCC = _4CC;
        tagID = _ID;
        tag_data = _data;
        tag_cleanup_ptr = _clnup;
        source_module = smod;
        source_tag_index = sind;
    }
    ~Tag() {
        delete[] tag_cleanup_ptr;
        switch (tag_FourCC) {
        case bitm:
            for (int i = 0; i < resources.Size(); i++) delete (BitmapResource*)resources[i];
            break;
        case rtgo:
            for (int i = 0; i < resources.Size(); i++) delete (ModelResource*)resources[i];
            break;
        }
    }
    std::string tagname_ext;
    std::string tagname;
    std::string tagpath;
    uint32_t tag_FourCC;
    uint32_t tagID;
    char* tag_data;
    char* tag_cleanup_ptr;
    CTList<void> resources; // resources can be of any type; // i assume this automatically instances this?
    // source data, so fetching resources doesn't require us refinding the tag each time
    std::string source_module;
    uint32_t source_tag_index;
    // placeholder variables for preview windows
    int32_t preview_1 = 0;
    int32_t preview_2 = 0;
    int32_t preview_3 = 0;
};
