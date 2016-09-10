#include <fstream>
#include <limits.h>
#include <stdlib.h>

#include <cbfiles.h>

namespace Critterbits {

std::string GetExpandedPath(const std::string & unexpanded_path) {
    char * expanded_path = realpath(unexpanded_path.c_str(), NULL);
    if (expanded_path != NULL) {
        std::string n_expanded_path{expanded_path};
        free(expanded_path);
        return n_expanded_path;
    } else {
        return std::string{};
    }
}

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