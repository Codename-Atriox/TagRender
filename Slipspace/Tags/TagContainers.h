#pragma once
#include <stdint.h>
#include <string>
#include <vector>
#include "../../Utilities/CTList.h"
#include "../../StringHelper.h"

struct Tag {
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
};
class BitmapResource {
    ~BitmapResource() {
        delete bitmap;
    }
    uint32_t resource_index;
    uint32_t X;
    uint32_t Y;
    ID3D11ShaderResourceView* bitmap;
};
class ModelResource {
    vector<float> verticies;
    vector<uint64_t> triangle_indicies; // should be an array of struct or something idk
};