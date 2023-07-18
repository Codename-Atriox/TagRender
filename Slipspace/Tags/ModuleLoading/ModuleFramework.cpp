#include "ModuleFramework.h"


Module::Module(string filename) {
    ifstream file_stream(filename, ios::binary | ios::ate);
    if (!file_stream.is_open()) {
        throw new exception("failed to open filestream");
    }
    streamsize file_size = file_stream.tellg();
    if (file_size < module_header_size) {
        throw new exception("filestream too small");
    }
    file_stream.seekg(0, ios::beg);

    // tempting to realign all this into a single byte array (like how it is in the file), however that would only impact performance for little benefit

    // read the header partition
    header = new module_header();
    file_stream.read((char*)header, module_header_size);

    // module file verification
    if (header->Head != 0x6D6F6864) { // 'mohd'
        throw new exception("target file does not appear to be module file");
    }
    if (header->Version != target_module_version) {
        throw new exception("module version does not match target version");
    }

    // read file partition
    files = new module_file[header->FileCount];
    file_stream.read((char*)files, module_file_size * header->FileCount);

    // read resource indicies partition
    resource_indexes = new uint32_t[header->ResourceCount];
    file_stream.read((char*)resource_indexes, 4 * header->ResourceCount);

    // read blocks partition
    blocks = new block_header[header->BlockCount];
    file_stream.read((char*)blocks, block_header_size * header->BlockCount);

    uint32_t file_offset = module_header_size + (module_file_size * header->FileCount) + (4 * header->ResourceCount) + (block_header_size * header->BlockCount);
    module_metadata_size = (file_offset / 0x1000 + 1) * 0x1000; // for some reason 343 aligns the metadata by 0x1000


    // and then he said "it's module'n time"
    //using (module_reader = new FileStream(module_file_path, FileMode.Open, FileAccess.Read)) {
        // read module header
        //module.module_info = read_and_convert_to<module_header>(module_header_size);

        // read module file headers
        //module.files = new module_file[module.module_info.FileCount];
        //for (int i = 0; i < module.files.Length; i++)
        //    module.files[i] = read_and_convert_to<module_file>(module_file_size);

        // read the string table
        //module.string_table = new byte[module.module_info.StringsSize];
        //module_reader.Read(module.string_table, 0, module.module_info.StringsSize);

        // read the resource indicies?
        //module.resource_table = new int[module.module_info.ResourceCount];
        //for (int i = 0; i < module.resource_table.Length; i++)
        //    module.resource_table[i] = read_and_convert_to<int>(4); // we should also fix this one too

        // read the data blocks
        //module.blocks = new block_header[module.module_info.BlockCount];
        //for (int i = 0; i < module.blocks.Length; i++)
        //    module.blocks[i] = read_and_convert_to<block_header>(block_header_size);


        // now to read the compressed data junk

        // align accordingly to 0x?????000 padding to read data
        long aligned_address = (module_reader.Position / 0x1000 + 1) * 0x1000;
        //module_reader.Seek(aligned_address, SeekOrigin.Begin);


        for (int i = 0; i < module.files.Length; i++) {
            // read the flags to determine how to process this file
            bool using_compression = (module.files[i].Flags & 0b00000001) > 0; // pretty sure this is true if reading_seperate_blocks is also true, confirmation needed
            bool reading_separate_blocks = (module.files[i].Flags & 0b00000010) > 0;
            bool reading_raw_file = (module.files[i].Flags & 0b00000100) > 0;

            byte[] decompressed_data = new byte[module.files[i].TotalUncompressedSize];
            long data_Address = aligned_address + module.files[i].DataOffset;

            if (reading_separate_blocks) {
                for (int b = 0; b < module.files[i].BlockCount; b++) {
                    var bloc = module.blocks[module.files[i].BlockIndex + b];
                    byte[] block_bytes;

                    if (bloc.Compressed == 1) {
                        module_reader.Seek(data_Address + bloc.CompressedOffset, SeekOrigin.Begin);

                        byte[] bytes = new byte[bloc.CompressedSize];
                        module_reader.Read(bytes, 0, bytes.Length);
                        block_bytes = Oodle.Decompress(bytes, bytes.Length, bloc.UncompressedSize);
                    }
                    else { // uncompressed
                        module_reader.Seek(data_Address + bloc.UncompressedOffset, SeekOrigin.Begin);

                        block_bytes = new byte[bloc.UncompressedSize];
                        module_reader.Read(block_bytes, 0, block_bytes.Length);
                    }
                    System.Buffer.BlockCopy(block_bytes, 0, decompressed_data, bloc.UncompressedOffset, block_bytes.Length);

                }
            }
            else {  // is the manifest thingo, aka raw file, read data based off compressed and uncompressed length
                module_reader.Seek(data_Address, SeekOrigin.Begin);
                if (using_compression) {
                    byte[] bytes = new byte[module.files[i].TotalCompressedSize];
                    module_reader.Read(bytes, 0, bytes.Length);
                    decompressed_data = Oodle.Decompress(bytes, bytes.Length, module.files[i].TotalUncompressedSize);
                }
                else module_reader.Read(decompressed_data, 0, module.files[i].TotalUncompressedSize);
            }
            // umm thats enough for now, this tool does not need to process the tags anymore

            // ok so now we need to unpack this tag, we'll basically just plop all of those decompressed bytes into a single file with the handy write all bytes function
            // so first we need to know what file/folder to write to, so lets fetch the name
            // NOTE: in later module versions, it does not provdei the file name, so we must either refere to a list of  names, or we must place the tag into a temp folder
            // i guess we could aos attempt tp calculate which folder this tag belongs in, as we could see who references it and stikc it in the folder of the guy who references
            // we could alos just group unknowns tags by tag group, so we'd ha ve the unknown folder, and then we'd have group folders inside that unknow folddedr that we'd then assort them into
            // but realistically, we'll worry about that when we get around to it, or rather when 343 gets around to it

            // get tag name
            string tag_name = "";
            int byte_offset = 0;
            while (true) {
                byte next_character = module.string_table[module.files[i].NameOffset + byte_offset];
                if (next_character == 0) break;

                tag_name += (char)next_character;
                byte_offset++;
            }
            // fixup the name if needed
            string file_path = out_tag_directory + tag_name;
            string test_extension = Path.GetExtension(file_path);
            if (test_extension.Contains(":"))
                file_path = Path.ChangeExtension(file_path, test_extension.Replace(":", "-"));

            // now write the file from the decompressed data (tag header + tag data + tag resource + whatever else we had)
            Directory.CreateDirectory(Path.GetDirectoryName(file_path));
            File.WriteAllBytes(file_path, decompressed_data);

        }
        // ok thats all, the tags have been read
    }
}