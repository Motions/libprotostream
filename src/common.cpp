#include "common.h"

bool read_loop(int fd, void *buf, size_t count, off_t offset) {
    size_t rd = 0;
    while(count > rd) {
        ssize_t ret;
        if(offset < 0)
            ret = read(fd, (uint8_t*) buf + rd, count);
        else
            ret = pread(fd, (uint8_t*) buf + rd, count, offset + rd);

        if(ret == 0) {
            fprintf(stderr, "Premature end of file.\n");
            return false;
        } else if(ret < 0) {
            perror("read");
            return false;
        }

        rd += ret;
    }
    return true;
}

bool write_loop(int fd, const void *buf, size_t count, off_t offset) {
    //if(offset > 0 && offset < 32) {
        //fprintf(stderr, "LOW WRITE: %ld\n", offset);
    //}
    size_t wr = 0;
    while(count > wr) {
        ssize_t ret;
        if(offset < 0)
            ret = write(fd, (uint8_t*) buf + wr, count);
        else
            ret = pwrite(fd, (uint8_t*) buf + wr, count, offset + wr);

        if(ret == 0) {
            fprintf(stderr, "write() returned 0?"); //TODO
            return false;
        } else if(ret < 0) {
            perror("read");
            return false;
        }

        wr += ret;
    }
    return true;
}
