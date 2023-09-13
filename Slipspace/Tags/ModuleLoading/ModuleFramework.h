#pragma once
#include <string>
#include <stdint.h>
#include <vector>

#include <fstream>
#include <iostream>
#include <sstream>
#include <filesystem>

#include "ModuleStructs.h"
#include "oodle/oodle.h"
#include "TagLoading/TagFramework.h"

#include "../../../StringHelper.h"

using namespace ModuleStructs;
using namespace std;

class Module {
public:
	Module(string filename, Oodle* oodler);
	~Module();

	module_file* find_tag(uint32_t tagID);
	int32_t find_tag_index(uint32_t tagID);
	void GetTagProcessed(uint32_t tagID, char*& output_tag_bytes, char*& output_cleanup_ptr, void* modules);
	void GetTagProcessed(module_file* file_ptr, char*& output_tag_bytes, char*& output_cleanup_ptr, void* modules);

	module_file* ReturnResourceHeader(uint32_t tag_index, uint32_t index);
	module_file* ReturnResourceHeader(module_file* tag_header, uint32_t index);
	HRESULT ReturnResource(uint32_t tag_index, uint32_t index, char* output_buffer, uint32_t output_size);

	module_file* GetTagHeader_AtIndex(uint32_t index);
private:
	HRESULT GetTagRaw(module_file* file_ptr, char* output_bytes);
public:
	const int32_t target_module_version = 53;
	string filename;
	string filepath;
	uint32_t file_count;
	// UI implementation
	int32_t selected_tag; // NOTE: maximum tag count is limited to int32
private:
	module_header* header;
	module_file* files;
	uint32_t* resource_indexes; 
	block_header* blocks;

	ifstream module_reader;
	uint64_t module_metadata_size; // used to offset when referencing datablock offsets
	Oodle* unpacker;

	// ////////////// //
	// TAG FRAMEWORK //
	// //////////// //
public:
	void Open_ready_tag(char* tag_bytes, uint64_t tag_size, char*& _Out_tag, char*& _Out_cleanup_ptr);
private:
	struct tag_loading_offsets {
		uint64_t tag_dependencies_offset;
		uint64_t data_blocks_offset;
		uint64_t tag_structs_offset;
		uint64_t data_references_offset;
		uint64_t tag_fixup_references_offset;
		uint64_t string_table_offset;
		uint64_t zoneset_info_offset;
		// these three are offsets into the headerless array (so -= header.size)
		uint64_t header_size;
		uint64_t data_1_offset; // tag data
		uint64_t data_2_offset; // resource data
		uint64_t data_3_offset; // alt resource data
	};
	uint64_t resolve_datablock_offset(TagStructs::data_block* datar, tag_loading_offsets* offsets);
	void Processtag(char* tag_bytes, module_file* file_header, char*& _Out_data, char*& _Out_cleanup_ptr, void* modules);
};



