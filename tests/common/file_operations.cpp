#include "file_operations.h"

#include <fstream>
#include <iostream>

std::string file_contents(const boost::filesystem::path& path) {
    std::ifstream stream{path.native().c_str(), std::ifstream::binary};
    return {std::istreambuf_iterator<char>{stream}, std::istreambuf_iterator<char>{}};
}
