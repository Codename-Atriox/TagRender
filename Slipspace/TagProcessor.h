#pragma once

#include "TagFramework.h"
#include "TagStructs/bitm.h"

void* handle_tag() // returns the data to the newly constructed object
{

	runtime_tag* _Out_tag; // we need to cleanup this after converting to another format
	auto Tag_Result = Opentag("D:\\T\\__chore\\pc__\\levels\\assets\\hack\\texture_tiles\\wz_rubble\\wz_rubble_concrete_broken_normal{pc}.bitmap", _Out_tag);

	switch (Tag_Result)
	{
	case bitmap:
		convert_tag_to_usable_texture(_Out_tag);
		break;
	case render_model:

		break;
	case runtime_geo:

		break;
	}
}
void convert_tag_to_usable_texture(runtime_tag* tag_data)
{
	BitmapGroup* bitmap_tag = (BitmapGroup*)tag_data->tag_data;
    // yes, we're doing it the silly way, would love to have a system that could handle all the lods
    // but we set this up terribly, maybe if we make a better version that just loads the
    int biggest_chunk = -1;
    int largest_chunk_size = -1;
    for (int c = 0; c < (*tag_data->streaming_chunks); c++)
    {
        if ((*tag_data->streaming_chunks)[c].size > largest_chunk_size)
        {
            biggest_chunk = c;
            largest_chunk_size = (*tag_data->streaming_chunks)[c].size;
        }
    }


}








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