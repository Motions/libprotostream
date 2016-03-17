#pragma once

#include <boost/filesystem.hpp>

class temporary_file {
public:
    temporary_file();

    temporary_file(const std::string& initial_contents);

    const char* filepath() const;

    temporary_file(const temporary_file&) = delete;

    temporary_file(temporary_file&& that);

    temporary_file& operator=(const temporary_file&) = delete;

    temporary_file& operator=(temporary_file&& that);

    ~temporary_file();

    void close();

    std::string contents() const;

    std::size_t size() const;

private:
    boost::filesystem::path path;
    bool released;
};