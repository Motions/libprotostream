#include "stream_reader.h"
#include "mmap_reader.h"
#include "stream_writer.h"
#include <cstdio>
#include <cstring>

int main(int argc, char *argv[]) {
    if(argc < 3) {
        puts("arg?");
        return 1;
    }
    char *path = argv[2];
    int n;
    if(argv[1][0] == 'w') { //write
        if(argc < 5) {
            puts("arg?");
            return 1;
        }
        sscanf(argv[3], "%d", &n);
        stream_writer wr;
        wr.open(path);
        wr.init(1, argv[4], strlen(argv[4]));
        for(int i = 0; i < n; ++i) {
            char *line = nullptr;
            size_t tmp = 0;
            size_t line_len = getline(&line, &tmp, stdin);
            char *space = strchr(line, ' ');
            //printf("%d", (int)(line_len - (space - line) - 1));
            printf("AP kf|%.*s| d|%.*s||\n", (int)(space - line), line, (int)(line_len - (space - line) - 1), space);
            wr.append_keyframe(line, space - line);
            wr.append_delta(space, line_len - (space - line) - 1);
            free(line);
        }
    } else {    //read
        //stream_reader rd;
        mmap_reader rd;
        n = rd.open(path);
        printf("open: %d\n", n);
        rd.load_header();
        size_t hdr_size;
        char *hdr = reinterpret_cast<char*>(rd.get_header(&hdr_size));
        printf("HDR %ld: |%.*s|\n", hdr_size, (int)hdr_size, hdr);
        for(int i = 0; i < n; ++i) {
            size_t kf_len = 0;
            char *kf = (char*) rd.get_keyframe_data(i, &kf_len);
            printf("KF %d: %ld|%.*s| D: ", i, kf_len, (int)kf_len, kf);
            auto ds = rd.get_deltaframes_for_kf(i);
            for(auto p: ds)
                printf("%ld|%.*s|, ", p.second, (int)p.second, p.first);
            printf("\n");
            //rd.free_deltaframe_vector(ds);
        }
    }

}

