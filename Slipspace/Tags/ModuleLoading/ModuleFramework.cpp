//#include "ModuleFramework.h"
// trigger warning: WARCRIME // idk how this 
#include "../TagProcessor.h" // i hope whoever wrote this language is happy with themselves, this is probably the worst language ever holy moly

module_file* Module::find_tag(uint32_t tagID) {
    for (int i = 0; i < header->FileCount; i++) {
        module_file* curr_file = &(files[i]);
        if (curr_file->GlobalTagId == tagID)
            return curr_file;
    }
    return (module_file*)0;
}
int32_t Module::find_tag_index(uint32_t tagID) {
    for (int32_t i = 0; i < header->FileCount; i++) {
        module_file* curr_file = &(files[i]);
        if (curr_file->GlobalTagId == tagID)
            return i;
    }
    return -1;
}

void Module::GetTagProcessed(uint32_t tagID, char*& output_tag_bytes, char*& output_cleanup_ptr, void* modules) {
    module_file* file_ptr = find_tag(tagID);
    if (file_ptr == 0)
        throw exception("failed to find tag in module");
    GetTagProcessed(file_ptr, output_tag_bytes, output_cleanup_ptr, modules);
}
void Module::GetTagProcessed(module_file* file_ptr, char*& output_tag_bytes, char*& output_cleanup_ptr, void* modules) {
    // read tag from module (ifstream)
    char* raw_tag_bytes = new char[file_ptr->TotalUncompressedSize];
    if (FAILED(GetTagRaw(file_ptr, raw_tag_bytes)))
        throw exception("get tag failed, not allowed!!");
    // process tag to make it usable

	Processtag(raw_tag_bytes, file_ptr, output_tag_bytes, output_cleanup_ptr, modules);
	delete[] raw_tag_bytes; // cleanup the file read request
}
module_file* Module::ReturnResourceHeader(uint32_t tag_index, uint32_t index) {
	return ReturnResourceHeader(GetTagHeader_AtIndex(tag_index), index);
}
module_file* Module::ReturnResourceHeader(module_file* tag_header, uint32_t index) {
    if (index >= tag_header->ResourceCount)
        throw exception("Attemtped to access out of bounds tag resource index");

    uint32_t resource_index = tag_header->ResourceIndex + index;
    if (resource_index >= header->ResourceCount)
        throw exception("Attemtped to access out of bounds module file resource index");

    // get the resources
    module_file* resource_tag = GetTagHeader_AtIndex(resource_indexes[resource_index]);
    if (resource_tag->GlobalTagId != -1 || resource_tag->ClassId != -1)
        throw exception("indexed resource file is not a resource file");

    //if ((resource_tag->Flags & flag_UseRawfile) == 0) // this is now compatible. hell yeah
    //    throw exception("resource files with tagdata content are not currently supported");

    return resource_tag;
}

HRESULT Module::ReturnResource(uint32_t tag_index, uint32_t index, char* output_buffer, uint32_t output_size){
    module_file* resource_header = ReturnResourceHeader(tag_index, index);
    if (resource_header->TotalUncompressedSize > output_size)
        throw exception("not enough room allocated to fit indexed resource");
    return GetTagRaw(resource_header, output_buffer);
}

