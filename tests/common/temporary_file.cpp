#include "temporary_file.h"
#include "file_operations.h"

temporary_file::temporary_file() : path{boost::filesystem::unique_path()}, released{false} { }

temporary_file::temporary_file(const std::string& initial_contents) : temporary_file{} {
    auto stream = std::ofstream{filepath(), std::ofstream::binary};
    stream << initial_contents;
}

const char* temporary_file::filepath() const {
    return path.native().c_str();
}

temporary_file::temporary_file(temporary_file&& that) : path{std::move(that.path)}, released{false} { }

temporary_file& temporary_file::operator=(temporary_file&& that) {
    path = std::move(that.path);
    released = false;
    that.released = true;
    return *this;
}

temporary_file::~temporary_file() {
    if (!released) {
        close();
    }
}

void temporary_file::close() {
    if (released) {
        throw std::logic_error{"Closing a released temporary file"};
    }

    released = true;
    if (boost::filesystem::exists(path)) {
        boost::filesystem::remove(path);
    }
}

std::string temporary_file::contents() const {
    return file_contents(path);
}

std::size_t temporary_file::size() const {
    return boost::filesystem::file_size(path);
}