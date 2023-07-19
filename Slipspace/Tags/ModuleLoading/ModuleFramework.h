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
private:
	module_file* find_tag(uint32_t tagID);
	void GetTagRaw(uint32_t tagID, char*& output_bytes, uint32_t& output_size);
public:
	Module(string filename);
	~Module();
	void GetTagProcessed(uint32_t tagID, char*& output_tag_bytes, char*& output_cleanup_ptr);
	void ReturnResource(uint32_t tagID, uint32_t index, char*& output_bytes, uint32_t output_size);
public:
	const int32_t target_module_version = 53;
	string filepath;
private:
	module_header* header;
	module_file* files;
	uint32_t* resource_indexes; 
	block_header* blocks;

	ifstream module_reader;
	uint64_t module_metadata_size; // used to offset when referencing datablock offsets
	Oodle* unpacker;
};



/*
class ModuleManager {
public:
	void OpenModule(string filename);
	void CloseModule(string filename);
	void OpenTag(uint32_t tagID);
private:
	vector<Module> loaded_modules;
};
*/