module_file* Module::GetTagHeader_AtIndex(uint32_t index){
    if (index >= file_count)
        throw exception("attempted to fetch module from invalid index");

    return &files[index];
}
// output bytes NEEDS to already be allocated
HRESULT Module::GetTagRaw(module_file* file_ptr, char* output_bytes) {
    if (output_bytes == 0)
        throw exception("tag output buffer was not preallocated");
    // then begin the read
    // we have to map all the data to read, based on the owned datablocks

    // read the flags to determine how to process this file
    bool using_compression       = (file_ptr->Flags & flag_UseCompression) != 0; // pretty sure this is true if reading_seperate_blocks is also true, confirmation needed
    bool reading_separate_blocks = (file_ptr->Flags & flag_UseBlocks) != 0;
    bool reading_raw_file        = (file_ptr->Flags & flag_UseRawfile) != 0;

    // check if this is a hd1 file?
    bool is_hd1 = (file_ptr->get_dataflags() & flag2_UseHd1) != 0;
    if (is_hd1) { // we're just going to throw an exception for now, as we'll create another function to check whether its a hd file or not
        throw exception("dobule check before-hand that this is not a hd1 resource!!!");
        return E_FAIL;
    }


    if (file_ptr->TotalUncompressedSize == 0)
        throw exception("module file was empty");

    // god dammit, mf 'long' how about you take a long trip down to the bottom of the ocean
    // please someone for the love of god make the long 64 bits, and not just literally an int
    uint64_t data_Address = module_metadata_size + file_ptr->get_dataoffset();

    if (reading_separate_blocks) {
        for (int i = 0; i < file_ptr->BlockCount; i++) {
            block_header bloc = blocks[file_ptr->BlockIndex + i];

            if (bloc.Compressed == 1) {
                module_reader.seekg(data_Address + bloc.CompressedOffset, ios::beg);

                char* raw_bytes = new char[bloc.CompressedSize];
                module_reader.read(raw_bytes, bloc.CompressedSize);

                try{unpacker->decompress(output_bytes + bloc.UncompressedOffset, bloc.UncompressedSize, raw_bytes, bloc.CompressedSize);
                }catch(exception ex){
                    delete[] raw_bytes;
                    delete[] output_bytes;
                    throw ex;
                }
                delete[] raw_bytes;
            }else{ // uncompressed
                module_reader.seekg(data_Address + bloc.UncompressedOffset, ios::beg);
                module_reader.read(output_bytes + bloc.UncompressedOffset, bloc.UncompressedSize);
            }
        }
    }else{  // is the manifest thingo, aka raw file, read data based off compressed and uncompressed length
        module_reader.seekg(data_Address, ios::beg);
        if (using_compression) {
            char* raw_bytes = new char[file_ptr->TotalCompressedSize];
            module_reader.read(raw_bytes, file_ptr->TotalCompressedSize);

            try{unpacker->decompress(output_bytes, file_ptr->TotalUncompressedSize, raw_bytes, file_ptr->TotalCompressedSize);
            }catch (exception ex) {
                delete[] raw_bytes;
                // delete[] output_bytes; // NOTE: THIS HAS TO BE CLEANED UP MANUALLY // what does that mean
                throw ex;
            }
            delete[] raw_bytes;
        }else 
            module_reader.read(output_bytes, file_ptr->TotalUncompressedSize);
    }
    return S_OK;
}

Module::Module(string path, Oodle* oodler) {
    // configure oodle
    unpacker = oodler;
    filepath = path;
    filename = StringHelper::GetFileNameFromPath(path);

    // open module file
    module_reader.open(path, ios::binary | ios::ate);
    if (!module_reader.is_open()) {
        throw exception("failed to open filestream");
    }
    streamsize file_size = module_reader.tellg();
    if (file_size < module_header_size) {
        throw exception("filestream too small");
    }
    module_reader.seekg(0, ios::beg);

    // tempting to realign all this into a single byte array (like how it is in the file), however that would only impact performance for little benefit

    // read the header partition
    header = new module_header();
    module_reader.read((char*)header, module_header_size);

    // module file verification
    if (header->Head != 0x64686F6D)  // 'mohd', but backwards
        throw exception("target file does not appear to be module file");
    if (header->Version != target_module_version) 
        throw exception("module version does not match target version");
    
    // read file partition
    files = new module_file[header->FileCount];
    module_reader.read((char*)files, module_file_size * header->FileCount);

    // read resource indicies partition
    resource_indexes = new uint32_t[header->ResourceCount];
    module_reader.read((char*)resource_indexes, 4 * header->ResourceCount);

    // read blocks partition
    blocks = new block_header[header->BlockCount];
    module_reader.read((char*)blocks, block_header_size * header->BlockCount);

    uint32_t file_offset = module_header_size + (module_file_size * header->FileCount) + (4 * header->ResourceCount) + (block_header_size * header->BlockCount);
    module_metadata_size = (file_offset / 0x1000 + 1) * 0x1000; // for some reason 343 aligns the metadata by 0x1000
    
    file_count = header->FileCount;
}
Module::~Module() {
    if (module_reader.is_open())
        module_reader.close();
}

// //////////////////// //
// TAG FRAMEWORK STUFF //
// ////////////////// //

uint64_t Module::resolve_datablock_offset(data_block* datar, tag_loading_offsets* offsets) {
	if (datar->Section == 0) return datar->Offset - offsets->header_size; // the offsets are directly mapped to the header block, we need to detract the header since we drop it at runtime
	else if (datar->Section == 1) return offsets->data_1_offset + datar->Offset;
	else if (datar->Section == 2) return offsets->data_2_offset + datar->Offset;
	else if (datar->Section == 3) return offsets->data_3_offset + datar->Offset;
	else return -1;
}

