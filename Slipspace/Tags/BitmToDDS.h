#pragma once
#include <stdint.h>
#include "TagStructs/bitm.h"
//#include "../DirectXTex-main/DirectXTex-main/DirectXTex/DirectXTex.h"
//#include "../DirectXTex-main/DirectXTex-main/DirectXTex/DDS.h"
//#include "../DirectXTex-main/DirectXTex-main/DirectXTex/DirectXTexDDS.cpp"
#include <DirectXTex.h>


// we need to create this function in the tag processor cod , because we need to access the tag reousrces and beable to call up the resources
static void ConvertBITMtoDDS(BitmapGroup* bitm) {

    // pseudo select-best-image code here
    // aka select the first one, because if theres more, then why???
    BitmapData* selected_bitmap = bitm->bitmaps[0];
    BitmapDataResource* bitmap_details = selected_bitmap->bitmap_resource_handle.content_ptr;
    if (selected_bitmap->type != BitmapType::_2D_texture) {
        throw new exception("unsupported image type");
    }
    DirectX::TexMetadata* meta = new DirectX::TexMetadata();

    // figure out if this texture is using internal data, or resource data
    if (bitmap_details->pixels.data_size != 0) { // use pixel data
        meta->width = selected_bitmap->height;
        meta->height = selected_bitmap->width;
    }else{ // use streaming data
        if (bitmap_details->streamingData.count == 0)
            throw new exception("no streaming data or pixel data");

        // select the image that we want to be using
        uint32_t index = 0; // probably the lowest quality
        if (index >= bitmap_details->streamingData.count)
            throw new exception("out of bounds index for streaming texture array");

        meta->width = bitmap_details->streamingData[index]->dimensions & 0x0000FFFF;
        meta->height = (bitmap_details->streamingData[index]->dimensions >> 16) & 0x0000FFFF;
    }

    meta->depth = selected_bitmap->depth;
    meta->arraySize = 1; // no support for array types yet
    meta->mipLevels = selected_bitmap->mipmap_count;
    // only for extended/compressed dds
    meta->miscFlags = 0; // the only flag seems to be TEX_MISC_TEXTURECUBE = 0x4
    meta->miscFlags2 = 0;
    meta->format = (DXGI_FORMAT)bitmap_details->format;
    meta->dimension = (DirectX::TEX_DIMENSION)3; // TEX_DIMENSION_TEXTURE2D


    size_t header_size = sizeof(uint32_t) + 124; // sizeof(DirectX::DDS_HEADER);
    if (DirectX::IsCompressed(meta->format)) header_size += 20; // sizeof(DirectX::DDS_HEADER_DXT10);

    char* DDSheader_dest = new char[header_size];

    size_t output_size = 0;
    HRESULT hr = EncodeDDSHeader(*meta, DirectX::DDS_FLAGS_NONE, (void*)DDSheader_dest, header_size, output_size);
    if (!SUCCEEDED(hr))
        throw new exception("image failed to generate DDS header");


    if (header_size != output_size)
        throw new exception("header size was incorrectly assumed! must investigate this image format!!!");

    return;
    /*
    image->header.size = 124;
    image->header.flags = 0x1 + 0x2 + 0x4 + 0x1000 + 0x8; // ?? what are all these for
    image->header.height = selected_bitmap->height;
    image->header.width = selected_bitmap->width;
    image->header.depth = selected_bitmap->depth;
    image->header.mipMapCount = selected_bitmap->mipmap_count;

    image->header.ddspf.size = 32;
    image->header.ddspf.flags = 0x1; // alpha, will |= it later
    image->header.ddspf.fourCC = 0; // dont set this yet?
    image->header.ddspf.RGBBitCount = 32;
    image->header.ddspf.RBitMask = 0x000000FF;
    image->header.ddspf.GBitMask = 0x0000FF00;
    image->header.ddspf.BBitMask = 0x00FF0000;
    image->header.ddspf.ABitMask = 0xFF000000;

    image->header.caps = 0x1000;
    image->header.caps2 = 0;
    image->header.caps3 = 0;
    image->header.caps4 = 0;
    image->header.reserved2 = 0;

    if (extended_header){
        image->header.ddspf.flags |= 0x4;
        image->header.ddspf.fourCC = 0x30445831; // 'DX10'
        image->extra.dxgiFormat = 0; // ADD THIS HERE WHATEVER IT IS
        image->extra.resourceDimension = 3;
        image->extra.miscFlag = 0;
        image->extra.arraySize = 1;
        image->extra.miscFlags2 = 0;
    }else{
        image->header.ddspf.flags |= 0x40;
        //image->extra.miscFlag = 0;
        //image->extra.arraySize = 1;
        //image->extra.miscFlags2 = 0x1;
    }
    */
}

