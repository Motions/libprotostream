#ifndef _MMAP_READER_H
#define _MMAP_READER_H

#include "stream_base.h"
#include <cstdlib>
#include <array>
#include <vector>
#include <sys/mman.h>


class mmap_reader : public stream_base {
    private:
    //uint8 for convenience (arithmetic)
    uint8_t *mmap_buffer;
    size_t mmap_buffer_size;
    public:
    mmap_reader():
        stream_base(stream_base::READ_ONLY),
        mmap_buffer(nullptr), mmap_buffer_size(0)
    {
    }
    virtual ~mmap_reader(){
        if(mmap_buffer && mmap_buffer != MAP_FAILED) {
            if(munmap(mmap_buffer, mmap_buffer_size) < 0)
                perror("munmap");
        }
    }
    virtual int open(const char *path) override;
    virtual void *load_header() override;
    virtual void *get_keyframe_data(offset_t n, size_t *size) override;
    virtual std::vector<std::pair<void*, size_t>> get_deltaframes_for_kf(offset_t keyframe) override;
    protected:
    virtual void find_keyframe(offset_t) override;
};
#endif
