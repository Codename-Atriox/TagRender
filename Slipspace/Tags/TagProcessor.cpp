#include "TagProcessor.h"




ModuleManager::ModuleManager()
{
	unpacker = new Oodle();
}

void ModuleManager::OpenModule(string filename){
	CloseModule(filename); // if this module is already open, reopen it

	try{
		Module* new_module = new Module(filename, unpacker);
		loaded_modules->push_back(new_module);
		open_modules++;
		total_tags += new_module->file_count;
	} catch (exception ex) {
		throw exception("failed to open module");
	}
}

void ModuleManager::CloseModule(string filename){
	/*
	for (int c = 0; c < loaded_modules->size(); c++) {
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
	for (int i = 0; i < loaded_tags->size(); i++)
		if ((*loaded_tags)[i]->tagID == tagID)
			return (*loaded_tags)[i];
	return (Tag*)0;
}
Tag* ModuleManager::OpenTag(uint32_t tagID){
	// first check if the tag already exists
	Tag* new_tag = GetTag(tagID);
	if (new_tag != (Tag*)0) return new_tag;

	for (int c = 0; c < loaded_modules->size(); c++) {
		Module* module_ptr = (*loaded_modules)[c];

		int32_t tag_index = module_ptr->find_tag_index(tagID);
		if (tag_index == -1) continue; // this module does not contain the tag

		module_file* file_ptr = module_ptr->GetTagHeader_AtIndex(tag_index);

		char* output_tag_bytes;
		char* output_cleanup_ptr;
		module_ptr->GetTagProcessed(file_ptr, output_tag_bytes, output_cleanup_ptr);

		Tag* new_tag = new Tag(std::string("UNIMPLEMENTED"), file_ptr->ClassId, file_ptr->GlobalTagId, output_tag_bytes, output_cleanup_ptr, module_ptr->filepath, tag_index);
		loaded_tags->push_back(new_tag);

		// TagToTexture(new_tag);
		return new_tag;
	}
	throw exception("tag with specified tagID was not found in any loaded modules");
}

Module* ModuleManager::GetModule_AtIndex(uint32_t index){
	if (index >= loaded_modules->size())
		throw exception("attempted to fetch module from invalid index");

	return (*loaded_modules)[index];
}

void ModuleManager::OpenTagResource(Tag* tag, uint32_t resource_index, char* resource_out_buffer, uint32_t buffer_size){
	// figure out which module this belongs to
	Module* parent_module = 0;
	for (uint32_t c = 0; c < loaded_modules->size(); c++) {
		if ((*loaded_modules)[c]->filepath == tag->source_module){
			parent_module = (*loaded_modules)[c];
			break;
	}}
	if (parent_module == 0)
		throw exception("tag has no parent module!! was it deallocated?");
	// attempt write resource to buffer
	parent_module->ReturnResource(tag->source_tag_index, resource_index, resource_out_buffer, buffer_size);
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


ID3D11ShaderResourceView* ModuleManager::BITM_GetTexture(Tag* tag, ID3D11Device* device) {
	if (tag->tag_FourCC != 0x6269746D) // 'bitm'
		return nullptr;

    // DEBUG TEMP CODE //
    // check to see if this tag already has an image loaded
    if (tag->resources.Size() > 0)
        return (ID3D11ShaderResourceView*)tag->resources[0];


    // pseudo select-best-image code here
    // aka select the first one, because if theres more, then why???
    BitmapGroup* bitm_tag = (BitmapGroup*)(tag->tag_data);
    BitmapData* selected_bitmap = bitm_tag->bitmaps[0];

    if (selected_bitmap->type != BitmapType::_2D_texture)
        throw exception("unsupported image type");
    if (selected_bitmap->bitmap_resource_handle.content_ptr == 0)
        throw exception("image data not present?!!");
    if (selected_bitmap->bitmap_resource_handle.is_chunked_resource == 0)
        throw exception("images with non-chunked/streamable data are not supported!!");

    BitmapDataResource* bitmap_details = selected_bitmap->bitmap_resource_handle.content_ptr;

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
    uint8_t resource_index;
    // figure out if this texture is using internal data, or resource data
    const bool is_using_pixel_data = (bitmap_details->pixels.data_size != 0);
    if (is_using_pixel_data) { // use pixel data
        meta->width = selected_bitmap->width;
        meta->height = selected_bitmap->height;
        image_data_size = bitmap_details->pixels.data_size;
    }else { // use streaming data
        if (bitmap_details->streamingData.count == 0)
            throw exception("no streaming data or pixel data");

        uint32_t index = 0; // probably the lowest quality // select the image that we want to be using
        if (index >= bitmap_details->streamingData.count)
            throw exception("out of bounds index for streaming texture array");

        meta->width = bitmap_details->streamingData[index]->dimensions & 0x0000FFFF;
        meta->height = (bitmap_details->streamingData[index]->dimensions >> 16) & 0x0000FFFF;
        image_data_size = bitmap_details->streamingData[index]->size;
        // NOTE: implement a more advanced system that actually compares sizes of resources to see which one this block refers to
        // for now we're just going to pretend the order of resources aligns with the order of streaming data blocks
        resource_index = index; // apparently that number is the opposite of what it should be (bitmap_details->streamingData[index]->chunkInfo >> 16) & 0xFF; // push 2 backs back, and ignore first byte
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
        // DXGI_FORMAT_BC1_UNORM_SRGB (72) is actually long header? // TODO??
        throw exception("header size was incorrectly assumed! must investigate this image format!!!");

    // then write the bitmap data
    if (is_using_pixel_data)  // non-resource pixel array
        memcpy(DDSheader_dest+header_size, bitmap_details->pixels.content_ptr, image_data_size);
    else // chunked resource  // an interesting problem arises, chunk files are aligned to a 0x?000 bytes in size // so the size is wrong
        OpenTagResource(tag, resource_index, DDSheader_dest+header_size, image_data_size);
    

    // and then we should have a fully loaded dds image in mem, which we should beable to now export for testing purposes
    DirectX::ScratchImage* DDS_image = new DirectX::ScratchImage();
    hr = DirectX::LoadFromDDSMemory(DDSheader_dest, header_size + image_data_size,  (DirectX::DDS_FLAGS)0, nullptr, *DDS_image);
    if (FAILED(hr))
        throw exception("failed to load DDS from memory");
    const wchar_t* export_file_path = L"C:\\Users\\Joe bingle\\Downloads\\test\\test.dds";
    hr = DirectX::SaveToDDSFile(*DDS_image->GetImage(0,0,0), (DirectX::DDS_FLAGS)0, export_file_path);
    if (FAILED(hr))
        throw exception("failed to save DDS to local file");
    //Image& image, _In_ DDS_FLAGS flags, _In_z_ const wchar_t*
    //hr = DirectX::SaveToWICFile(*DDS_image->GetImage(0, 0, 0), (DirectX::DDS_FLAGS)0, export_file_path);

    ID3D11ShaderResourceView* image_resource = nullptr;
    hr = DirectX::CreateShaderResourceView(device, DDS_image->GetImages(), DDS_image->GetImageCount(), DDS_image->GetMetadata(), &image_resource);
    if (FAILED(hr))
        throw exception("failed to get shader view from texture");

    tag->resources.Append(image_resource);

    return image_resource;
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




