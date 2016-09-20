#include <fstream>
#include <limits.h>
#include <stdlib.h>

#include <cb/files.hpp>

namespace Critterbits {

std::string GetExpandedPath(const std::string & unexpanded_path) {
#ifdef WIN32
    char * expanded_path = _fullpath(NULL, unexpanded_path.c_str(), _MAX_PATH);
#else
    char * expanded_path = realpath(unexpanded_path.c_str(), NULL);
#endif
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