/*TAG_OBJ_TYPE Open_standalone_tag(string _In_tag_path, runtime_tag*& _Out_tag) {
	ifstream file_stream(_In_tag_path, ios::binary | ios::ate);
	if (!file_stream.is_open()) {
		return FAILED_OPENING;
	}
	std::streamsize file_size = file_stream.tellg();
	if (file_size < tag_header_size) {
		return FAILED_SIZE;
	}
	// read the whole file
	char* tag_bytes = new char[file_size];
	file_stream.seekg(0, ios::beg);
	file_stream.read(tag_bytes, file_size);
	file_stream.close();

	// we now need to look for resource files, if its a chunk, add it to the out array
	// if its a tag struct file, add it to the inputs
	vector<size_pointer>* chunk_resources = new vector<size_pointer>();
	vector<size_pointer>* struct_resources = new vector<size_pointer>(); // NOTE: this pointer gets deleted when read, and replaced with a new one to a smaller block
	// convert path into parent directory
	const size_t last_slash_idx = _In_tag_path.rfind('\\');
	if (std::string::npos != last_slash_idx) _In_tag_path = _In_tag_path.substr(0, last_slash_idx);

	for (const auto& entry : fs::directory_iterator(_In_tag_path)){
		string current_file = entry.path().string();
		if (_In_tag_path == current_file.substr(0, _In_tag_path.length()) && current_file[_In_tag_path.length()] == '['){
			ifstream resource_stream(_In_tag_path, ios::binary | ios::ate);
			if (!resource_stream.is_open()) {
				continue;
			}
			std::streamsize resource_size = resource_stream.tellg();
			if (resource_size == 0) {
				continue;
			}
			// read the whole file
			char* resource_bytes = new char[resource_size];
			resource_stream.seekg(0, ios::beg);
			resource_stream.read(resource_bytes, resource_size);
			resource_stream.close();

			size_pointer* newthinger = new size_pointer(resource_bytes, resource_size);
			if (current_file.substr(0, _In_tag_path.length()).find(".chunk") != std::string::npos) // chunked resource
				chunk_resources->push_back(*newthinger);
			else struct_resources->push_back(*newthinger); // regular resource
	}}

	void* tag_data;
	char* cleanup_ptr;
	TAG_OBJ_TYPE output = Processtag(tag_bytes, file_size, tag_data, struct_resources, cleanup_ptr);
	// then create
	_Out_tag = new runtime_tag(tag_data, chunk_resources, struct_resources, cleanup_ptr);
	return output; // we only input non chunk resources
} */
void Module::Processtag(char* tag_bytes, module_file* file_header, char*& _Out_data, char*& _Out_cleanup_ptr, void* modules) {
	uint64_t current_byte_offset = 0;
	uint32_t current_resource_index = 0;

	// read header from bytes
	tag_header* header = (tag_header*)tag_bytes;
	current_byte_offset += tag_header_size;
	if (header->Magic != 1752392565) { // 'hscu'
		throw exception("Incorrect 'Magic'! potential wrong file type!");
	}

	tag_loading_offsets* offsets = new tag_loading_offsets;
	// index each significant block
	offsets->tag_dependencies_offset = current_byte_offset;
	current_byte_offset += static_cast<uint64_t>(header->DependencyCount) * tag_dependency_size;

	offsets->data_blocks_offset = current_byte_offset;
	current_byte_offset += static_cast<uint64_t>(header->DataBlockCount) * data_block_size;

	offsets->tag_structs_offset = current_byte_offset;
	current_byte_offset += static_cast<uint64_t>(header->TagStructCount) * tag_def_structure_size;

	offsets->data_references_offset = current_byte_offset;
	current_byte_offset += static_cast<uint64_t>(header->DataReferenceCount) * data_reference_size;

	offsets->tag_fixup_references_offset = current_byte_offset;
	current_byte_offset += static_cast<uint64_t>(header->TagReferenceCount) * tag_fixup_reference_size;

	offsets->string_table_offset = current_byte_offset;
	// idk why this isn't commented out yet, but why do some tags still have this value set. thanks john halo
	//current_byte_offset += header->StringTableSize;

	offsets->zoneset_info_offset = current_byte_offset;
	current_byte_offset += header->ZoneSetDataSize; // unsure if this includes the header or just all the elements
	// current offset now marks the entirety of the header, so we can create a new array excluding (current_offset) bytes
	offsets->header_size = file_header->TotalUncompressedSize - current_byte_offset;

	char* runtime_bytes = new char[file_header->TotalUncompressedSize - current_byte_offset];
	std::copy(tag_bytes + current_byte_offset, tag_bytes + file_header->TotalUncompressedSize, runtime_bytes);

	// ok now we need to get the offsets for the 3 extra blocks (or was it two?) although these can be calculated inplace rather easily as they're only one long + another

	offsets->data_1_offset = header->HeaderSize - current_byte_offset;
	offsets->data_2_offset = offsets->data_1_offset + header->DataSize;
	offsets->data_3_offset = offsets->data_2_offset + header->ResourceDataSize;
	// failsafe see if the numbers are HUGE (they should not be HUGE)
	if ((int64_t)offsets->data_1_offset < 0 || (int64_t)offsets->data_2_offset < 0 || (int64_t)offsets->data_3_offset < 0)
		throw exception("very bad offsets detected!!!");

	// now we note any unmapped data segments in the file

	if (offsets->data_3_offset + current_byte_offset + header->ActualResoureDataSize != file_header->TotalUncompressedSize) {
		delete[] runtime_bytes;
		delete offsets;
		throw exception("unaccounted bytes detected in tag file! potential read failure! potential bad struct mappings!");
	}

	// first we should figure out the tag type, which we can do by looking through the tag structs 
	// and seeing which is the root, the guid will tell us which tag this is
	uint64_t target_group = 0;
	tag_def_structure* root_struct = nullptr; // we need this guy so we know which datablock is the root

	for (uint32_t c = 0; c < header->TagStructCount; c++) {
		tag_def_structure* curr_struct = reinterpret_cast<tag_def_structure*>(&tag_bytes[offsets->tag_structs_offset + (c * tag_def_structure_size)]);
		if (curr_struct->Type != 0) continue;

		target_group = curr_struct->GUID_1 ^ curr_struct->GUID_2;
		root_struct = curr_struct;
		break;
	}

	if (target_group == 0) {
		delete[] runtime_bytes;
		delete offsets;
		throw exception("No valid root struct!!!");
	}

	// lets do runtime fixups on the file now
	// fixup tag definition structures
	for (uint32_t c = 0; c < header->TagStructCount; c++) {
		tag_def_structure* current_struct = reinterpret_cast<tag_def_structure*> (&tag_bytes[offsets->tag_structs_offset + (c * tag_def_structure_size)]);
		if (current_struct->FieldBlock == -1) continue; // we dont need to give the main struct's pointer to anything as we already have it
		data_block* datar = reinterpret_cast<data_block*> (&tag_bytes[offsets->data_blocks_offset + (current_struct->FieldBlock * data_block_size)]);

		// we have to write the offset differently, depending on what type of struct this is
		if (current_struct->Type == 1) { // struct or custom?
			_basic_tagblock* tagblock = reinterpret_cast<_basic_tagblock*>(&runtime_bytes[resolve_datablock_offset(datar, offsets) + current_struct->FieldOffset]);
			if (current_struct->TargetIndex != -1) {
				data_block* contained_datar = reinterpret_cast<data_block*> (&tag_bytes[offsets->data_blocks_offset + (current_struct->TargetIndex * data_block_size)]);
				tagblock->content_ptr = &runtime_bytes[resolve_datablock_offset(contained_datar, offsets)];
			}
			else tagblock->content_ptr = nullptr;
		}
		// '4' because we dont really care what it is, just let us load
		else if (current_struct->Type == 2 || current_struct->Type == 3 || current_struct->Type == 4) { // resource // not sure why we previously had type 3 as tagblock??
			// we're not going to read external types right now
			_basic_resource* resource = reinterpret_cast<_basic_resource*>(&runtime_bytes[resolve_datablock_offset(datar, offsets) + current_struct->FieldOffset]);
			// use the handle as an index to the resource
			if (resource->runtime_resource_handle != 0 && resource->runtime_resource_handle != 0xBCBCBCBC) // verify that we didn't mess up the mappings
				throw exception("tag has data on runtime_resource_handle! there should be no data here! investigate!!");
			resource->runtime_resource_handle = current_resource_index;

			if (resource->is_chunked_resource == 0 && current_struct->Type != 4) {
				// get resource file header & process it as a regular tag
				module_file* resource_tag_header = ReturnResourceHeader(file_header, current_resource_index);
				current_resource_index++; // we only increase the index when

				char* resource_tag_ptr = nullptr;
				char* resource_cleanup_ptr = nullptr; // TODO: ADD SOMETHING TO BEABLE TO CLEAN THIS UP!!!
				GetTagProcessed(resource_tag_header, resource_tag_ptr, resource_cleanup_ptr, modules);
				resource->content_ptr = resource_tag_ptr; // link this resource tag to our current tag
			}
			else { // regular resources also have structs inside the main file that reference them
				if (current_struct->TargetIndex != -1) {
					data_block* contained_datar = reinterpret_cast<data_block*> (&tag_bytes[offsets->data_blocks_offset + (current_struct->TargetIndex * data_block_size)]);
					resource->content_ptr = &runtime_bytes[resolve_datablock_offset(contained_datar, offsets)];
				}
				else resource->content_ptr = nullptr;
			}
		}
		else { // unknown
			throw exception("unknown type thing!!");
		}
	}
	// fixup data references
	for (uint32_t c = 0; c < header->DataReferenceCount; c++) {
		data_reference* current_datar = reinterpret_cast<data_reference*>(&tag_bytes[offsets->data_references_offset + (c * data_reference_size)]);
		data_block* datar = reinterpret_cast<data_block*> (&tag_bytes[offsets->data_blocks_offset + (current_datar->FieldBlock * data_block_size)]);

		_basic_data* _datarblock = reinterpret_cast<_basic_data*>(&runtime_bytes[resolve_datablock_offset(datar, offsets) + current_datar->FieldOffset]);
		if (current_datar->TargetIndex != -1) {
			data_block* contained_datar = reinterpret_cast<data_block*> (&tag_bytes[offsets->data_blocks_offset + (current_datar->TargetIndex * data_block_size)]);
			_datarblock->content_ptr = &runtime_bytes[resolve_datablock_offset(contained_datar, offsets)];
		}
		else _datarblock->content_ptr = nullptr;
	}
	// fixup tag references
	for (uint32_t c = 0; c < header->TagReferenceCount; c++)
	{
		tag_fixup_reference* current_tagref = reinterpret_cast<tag_fixup_reference*> (&tag_bytes[offsets->tag_fixup_references_offset + (c * tag_fixup_reference_size)]);

		data_block* datar = reinterpret_cast<data_block*> (&tag_bytes[offsets->data_blocks_offset + (current_tagref->FieldBlock * data_block_size)]);
		_basic_tagref* _tagref = reinterpret_cast<_basic_tagref*>(&runtime_bytes[resolve_datablock_offset(datar, offsets) + current_tagref->FieldOffset]);

		if (current_tagref->DepdencyIndex != -1) {
			// idk how we're supposed to get the tag bytes from down here		
			tag_dependency* tag_dep = reinterpret_cast<tag_dependency*>(&tag_bytes[offsets->tag_dependencies_offset + (current_tagref->DepdencyIndex * tag_dependency_size)]);

			ModuleManager* mod_manager = (ModuleManager*)modules;
			try {
				Tag* referenced_tag = mod_manager->OpenTag(tag_dep->GlobalID);
				_tagref->content_ptr = referenced_tag->tag_data;
			}
			catch (exception ex) {
				ErrorLog::log_error("dependency tag could not be loaded: " + std::string(ex.what()));
				// this tool was NOT designed in mind of exception catching, HELLO MEMORY LEAKS!!!!
				_tagref->content_ptr = nullptr;
			}
		}
		else _tagref->content_ptr = nullptr;
	}
	
	/*
	TAG_OBJ_TYPE resulting_group = NONE;
	switch (target_group){
	case  1236057003492058159: resulting_group = bitmap;		break;
	case  4657725475941061082: resulting_group = runtime_geo;	break;
	case 13546876791234752572: resulting_group = render_model;	break;
	case  9265759122008847170: resulting_group = level;			break;
	}
	*/
	// assign the output data
	data_block* root_datar = reinterpret_cast<data_block*> (&tag_bytes[offsets->data_blocks_offset + (root_struct->TargetIndex * data_block_size)]);

	_Out_data = &runtime_bytes[resolve_datablock_offset(root_datar, offsets)];
	_Out_cleanup_ptr = runtime_bytes; // how is this outputting a number thats 0x28 larger than the previous
	delete offsets;
}
