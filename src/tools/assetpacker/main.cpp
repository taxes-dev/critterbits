#include <iostream>
#include <zstd.h>

int main(int argc, char ** argv) {
    ZSTD_CStream * stream = ZSTD_createCStream();
    std::cout << "Hello world " << ZSTD_VERSION_NUMBER << std::endl;
    ZSTD_freeCStream(stream);
    return 0;
}