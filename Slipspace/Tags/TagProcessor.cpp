#include "TagProcessor.h"




ModuleManager::ModuleManager()
{
	unpacker = new Oodle();
}

void ModuleManager::OpenModule(string filename){
	CloseModule(filename); // if this module is already open, reopen it

	try{
		Module* new_module = new Module(filename, unpacker);
		loaded_modules.push_back(new_module);
		open_modules++;
		total_tags += new_module->file_count;
	} catch (exception ex) {
		throw exception("failed to open module");
	}
}

void ModuleManager::CloseModule(string filename){
	/*
	for (int c = 0; c < loaded_modules.size(); c++) {
		Module* curr_module = (*loaded_modules)[c];
		if (curr_module->filepath != filename) continue;

		open_modules--;
		total_tags -= curr_module->file_count;
		std::vector<Module*>& vec = *loaded_modules;
		vec.erase(std::remove(vec.begin(), vec.end(), c), vec.end());
		delete curr_module; // destroy it
		return;
	}
	*/
}

Tag* ModuleManager::GetTag(uint32_t tagID) {
	for (int i = 0; i < loaded_tags.size(); i++)
		if (loaded_tags[i]->tagID == tagID)
			return loaded_tags[i];
	return (Tag*)0;
}
Tag* ModuleManager::OpenTag(uint32_t tagID){
	// first check if the tag already exists
	Tag* new_tag = GetTag(tagID);
	if (new_tag != (Tag*)0) return new_tag;

	for (int c = 0; c < loaded_modules.size(); c++) {
		Module* module_ptr = loaded_modules[c];

		int32_t tag_index = module_ptr->find_tag_index(tagID);
		if (tag_index == -1) continue; // this module does not contain the tag

		module_file* file_ptr = module_ptr->GetTagHeader_AtIndex(tag_index);

		char* output_tag_bytes;
		char* output_cleanup_ptr;
		module_ptr->GetTagProcessed(file_ptr, output_tag_bytes, output_cleanup_ptr);


		Tag* new_tag = new Tag(TagnameProcessor.GetTagname(file_ptr->GlobalTagId), file_ptr->ClassId, file_ptr->GlobalTagId, output_tag_bytes, output_cleanup_ptr, module_ptr->filepath, tag_index);
		loaded_tags.push_back(new_tag);

		// TagToTexture(new_tag);
		return new_tag;
	}
	throw exception("tag with specified tagID was not found in any loaded modules");
}

Module* ModuleManager::GetModule_AtIndex(uint32_t index){
	if (index >= loaded_modules.size())
		throw exception("attempted to fetch module from invalid index");

	return loaded_modules[index];
}
Module* ModuleManager::GetModule_FromTag(Tag* tag) {
    Module* parent_module = 0;
    for (uint32_t c = 0; c < loaded_modules.size(); c++) {
        if (loaded_modules[c]->filepath == tag->source_module) {
            parent_module = loaded_modules[c];
            break;
        }
    }
    return parent_module;
}


HRESULT ModuleManager::OpenTagResource(Tag* tag, uint32_t resource_index, char* resource_out_buffer, uint32_t buffer_size){
	// figure out which module this belongs to
    Module* parent_module = GetModule_FromTag(tag);
	if (parent_module == 0)
		throw exception("tag has no parent module!! was it deallocated?");
	// attempt write resource to buffer
	return parent_module->ReturnResource(tag->source_tag_index, resource_index, resource_out_buffer, buffer_size);
}
bool ModuleManager::IsTagResourceHd1(Tag* tag, uint32_t resource_index) {
    Module* parent_module = GetModule_FromTag(tag);
    if (parent_module == 0)
        throw exception("tag has no parent module!! was it deallocated?");
    auto var = parent_module->ReturnResourceHeader(tag->source_tag_index, resource_index);
    bool test = (var->get_dataflags() & flag2_UseHd1) != 0;
    if (tag->preview_1 == 5) {
        throw exception("poop");
    }
    return test;
}

/*
void BITM_bitmap_count()
{

}
void BITM_bitmap_is_chunked() {

}
//void BITM_bitmap_is; */

