#include "ModuleFramework.h"


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

void Module::GetTagProcessed(uint32_t tagID, char*& output_tag_bytes, char*& output_cleanup_ptr) {
    module_file* file_ptr = find_tag(tagID);
    if (file_ptr == 0)
        throw exception("failed to find tag in module");
    GetTagProcessed(file_ptr, output_tag_bytes, output_cleanup_ptr);
}
void Module::GetTagProcessed(module_file* file_ptr, char*& output_tag_bytes, char*& output_cleanup_ptr) {
    // read tag from module (ifstream)
    char* raw_tag_bytes = new char[file_ptr->TotalUncompressedSize];
    GetTagRaw(file_ptr, raw_tag_bytes);
    // process tag to make it usable
    TagProcessing::Open_ready_tag(raw_tag_bytes, file_ptr->TotalUncompressedSize, output_tag_bytes, output_cleanup_ptr);
}

void Module::ReturnResource(uint32_t tag_index, uint32_t index, char* output_buffer, uint32_t output_size){
    module_file* target_tag = GetTagHeader_AtIndex(tag_index);

    if (index >= target_tag->ResourceCount)
        throw exception("Attemtped to access out of bounds tag resource index");

    uint32_t resource_index = target_tag->ResourceIndex + index;
    if (resource_index >= header->ResourceCount)
        throw exception("Attemtped to access out of bounds module file resource index");

    // get the resources
    module_file* resource_tag = GetTagHeader_AtIndex(resource_indexes[resource_index]);
    if (resource_tag->GlobalTagId != -1 || resource_tag->ClassId != -1)
        throw exception("indexed resource file is not a resource file");

    if ((resource_tag->Flags & flag_UseRawfile) == 0)
        throw exception("resource files with tagdata content are not currently supported");

    if (resource_tag->TotalUncompressedSize > output_size)
        throw exception("not enough room allocated to fit indexed resource");

    GetTagRaw(resource_tag, output_buffer);
}

module_file* Module::GetTagHeader_AtIndex(uint32_t index){
    if (index >= file_count)
        throw exception("attempted to fetch module from invalid index");

    return &files[index];
}
// output bytes NEEDS to already be allocated
void Module::GetTagRaw(module_file* file_ptr, char* output_bytes) {
    if (output_bytes == 0)
        throw exception("tag output buffer was not preallocated");
    // then begin the read
    // we have to map all the data to read, based on the owned datablocks

    // read the flags to determine how to process this file
    bool using_compression       = (file_ptr->Flags & flag_UseCompression) != 0; // pretty sure this is true if reading_seperate_blocks is also true, confirmation needed
    bool reading_separate_blocks = (file_ptr->Flags & flag_UseBlocks) != 0;
    bool reading_raw_file        = (file_ptr->Flags & flag_UseRawfile) != 0;

    if (file_ptr->TotalUncompressedSize == 0)
        throw exception("module file was empty");

    // god dammit, mf 'long' how about you take a long trip down to the bottom of the ocean
    // please someone for the love of god make the long 64 bits, and not just literally an int
    uint64_t data_Address = module_metadata_size + file_ptr->DataOffset;

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
                // delete[] output_bytes; // NOTE: THIS HAS TO BE CLEANED UP MANUALLY
                throw ex;
            }
            delete[] raw_bytes;
        }else 
            module_reader.read(output_bytes, file_ptr->TotalUncompressedSize);
    }
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
