// TagFramework.h - Contains the logic for reading tag files into renderable objects
#pragma once
#include <string>
#include <map>
#include <stdint.h>
#include <vector>
#include <fstream>
#include <iostream>
#include <sstream>
#include <filesystem>
#include "TagStructs.h"
using namespace TagStructs;
using namespace std;

// this defines the possible data type output from reading a tag file
// if its none of these, then the data will be discarded as we have no use for it
// thus returning a nullptr
/*
enum TAG_OBJ_TYPE{
	NONE,
	bitmap,
	render_model,
	physics_model,
	collision_model,
	bsp,
	runtime_geo,
	terrain,
	level
};
struct size_pointer {
public:
	size_pointer(char* ptr, unsigned int _size){
		content_ptr = ptr;
		size = _size;
	}
	~size_pointer() {
		if (content_ptr != 0)
			delete[] content_ptr;
	}
	char* content_ptr;
	unsigned int size;
};
class runtime_tag{
public:
    void* tag_data; // to use this, you must cast it to the specific tag's structure, these structures can be generated via the plugins_to_cpp_convertor;
    vector<size_pointer>* streaming_chunks;
	runtime_tag(void* _tag_data, vector<size_pointer>* chunk_resources, vector<size_pointer>* struct_resources, char* _cleanup_ptr) {
		tag_data = _tag_data;
		streaming_chunks = chunk_resources;
		resource_structs_cleanup_ptrs = struct_resources;
		cleanup_ptr = _cleanup_ptr;
	}
	~runtime_tag(){ // cleanup all allocated structs/resources
		delete[] cleanup_ptr;
		// these should be handled automatically now
		//for (int c = 0; c < streaming_chunks->size(); c++) delete (*streaming_chunks)[c];
		//for (int c = 0; c < resource_structs_cleanup_ptrs->size(); c++) delete (*resource_structs_cleanup_ptrs)[c];
	}
private:
	char* cleanup_ptr; // call 'delete' on this to clean up the tag
	vector<size_pointer>* resource_structs_cleanup_ptrs; // used to clean up, each item's size will be listed as 0 when loaded
};
*/


static class TagProcessing {

};
