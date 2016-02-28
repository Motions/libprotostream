#include "stream.h"
#include "file_backend.h"
#include "mmap_backend.h"
#include "cache.h"

#include <iostream>

using namespace protostream;

struct string_factory {
    using type = std::string;

    static type build(const char *start, std::size_t len) {
        return std::string(start, start + len);
    }

    template<class Ptr>
    static type build(Ptr &&ptr, std::size_t len) {
        return build(reinterpret_cast<const char *>(detail::as_ptr(ptr)), len);
    }
};

using stream_writer = stream<
        with_backend<file_backend<file_mode_t::READ_APPEND>>,
        with_cache<full_cache>,
        with_keyframe_factory<string_factory>,
        with_delta_factory<string_factory>,
        with_proto_header_factory<string_factory>>;

using mmap_reader = stream<
        with_backend<mmap_backend>,
        with_cache<offsets_only_cache>,
        with_keyframe_factory<string_factory>,
        with_delta_factory<string_factory>,
        with_proto_header_factory<string_factory>>;

using stream_reader = stream<
        with_backend<file_backend<file_mode_t::READ_ONLY>>,
        with_cache<full_cache>,
        with_keyframe_factory<string_factory>,
        with_delta_factory<string_factory>,
        with_proto_header_factory<string_factory>>;

int main(int argc, char *argv[]) {
    if (argc < 3) {
        puts("arg?");
        return 1;
    }
    char *path = argv[2];
    int n;
    if (argv[1][0] == 'w') { //write
        if (argc < 5) {
            puts("arg?");
            return 1;
        }
        sscanf(argv[3], "%d", &n);
        stream_writer wr(path, 2, argv[4], strlen(argv[4]));
        for (int i = 0; i < n; ++i) {
            char *line = nullptr;
            size_t tmp = 0;
            ssize_t line_len = getline(&line, &tmp, stdin);
            char *space = strchr(line, ' ');
            //printf("%d", (int)(line_len - (space - line) - 1));
            printf("AP kf|%.*s| d|%.*s||\n", (int) (space - line), line, (int) (line_len - (space - line) - 1), space);
            wr.append_keyframe(reinterpret_cast<std::uint8_t *>(line), space - line);
            wr.append_delta(reinterpret_cast<std::uint8_t *>(space), line_len - (space - line) - 1);
            free(line);
        }
    } else {    //read
        mmap_reader rd(path);
        std::cout << "header: " << rd.get_proto_header() << std::endl;
        for (const auto &keyframe: rd) {
            std::cout << "keyframe: " << keyframe.get() << std::endl;

            for (const auto &delta: keyframe) {
                std::cout << "delta: " << delta.get() << std::endl;
            }
        }
    }

}

