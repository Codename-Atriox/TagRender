#pragma once
#include <stdint.h>
#include <string>
#include <map>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <vector>

class Tagnames {
private:
    std::map<uint32_t, std::string> tagnames;
public:
    const char* tagnames_file = "Data\\Slipspace\\tagnamesSLIM.txt";
    Tagnames() {
        try{
            // open the file:
            std::streampos fileSize;
            std::ifstream file(tagnames_file, std::ios::binary);

            // get its size:
            file.seekg(0, std::ios::end);
            fileSize = file.tellg();
            file.seekg(0, std::ios::beg);

            // read the data:
            std::vector<char> fileData(fileSize);
            file.read((char*)&fileData[0], fileSize);

            // then process into tagnames map
            uint32_t index = 0;
            while (index < fileSize) {
                // read tagid
                uint32_t tagid = *(uint32_t*)&fileData[index];
                index += 4;
                // then while loop to read string
                std::string tagname = &fileData[index];
                index += tagname.length() + 1;
                // add into the map
                tagnames[tagid] = tagname;
            }
        }catch (std::exception exe) {
            // do nothing here, we will log these errors when we build an error log lol
        }
        
    }
    std::string GetTagname(uint32_t tagid) {
        std::map<uint32_t, std::string>::iterator i = tagnames.find(tagid);
        if (i == tagnames.end()) {
            return int_to_hex(tagid);
        } 
        else return i->second;
    }
private:

    std::string int_to_hex(uint32_t i){ // doesn't seem the best but whatever
        std::stringstream stream;
        stream << "0x"
            << std::setfill('0') << std::setw(sizeof(uint32_t) * 2)
            << std::hex << i;
        return stream.str();
    }
};






