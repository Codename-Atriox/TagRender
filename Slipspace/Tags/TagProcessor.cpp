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

        //int32_t tag_index = new_module->find_tag_index(0xFC3EA86A);
        //if (tag_index == -1) 
        //    throw exception("tag not found"); // this module does not contain the tag
        //return;
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
Tag* ModuleManager::OpenTag(uint32_t tagID, ID3D11Device* device){
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
        if (new_tag->tagname == "") {
            new_tag->tagname = "name failed";
        }
		loaded_tags.push_back(new_tag);

        // then do any processing required for the tag
        // and for every tag, we're going to use that anytag blank slot to point to our tag header structure thing // so then we can use it for things like dependency arrays etc
        ((rtgo::AnyTag_struct_definition*)new_tag->tag_data)->vtable_space = (long)new_tag; // cursed
        switch (new_tag->tag_FourCC) {
        case Tag::rtgo:
            RTGO_loadbuffers(new_tag, device);
            break;
        }

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
    return (parent_module->ReturnResourceHeader(tag->source_tag_index, resource_index)->get_dataflags() & flag2_UseHd1) != 0;
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

BitmapResource* ModuleManager::BITM_GetTexture(Tag* tag, ID3D11Device* device, int32_t target_resource, bool load_next_best) {
	if (tag->tag_FourCC != Tag::bitm) // 'bitm'
		return nullptr;

    // TODO: OPTIMIZE THIS, we shouldn't have to compute whether a resource index is hd1 or not everytime
    // maybe if it is hd1, create a dummy resource with the hd1 bool set & then go through list until we find a non-hd1 asset?

    BitmapGroup* bitm_tag = (BitmapGroup*)(tag->tag_data);

    bool multi_mode = (bitm_tag->bitmaps.count > 1);

    BitmapData* selected_bitmap = nullptr;
    if (multi_mode){
        if (target_resource >= bitm_tag->bitmaps.count)
            throw exception("bad multi image mode resource target index (overflowed)!!!");
        load_next_best = false; // this disables certain code that should never run when multimode
        selected_bitmap = bitm_tag->bitmaps[target_resource]; // multi-mode means our index is to the bitmap, not 
    }else selected_bitmap = bitm_tag->bitmaps[0];
        
    BitmapDataResource* bitmap_details = selected_bitmap->bitmap_resource_handle.content_ptr;


    // calculate actual resource index before doing anything
    // so we can makre sure we haven't already created this guy
    const bool has_pixel_data = (bitmap_details->pixels.data_size != 0);
    const bool has_streamble_data = (bitmap_details->streamingData.count != 0);

    // count all resources, if resources are chunks
    bool is_using_pixel_data = false;
    int32_t resource_file_index = target_resource;
    if (!multi_mode) {
        int32_t resource_count = has_pixel_data; // (0 or 1 for init count)
        resource_count += bitmap_details->streamingData.count;

        // if there is no images, then fail
        if (resource_count == 0)
            throw exception("no buffers to source image from");

        // if target index is larger than size, set to highest index
        // or if index is -1, then also selected highest index
        if (target_resource >= resource_count || target_resource < 0)
            target_resource = resource_count - 1;

        if (has_pixel_data) {
            if (target_resource == 0) // resource 0 means lowest, the data in the buffer is assumedly the lowest quality // however infinite i believe uses that as a buffer, and thus cannot have both streamable & pixel data
                is_using_pixel_data = true;
            resource_file_index--;
        }
    }
    else is_using_pixel_data = true; // im pretty sure its not possible for multi mode to have another other than single mips



    // now check if that resource is valid, if not then go down the list until we find a valid resource.
    // determine whether we are currently loading 
    bool hd1_loaded = false; // PLACEHOLDER // replace with grabbing the module's bool for this


    // only run this for when using resources
    // iterate through all resources until 
    //if (!is_using_pixel_data) {
    //    while (IsTagResourceHd1(tag, resource_file_index)) {
    //        target_resource--;
    //        resource_file_index--;
    //        if (resource_file_index < 0) {
    //            if (has_pixel_data) break; // this means that all resource files were not compatible, and so we have to resort to our pixel buffer
    //            else throw exception("all available resources relied on hd1 module, which we do not have loaded. cannot fetch any resources!!!!");
    //        }
    //    }
    //}




    // now check to see if this resource already exists.
    // if it does & the asset is hd1 & we haven't loaded hd1 modules, then we have to find the next best
    if (!is_using_pixel_data && load_next_best && !hd1_loaded) {
        BitmapResource* next_best = 0;
        int32_t next_best_nonhd1 = -1;
        int32_t lowest_hd1_asset_index = target_resource + 1; // assuming hd1's are indexed always after non-hd1 assets

        for (int i = 0; i < tag->resources.Size(); i++) {
            BitmapResource* res = (BitmapResource*)tag->resources[i];
            if (res->hd1) {
                if (res->resource_index < lowest_hd1_asset_index)
                    lowest_hd1_asset_index = res->resource_index;
            } else if (res->resource_index <= target_resource && res->resource_index > next_best_nonhd1) {
                next_best_nonhd1 = res->resource_index;
                next_best = res;
        }}

        // if our next index is not the index below the lowest recorded hd1 asset index, then we clearly have not loaded it yet
        if (next_best_nonhd1 < lowest_hd1_asset_index - 1) {
            target_resource = lowest_hd1_asset_index - 1; // basically, just load the next asset after the lowest hd1, until we've loaded an asset that isn't hd1
            resource_file_index = target_resource - has_pixel_data;
            if (target_resource < 0) throw exception("all assets marked as hd1!!! no assets to fallback to!");
        } else { // we found the next best resource index already
            if (next_best == 0) throw exception("epic code logic fail! this is impossible");
            return next_best;

    }} else for (int i = 0; i < tag->resources.Size(); i++) { // we just grab whichever index we need, regardless of whether its hd1 or not
        BitmapResource* res = (BitmapResource*)tag->resources[i];
        if (res->resource_index == target_resource) return res;
    }
    
    // rerun criteria, as we may now be relying on pixel data
    if (has_pixel_data && target_resource == 0)
        is_using_pixel_data = true;
    
    


    // error checking only needs to occur the first time, no point double checking everytime we need to access the index
    if (selected_bitmap->type != BitmapType::_2D_texture)
        throw exception("unsupported image type");
    if (selected_bitmap->bitmap_resource_handle.content_ptr == 0)
        throw exception("image data not present?!!");

    // make sure multi/single modes have according resource types
    if (multi_mode && selected_bitmap->bitmap_resource_handle.is_chunked_resource != 0)
        throw exception("multi mode bitmaps must use non-chunked resources!! (it would be impossible for them to be set otherwise)");
    if (!multi_mode && selected_bitmap->bitmap_resource_handle.is_chunked_resource == 0)
        throw exception("bitmaps with single image should be in chunk mode!! must investigate!!");


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

    BitmapResource* resource_container = new BitmapResource();
    resource_container->Width = meta->width;
    resource_container->Height = meta->height;
    resource_container->resource_index = target_resource;
    if (is_using_pixel_data) resource_container->hd1 = false; // im pretty sure it cant be possible for buffer data to be in hd1, as it has to exist within the main tag's blocks
    else                     resource_container->hd1 = IsTagResourceHd1(tag, resource_file_index);

    // if this was marked as hd1 & we dont have hd1 loaded, then blank this & next time it will load a lower index asset
    if (resource_container->hd1 && !hd1_loaded) {
        tag->resources.Append(resource_container);
        delete meta;
        return resource_container;
    }

    char* DDSheader_dest = new char[header_size + image_data_size];

    size_t output_size = 0;
    HRESULT hr = EncodeDDSHeader(*meta, DirectX::DDS_FLAGS_NONE, (void*)DDSheader_dest, header_size, output_size);
    if (!SUCCEEDED(hr))
        throw exception("image failed to generate DDS header");
    if (header_size != output_size)
        throw exception("header size was incorrectly assumed! must investigate this image format!!!");

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




void ModuleManager::RTGO_loadbuffers(Tag* tag, ID3D11Device* device) {
    if (tag->tag_FourCC != Tag::rtgo) // 'bitm'
        return;
    // we need to load all resources into single char buffer, we do not need to store size as all links to buffer are created here
    // nothing will directly access the pointer, except for cleaing up, which we'll do in a custom struct destructor

    rtgo::RuntimeGeoTag* runtime_geo = (rtgo::RuntimeGeoTag*)tag->tag_data;

    // no support for multiple 'mesh reosurce groups' yet, throw exception
    if (runtime_geo->render_geometry.mesh_package.mesh_resource_groups.count != 1)
        throw exception("bad number of resource groups in runtime geo");

    rtgo::RenderGeometryMeshPackageResourceGroup* current_runtime_geo_group = runtime_geo->render_geometry.mesh_package.mesh_resource_groups[0];
    // double check to make sure the file exists as expected, we dont know how non-chunked models work yet
    if (current_runtime_geo_group->mesh_resource.is_chunked_resource == 0)
        throw exception("non-chunked geo resources are not yet supported!!!");
    
    rtgo::s_render_geometry_api_resource* current_geo_resource = current_runtime_geo_group->mesh_resource.content_ptr;

    // first we have to iterate through all resources & write their contents to the buffers
    // all buffer datas will go into the one char buffer, which we then give the reference to the thing

    // find total buffer size
    uint64_t buffer_size = 0;

    // cache offsets of all buffers
    if (current_geo_resource->Streaming_Buffers.count == 0) throw exception("no streaming buffers to pull from!!!");
    uint64_t* offsets = new uint64_t[current_geo_resource->Streaming_Buffers.count];

    for (int i = 0; i < current_geo_resource->Streaming_Buffers.count; i++) {
        offsets[i] = buffer_size;
        buffer_size += current_geo_resource->Streaming_Buffers[i]->buffer_size;
    }
    char* streaming_buffer = new char[buffer_size];

    for (uint32_t i = 0; i < current_geo_resource->Streaming_Chunks.count; i++) {
        // loop through all buffers
        rtgo::StreamingGeometryChunk* curr_buffer_chunk = current_geo_resource->Streaming_Chunks[i];
        uint64_t buffer_offset = offsets[curr_buffer_chunk->buffer_index];
        if (curr_buffer_chunk->buffer_start > curr_buffer_chunk->buffer_end)
            throw exception("bad buffer indicies");
        if (curr_buffer_chunk->buffer_end > buffer_size)
            throw exception("buffer end index overflows buffer size!!");
        if (curr_buffer_chunk->buffer_start == curr_buffer_chunk->buffer_end)
            break; // using a break instead because if any other buffer beyonds this one does have information, then this would be error prone
            //continue; // this buffer has no content, thus likely no resource file

        // then just dump the data in
        if (FAILED(OpenTagResource(tag, i, streaming_buffer + (buffer_offset + curr_buffer_chunk->buffer_start), curr_buffer_chunk->buffer_end - curr_buffer_chunk->buffer_start)))
            throw exception("resource chunk failed to load");
    }
    delete[] offsets;
    // leave a pointer to the buffer array in the runtime data slot, so it can be cleaned up later
    current_geo_resource->Runtime_Data = (uint64_t)streaming_buffer;

    // now we load to fill in the buffers for each of the d3d11 buffer things
    // which ironically all require us to create new d3dbuffers, because appanrently we couldn't just build them into the structs

    // iterate through buffers and generate D3D11 buffers
    for (uint32_t i = 0; i < current_geo_resource->pc_vertex_buffers.count; i++) {
        rtgo::RasterizerVertexBuffer* vert_buffer = current_geo_resource->pc_vertex_buffers[i];

        D3D11_BUFFER_DESC vertexBufferDesc;
        ZeroMemory(&vertexBufferDesc, sizeof(vertexBufferDesc)); // do we even need to do this?

        vertexBufferDesc.Usage = D3D11_USAGE::D3D11_USAGE_DEFAULT; // apparently 343's mapped value is wrong (D3D11_USAGE)vert_buffer->d3dbuffer.usage; // 0
        vertexBufferDesc.ByteWidth = vert_buffer->d3dbuffer.byte_width; // 144
        vertexBufferDesc.BindFlags = vert_buffer->d3dbuffer.bind_flags; // 1 'vertex buffer'
        vertexBufferDesc.CPUAccessFlags = vert_buffer->d3dbuffer.cpu_flags; // 0
        vertexBufferDesc.MiscFlags = 0; // vert_buffer->d3dbuffer.misc_flags; // 32 // this causes issues for the first 2 buffers

        D3D11_SUBRESOURCE_DATA vertexBufferData;
        ZeroMemory(&vertexBufferData, sizeof(vertexBufferData));
        if (vert_buffer->d3dbuffer.d3d_buffer.data_size != 0)
            throw exception("render geo with non-chunked data is not currently supported!! because i have no idea what could be contained in this buffer!!");
        vertexBufferData.pSysMem = streaming_buffer + vert_buffer->offset;

        ID3D11Buffer* result;
        HRESULT hr = device->CreateBuffer(&vertexBufferDesc, &vertexBufferData, &result);
        vert_buffer->m_resource = (int64_t)result;
        if (FAILED(hr)) 
            throw exception("failed to generate d3d11 buffer!!");
    }
    // iterate through buffers and generate D3D11 buffers
    for (uint32_t i = 0; i < current_geo_resource->pc_index_buffers.count; i++) {
        rtgo::RasterizerIndexBuffer* index_buffer = current_geo_resource->pc_index_buffers[i];

        D3D11_BUFFER_DESC indexBufferDesc;
        ZeroMemory(&indexBufferDesc, sizeof(indexBufferDesc)); // do we even need to do this?

        indexBufferDesc.Usage = D3D11_USAGE::D3D11_USAGE_DEFAULT; // not sure why 343's is some weird thing, i think its literally just the enum but as a number   (D3D11_USAGE)index_buffer->d3dbuffer.usage;
        indexBufferDesc.ByteWidth = index_buffer->d3dbuffer.byte_width;
        indexBufferDesc.BindFlags = index_buffer->d3dbuffer.bind_flags;
        indexBufferDesc.CPUAccessFlags = index_buffer->d3dbuffer.cpu_flags;
        indexBufferDesc.MiscFlags = 0; // index_buffer->d3dbuffer.misc_flags;

        D3D11_SUBRESOURCE_DATA vertexBufferData;
        ZeroMemory(&vertexBufferData, sizeof(vertexBufferData));
        if (index_buffer->d3dbuffer.d3d_buffer.data_size != 0)
            throw exception("render geo with non-chunked data is not currently supported!! because i have no idea what could be contained in this buffer!!");
        vertexBufferData.pSysMem = streaming_buffer + index_buffer->offset;

        ID3D11Buffer* result = nullptr;
        HRESULT hr = device->CreateBuffer(&indexBufferDesc, &vertexBufferData, &result);
        index_buffer->m_resource = (int64_t)result;
        if (FAILED(hr)) 
            throw exception("failed to generate d3d11 buffer!!");
    }
    // thats all, we dont have anything to return, because we just wrote everything to the tag
    // we can now render this data via the interfaces we just created & referencing them using the meshes tagdata struct
}
