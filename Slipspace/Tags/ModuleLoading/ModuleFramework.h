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
	void GetTagProcessed(uint32_t tagID, char*& output_tag_bytes, char*& output_cleanup_ptr);
	void GetTagProcessed(module_file* file_ptr, char*& output_tag_bytes, char*& output_cleanup_ptr);
	void ReturnResource(uint32_t tag_index, uint32_t index, char* output_buffer, uint32_t output_size);

	module_file* GetTagHeader_AtIndex(uint32_t index);
private:
	void GetTagRaw(module_file* file_ptr, char* output_bytes);
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
};



