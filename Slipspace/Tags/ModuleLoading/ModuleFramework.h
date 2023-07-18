#pragma once
#include <string>
#include <stdint.h>
#include <vector>

#include <fstream>
#include <iostream>
#include <sstream>
#include <filesystem>

#include "ModuleStructs.h"
using namespace ModuleStructs;
using namespace std;

class Module {
public:
	Module(string filename);
	void ReturnTag(uint32_t tagID, char*& output_bytes, uint32_t output_size);
	void ReturnResource(uint32_t tagID, uint32_t index, char*& output_bytes, uint32_t output_size);
public:
	const int32_t target_module_version = 53;
	string filepath;
private:
	module_header* header;
	module_file* files;
	uint32_t* resource_indexes; 
	block_header* blocks;
	uint64_t module_metadata_size; // used to offset when referencing datablock offsets
};




class ModuleManager {
public:
	void OpenModule(string filename);
	void CloseModule(string filename);
	void OpenTag(uint32_t tagID);
private:
	vector<Module> loaded_modules;
};