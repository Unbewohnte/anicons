/*
Copyright © 2021, 2022 Kasyanov Nikolay Alexeyevich

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the “Software”), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/


#include <ios>
#include <iostream>
#include <fstream>

uint32_t getsize(char buf[]) {
    return *(uint32_t*) buf;
};

// RIFF chunk implementation
class Chunk {
    public:
    std::string fourcc;
    uint32_t size;
    // position in file where this exact chunk`s contents are stored
    uint64_t data_start_pos;
    uint64_t data_end_pos;

    ~Chunk() {};

    // reads chunk header data and stops before its contents
    int read(std::fstream* file, uint64_t from = 0) {
        file->seekg(from, std::ios::beg);

        if (file->eof()) {
            return 1;
        };

        char buf[8];
        file->read(buf, 8);

        uint64_t current_pos = file->tellg();
        
        // fourcc
        for (unsigned int i = 0; i < 4; i++) {
            fourcc += buf[i];
        };

        // size, end position
        size = getsize(buf + 4);
        if (size & 1) {
            data_end_pos = size + 1 + current_pos;
        } else {
            data_end_pos = size + current_pos;
        };
        
        // data start
        if (fourcc == "RIFF" || fourcc == "LIST") {
            data_start_pos = current_pos + 4;
        } else {
            data_start_pos = current_pos;
        };

        return 0;
    };

    // fills buf with the contents of the chunk
    int read_contents(std::fstream* file, char buf[]) {
        // sanity checks
        if (fourcc.length() != 4 || size == 0) {
            return 1;
        }
        file->seekg(data_start_pos, std::ios::beg);

        file->read(buf, size);
        
        // account for a padding byte
        if (size & 1) {
            uint64_t current_pos = file->tellg();
            file->seekg(current_pos + 1, std::ios::beg);
        };

        return 0;
    };
};

void print_help() {
    std::cout << "anicons v0.1.1\nhelp: anicons [FILEPATH]" << std::endl;
};

int main(int argc, char* argv[]) {
    if (argc < 2) {
        print_help();
        return 0;
    }

    std::string filepath = argv[1];
    
    // open file
    std::fstream riff_file(filepath, std::ios::in | std::ios::binary);
    if (!riff_file.is_open()) {
        std::cout << "Could not open \"" << filepath << "\""  << std::endl;
        return 1;
    };

    std::string filename = "";
    std::string parent_dir = "";
    auto pos = filepath.find_last_of("/\\");
    if (pos != std::string::npos) {
        parent_dir = filepath.substr(0, pos);
        filename = filepath.substr(pos, filepath.length());
    } else {
        filename = filepath;
    }

    // try to read RIFF chunk
    Chunk riff_chunk;
    int errcode = riff_chunk.read(&riff_file);
    if (errcode != 0 || riff_chunk.fourcc != "RIFF") {
        std::cout << "Could not read RIFF chunk from \"" << filename << "\"" << std::endl;
        return 1;
    };
    
    // go through each chunk and find "icon" ones
    uint64_t from = 12;
    uint32_t ico_counter = 0;
    while (from < riff_chunk.size || riff_file.eof()) {
        Chunk ch;
        ch.read(&riff_file, from);

        if (ch.fourcc == "icon") {
            // save it as a separate file
            char buf[ch.size];
            ch.read_contents(&riff_file, buf);

            std::fstream icon_file;
            if (parent_dir != "") {
                icon_file = std::fstream(
                    parent_dir + filename + "-" + std::to_string(ico_counter) + ".ico",
                     std::ios::out | std::ios::binary);
            } else {
                icon_file = std::fstream(
                    filename + "-" + std::to_string(ico_counter) + ".ico",
                     std::ios::out | std::ios::binary);
            }
            icon_file.write(buf, ch.size);
            icon_file.close();

            ico_counter++;
        };

        if (ch.fourcc == "LIST") {
            from = ch.data_start_pos;
        } else {
            from = ch.data_end_pos;
        };
    };
    
    riff_file.close();

    return 0;
}