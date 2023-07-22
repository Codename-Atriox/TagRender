#include "TagProcessor.h"


ModuleManager::Tag::Tag(uint32_t _4CC, uint32_t _ID, char* _data, char* _clnup, char** _r, uint32_t _rcnt) {
	tag_FourCC = _4CC;
	tagID = _ID;
	tag_data = _data;
	tag_cleanup_ptr = _clnup;
	resources = _r;
	resource_count = _rcnt;
}
ModuleManager::Tag::~Tag() {
	delete[] tag_cleanup_ptr;
	for (int i = 0; i < resource_count; i++)
		delete[] resources[i];
	delete[] resources;
}

void ModuleManager::OpenModule(string filename){
	CloseModule(filename); // if this module is already open, reopen it

	try{
		Module* new_module = new Module(filename);
		loaded_modules->push_back(new_module);
		open_modules++;
		total_tags += new_module->file_count;
	} catch (exception ex) {
		throw new exception("failed to open module");
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

ModuleManager::Tag* ModuleManager::GetTag(uint32_t tagID) {
	for (int i = 0; i < loaded_tags->size(); i++)
		if ((*loaded_tags)[i].tagID == tagID)
			return &(*loaded_tags)[i];
	return (Tag*)0;
}
ModuleManager::Tag* ModuleManager::OpenTag(uint32_t tagID){
	// first check if the tag already exists
	Tag* new_tag = GetTag(tagID);
	if (new_tag != (Tag*)0) return new_tag;

	for (int c = 0; c < loaded_modules->size(); c++) {
		Module* module_ptr = (*loaded_modules)[c];

		module_file* file_ptr = module_ptr->find_tag(tagID);
		if (file_ptr == 0)
			continue; // this module does not contain the tag

		char* output_tag_bytes;
		char* output_cleanup_ptr;
		module_ptr->GetTagProcessed(file_ptr, output_tag_bytes, output_cleanup_ptr);

		char** tag_resources = (char**)(new char[file_ptr->ResourceCount]); // only create the base level
		for (int i = 0; i < file_ptr->ResourceCount; i++)
			tag_resources[i] = (char*)0; // null each pointer in the resource array

		Tag* new_tag = new Tag(file_ptr->ClassId, file_ptr->GlobalTagId, output_tag_bytes, output_cleanup_ptr, tag_resources, file_ptr->ResourceCount);
		loaded_tags->push_back(*new_tag);
		return new_tag;
	}
	throw new exception("tag with specified tagID was not found in any loaded modules");
}

Module* ModuleManager::GetModule_AtIndex(uint32_t index){
	if (index >= loaded_modules->size())
		throw new exception("attempted to fetch module from invalid index");

	return (*loaded_modules)[index];
}