bool is_short_header(DXGI_FORMAT format) {
    switch (format) {
    case DXGI_FORMAT_R8G8B8A8_UNORM:
    case DXGI_FORMAT_R16G16_UNORM:
    case DXGI_FORMAT_R8G8_UNORM:
    case DXGI_FORMAT_R16_UNORM:
    case DXGI_FORMAT_R8_UNORM:
    case DXGI_FORMAT_A8_UNORM:
    case DXGI_FORMAT_R8G8_B8G8_UNORM:
    case DXGI_FORMAT_G8R8_G8B8_UNORM:
    case DXGI_FORMAT_BC1_UNORM:
    case DXGI_FORMAT_BC2_UNORM:
    case DXGI_FORMAT_BC3_UNORM:
    case DXGI_FORMAT_BC4_SNORM:
    case DXGI_FORMAT_BC5_SNORM:
    case DXGI_FORMAT_B5G6R5_UNORM:
    case DXGI_FORMAT_B5G5R5A1_UNORM:
    case DXGI_FORMAT_R8G8_SNORM:
    case DXGI_FORMAT_R8G8B8A8_SNORM:
    case DXGI_FORMAT_R16G16_SNORM:
    case DXGI_FORMAT_B8G8R8A8_UNORM:
    case DXGI_FORMAT_B8G8R8X8_UNORM:
    case DXGI_FORMAT_B4G4R4A4_UNORM:
    case DXGI_FORMAT_YUY2:
    case DXGI_FORMAT_R32G32B32A32_FLOAT:
    case DXGI_FORMAT_R16G16B16A16_FLOAT:
    case DXGI_FORMAT_R16G16B16A16_UNORM:
    case DXGI_FORMAT_R16G16B16A16_SNORM:
    case DXGI_FORMAT_R32G32_FLOAT:
    case DXGI_FORMAT_R16G16_FLOAT:
    case DXGI_FORMAT_R32_FLOAT:
    case DXGI_FORMAT_R16_FLOAT:
   /*  // only short header if conversion flag 'DDS_FLAGS_FORCE_DX9_LEGACY' 
    case DXGI_FORMAT_R10G10B10A2_UNORM:
    case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
    case DXGI_FORMAT_BC1_UNORM_SRGB:
    case DXGI_FORMAT_BC2_UNORM_SRGB:
    case DXGI_FORMAT_BC3_UNORM_SRGB:
    case DXGI_FORMAT_BC4_UNORM:
    case DXGI_FORMAT_BC5_UNORM:
    case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
    case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB: */
        return true;
    }
    return false;
}

