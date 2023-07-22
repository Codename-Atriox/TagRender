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
using namespace ModuleStructs;
using namespace std;

class Module {
public:
	Module(string filename);
	~Module();

	module_file* find_tag(uint32_t tagID);
	void GetTagProcessed(uint32_t tagID, char*& output_tag_bytes, char*& output_cleanup_ptr);
	void GetTagProcessed(module_file* file_ptr, char*& output_tag_bytes, char*& output_cleanup_ptr);
	void ReturnResource(uint32_t tagID, uint32_t index, char*& output_bytes, uint32_t output_size);

	module_file* GetTagHeader_AtIndex(uint32_t index);
private:
	void GetTagRaw(module_file* file_ptr, char*& output_bytes, uint32_t& output_size);
public:
	const int32_t target_module_version = 53;
	string filepath;
	uint32_t file_count;
private:
	module_header* header;
	module_file* files;
	uint32_t* resource_indexes; 
	block_header* blocks;

	ifstream module_reader;
	uint64_t module_metadata_size; // used to offset when referencing datablock offsets
	Oodle* unpacker;
};



