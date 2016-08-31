#include <fstream>

#include <cbfiles.h>

namespace Critterbits {
bool ReadTextFile(const std::string & file_name, std::string ** file_contents) {
    std::fstream fs;

    fs.open(file_name, std::fstream::in);
    if (fs.good()) {
        *file_contents = new std::string((std::istreambuf_iterator<char>(fs)), (std::istreambuf_iterator<char>()));
        fs.close();
        return true;
    }
    return false;
}
}