// index -1 : highest texture resolution
BitmapResource* ModuleManager::BITM_GetTexture(Tag* tag, ID3D11Device* device, int target_resource) {
	if (tag->tag_FourCC != Tag::bitm) // 'bitm'
		return nullptr;



    BitmapGroup* bitm_tag = (BitmapGroup*)(tag->tag_data);
    BitmapData* selected_bitmap = bitm_tag->bitmaps[0];
    BitmapDataResource* bitmap_details = selected_bitmap->bitmap_resource_handle.content_ptr;


    // calculate actual resource index before doing anything
    // so we can makre sure we haven't already created this guy
    const bool has_pixel_data = (bitmap_details->pixels.data_size != 0);
    const bool has_streamble_data = (bitmap_details->streamingData.count != 0);
    // count all resources
    int32_t resource_count = has_pixel_data; // (0 or 1 for init count)
    resource_count += bitmap_details->streamingData.count;

    // if there is no images, then fail
    if (resource_count == 0)
        throw exception("no buffers to source image from");

    // if target index is larger than size, set to highest index
    // or if index is -1, then also selected highest index
    bool is_using_pixel_data = false;
    if (target_resource >= resource_count || target_resource < 0)
        target_resource = resource_count - 1;

    int32_t resource_file_index = target_resource;
    if (has_pixel_data) {
        if (target_resource == 0) // resource 0 means lowest, the data in the buffer is assumedly the lowest quality // however infinite i believe uses that as a buffer, and thus cannot have both streamable & pixel data
            is_using_pixel_data = true;
        resource_file_index--;
    }

    // now check if that resource is valid, if not then go down the list until we find a valid resource.
    // determine whether we are currently loading 
    bool hd1_loaded = false; // PLACEHOLDER // replace with grabbing the module's bool for this


    // only run this for when using resources
    if (!is_using_pixel_data) {
        while (IsTagResourceHd1(tag, resource_file_index)) {
            target_resource--;
            resource_file_index--;

            if (resource_file_index < 0) {
                if (has_pixel_data) break; // this means that all resource files were not compatible, and so we have to resort to our pixel buffer
                else throw exception("all available resources relied on hd1 module, which we do not have loaded. cannot fetch any resources!!!!");
            }
        }
    }

    // rerun criteria, as we may now be relying on pixel data
    if (has_pixel_data && target_resource == 0)
        is_using_pixel_data = true;

    tag->preview_1 = target_resource; // update the value to let the user know whats up

    // now check to see if this resource already exists.
    for (int i = 0; i < tag->resources.Size(); i++) {
        BitmapResource* res = (BitmapResource*)tag->resources[i];
        if (res->resource_index == target_resource) {
            return res;
        }
    }


    // error checking only needs to occur the first time, no point double checking everytime we need to access the index
    if (selected_bitmap->type != BitmapType::_2D_texture)
        throw exception("unsupported image type");
    if (selected_bitmap->bitmap_resource_handle.content_ptr == 0)
        throw exception("image data not present?!!");
    if (selected_bitmap->bitmap_resource_handle.is_chunked_resource == 0)
        throw exception("images with non-chunked/streamable data are not supported!!");


    DirectX::TexMetadata* meta = new DirectX::TexMetadata();

    // configure meta data

    meta->depth = selected_bitmap->depth;
    meta->arraySize = 1; // no support for array types yet
    meta->mipLevels = 0; // it seems this does not apply for all versions // selected_bitmap->mipmap_count;
    // only for extended/compressed dds
    meta->miscFlags = 0; // the only flag seems to be TEX_MISC_TEXTURECUBE = 0x4
    meta->miscFlags2 = 0;
    meta->format = (DXGI_FORMAT)bitmap_details->format;
    meta->dimension = (DirectX::TEX_DIMENSION)3; // TEX_DIMENSION_TEXTURE2D


    size_t header_size = sizeof(uint32_t) + 124; // sizeof(DirectX::DDS_HEADER);


    if (!is_short_header(meta->format)) header_size += 20; // sizeof(DirectX::DDS_HEADER_DXT10);

    size_t image_data_size;


    // figure out if this texture is using internal data, or resource data
    if (is_using_pixel_data) { // use pixel data
        meta->width = selected_bitmap->width;
        meta->height = selected_bitmap->height;
        image_data_size = bitmap_details->pixels.data_size;
    }else { // use streaming data
        if (bitmap_details->streamingData.count == 0)
            throw exception("no streaming data or pixel data");

        if (resource_file_index >= bitmap_details->streamingData.count)
            throw exception("out of bounds index for streaming texture array");

        meta->width = bitmap_details->streamingData[resource_file_index]->dimensions & 0x0000FFFF;
        meta->height = (bitmap_details->streamingData[resource_file_index]->dimensions >> 16) & 0x0000FFFF;
        image_data_size = bitmap_details->streamingData[resource_file_index]->size;
        // NOTE: implement a more advanced system that actually compares sizes of resources to see which one this block refers to
        // for now we're just going to pretend the order of resources aligns with the order of streaming data blocks
        // apparently that number is the opposite of what it should be (bitmap_details->streamingData[index]->chunkInfo >> 16) & 0xFF; // push 2 backs back, and ignore first byte
        // we should also load the image here // negative, no point writing image just yet if it has a chance of failing
    }
    // if its a 1x1 pixel, then clear mip map levels?
    if (meta->width == 1 || meta->height == 1)
        meta->mipLevels = 1; // i dont think this will fix it



    char* DDSheader_dest = new char[header_size + image_data_size];

    size_t output_size = 0;
    HRESULT hr = EncodeDDSHeader(*meta, DirectX::DDS_FLAGS_NONE, (void*)DDSheader_dest, header_size, output_size);
    if (!SUCCEEDED(hr))
        throw exception("image failed to generate DDS header");
    if (header_size != output_size)
        throw exception("header size was incorrectly assumed! must investigate this image format!!!");

    BitmapResource* resource_container = new BitmapResource();
    resource_container->Width = meta->width;
    resource_container->Height = meta->height;
    resource_container->resource_index = target_resource;

    // i believe we delete meta? // as it should be copied to the DDS header
    delete meta;

    // then write the bitmap data
    if (is_using_pixel_data)  // non-resource pixel array
        memcpy(DDSheader_dest+header_size, bitmap_details->pixels.content_ptr, image_data_size);
    else if (FAILED(OpenTagResource(tag, resource_file_index, DDSheader_dest + header_size, image_data_size))) { // chunked resource
        throw exception("this cannot happen yet, must debug!!");
        // then cleanup & return stub tag details, just so it doesn't continue to attempt to load the tag over and over again if it ever fails
        delete[] DDSheader_dest;
        tag->resources.Append(resource_container);
        return resource_container;
    }
    



    // and then we should have a fully loaded dds image in mem, which we should beable to now export for testing purposes
    hr = DirectX::LoadFromDDSMemory(DDSheader_dest, header_size + image_data_size, (DirectX::DDS_FLAGS)0, nullptr, resource_container->scratch_image);
    if (FAILED(hr))
        throw exception("failed to load DDS from memory");
    //const wchar_t* export_file_path = L"C:\\Users\\Joe bingle\\Downloads\\test\\test.dds";
    //hr = DirectX::SaveToDDSFile(*DDS_image->GetImage(0,0,0), (DirectX::DDS_FLAGS)0, export_file_path);
    //if (FAILED(hr))
    //    throw exception("failed to save DDS to local file");
    //Image& image, _In_ DDS_FLAGS flags, _In_z_ const wchar_t*
    //hr = DirectX::SaveToWICFile(*DDS_image->GetImage(0, 0, 0), (DirectX::DDS_FLAGS)0, export_file_path);

    hr = DirectX::CreateShaderResourceView(device, resource_container->scratch_image.GetImages(), resource_container->scratch_image.GetImageCount(), resource_container->scratch_image.GetMetadata(), &resource_container->image_view);
    if (FAILED(hr))
        throw exception("failed to get shader view from texture");

    tag->resources.Append(resource_container);

    return resource_container;
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




