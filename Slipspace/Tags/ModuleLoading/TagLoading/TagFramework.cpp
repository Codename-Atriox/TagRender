
#include "TagFramework.h"


uint64_t TagProcessing::resolve_datablock_offset(data_block* datar, tag_loading_offsets* offsets) {
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
void TagProcessing::Open_ready_tag(char* tag_bytes, uint64_t tag_size, char*& _Out_tag, char*& _Out_cleanup_ptr){
	Processtag(tag_bytes, tag_size, _Out_tag, _Out_cleanup_ptr);
	delete[] tag_bytes; // cleanup the file read request
}
void TagProcessing::Processtag(char* tag_bytes, uint64_t file_size, char*& _Out_data, char*& _Out_cleanup_ptr){
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
	current_byte_offset += header->StringTableSize;

	offsets->zoneset_info_offset = current_byte_offset;
	current_byte_offset += header->ZoneSetDataSize; // unsure if this includes the header or just all the elements
	// current offset now marks the entirety of the header, so we can create a new array excluding (current_offset) bytes
	offsets->header_size = file_size - current_byte_offset;

	char* runtime_bytes = new char[file_size-current_byte_offset];
	std::copy(tag_bytes+current_byte_offset, tag_bytes+file_size, runtime_bytes);

	// ok now we need to get the offsets for the 3 extra blocks (or was it two?) although these can be calculated inplace rather easily as they're only one long + another

	offsets->data_1_offset = header->HeaderSize - current_byte_offset;
	offsets->data_2_offset = offsets->data_1_offset + header->DataSize;
	offsets->data_3_offset = offsets->data_2_offset + header->ResourceDataSize;

	// now we note any unmapped data segments in the file
	
	if (offsets->data_3_offset + current_byte_offset + header->ActualResoureDataSize != file_size) {
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

	if (target_group == 0){
		delete[] runtime_bytes;
		delete offsets;
		throw exception("No valid root struct!!!");
	}

	// lets do runtime fixups on the file now
	// fixup tag definition structures
	for (uint32_t c = 0; c < header->TagStructCount; c++){
		tag_def_structure* current_struct = reinterpret_cast<tag_def_structure*> (&tag_bytes[offsets->tag_structs_offset + (c * tag_def_structure_size)]);
		if (current_struct->FieldBlock == -1) continue; // we dont need to give the main struct's pointer to anything as we already have it
		data_block* datar = reinterpret_cast<data_block*> (&tag_bytes[offsets->data_blocks_offset + (current_struct->FieldBlock * data_block_size)]);

		// we have to write the offset differently, depending on what type of struct this is
		if (current_struct->Type == 1){ // struct or custom?
			_basic_tagblock* tagblock = reinterpret_cast<_basic_tagblock*>(&runtime_bytes[resolve_datablock_offset(datar, offsets) + current_struct->FieldOffset]);
			if (current_struct->TargetIndex != -1){
				data_block* contained_datar = reinterpret_cast<data_block*> (&tag_bytes[offsets->data_blocks_offset + (current_struct->TargetIndex * data_block_size)]);
				tagblock->content_ptr = &runtime_bytes[resolve_datablock_offset(contained_datar, offsets)];
			} else tagblock->content_ptr = nullptr;
		}
		else if (current_struct->Type == 2 || current_struct->Type == 3) { // resource // not sure why we previously had type 3 as tagblock??
			// we're not going to read external types right now
			_basic_resource* resource = reinterpret_cast<_basic_resource*>(&runtime_bytes[resolve_datablock_offset(datar, offsets) + current_struct->FieldOffset]);
			// use the handle as an index to the resource
			if (resource->runtime_resource_handle != 0 && resource->runtime_resource_handle != 0xBCBCBCBC) // verify that we didn't mess up the mappings
				throw exception("tag has data on runtime_resource_handle! there should be no data here! investigate!!");
			resource->runtime_resource_handle = current_resource_index;

			if (resource->is_chunked_resource == 0) { // currently nothing is to be done here
				/*void* resource_struct; // output struct ptr
				char* new_resource_ptr;
				Processtag((*resources)[current_resource_index].content_ptr, (*resources)[current_resource_index].size, resource_struct, nullptr, new_resource_ptr);
				(*resources)[current_resource_index].content_ptr = new_resource_ptr;
				(*resources)[current_resource_index].size = 0;
				resource->content_ptr = resource_struct;*/

			}
			else { // regular resources also have structs inside the main file that reference them
				if (current_struct->TargetIndex != -1) {
					data_block* contained_datar = reinterpret_cast<data_block*> (&tag_bytes[offsets->data_blocks_offset + (current_struct->TargetIndex * data_block_size)]);
					resource->content_ptr = &runtime_bytes[resolve_datablock_offset(contained_datar, offsets)];
				}
				else resource->content_ptr = nullptr;
			}
			current_resource_index++;
		}else{ // unknown
			throw exception("unknown type thing!!");
	}}
	// fixup data references
	for (uint32_t c = 0; c < header->DataReferenceCount; c++){
		data_reference* current_datar = reinterpret_cast<data_reference*>(&tag_bytes[offsets->data_references_offset + (c * data_reference_size)]);
		data_block* datar = reinterpret_cast<data_block*> (&tag_bytes[offsets->data_blocks_offset + (current_datar->FieldBlock * data_block_size)]);

		_basic_data* _datarblock = reinterpret_cast<_basic_data*>(&runtime_bytes[resolve_datablock_offset(datar, offsets) + current_datar->FieldOffset]);
		if (current_datar->TargetIndex != -1) {
			data_block* contained_datar = reinterpret_cast<data_block*> (&tag_bytes[offsets->data_blocks_offset + (current_datar->TargetIndex * data_block_size)]);
			_datarblock->content_ptr = &runtime_bytes[resolve_datablock_offset(contained_datar, offsets)];
		} else _datarblock->content_ptr = nullptr;
	}
	/* fixup tag references
	for (uint32_t c = 0; c < header->TagReferenceCount; c++)
	{
		tag_fixup_reference* current_struct = reinterpret_cast<tag_fixup_reference*> (&tag_bytes[offsets->tag_fixup_references_offset + (c * tag_fixup_reference_size)]);


	}
	*/
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
