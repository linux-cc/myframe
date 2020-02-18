#ifndef __MEMORY_BUDDY_ALLOC_H__
#define __MEMORY_BUDDY_ALLOC_H__

namespace memory {

class BuddyAlloc {
public:
    BuddyAlloc(int blocks, int blockSize, int pageSize = 4096):
        _maddr(0), _buffer(0), _tree(0), _size(0), _blockShiftBit(0), _blocksPow(0) {
        init(blocks, blockSize, pageSize);
    }
    ~BuddyAlloc();
    void init(int blocks, int blockSize, int pageSize);
    void *alloc(int size);
    void free(const void *addr);
    int getPageSize() const { return _pageSize; }
    char *dump();
    char *buffer() const { return _buffer; }

private:
    char *_maddr;
    char *_buffer;
    char *_tree;
    int _size;
    int _pageSize;
    char _blockShiftBit;
    char _blocksPow;
};

} /* namespace memory */

#endif /* ifndef __MEMORY_BUDDYALLOC_H__ */
