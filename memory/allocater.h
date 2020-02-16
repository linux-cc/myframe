#ifndef __MEMORY_ALLOC_H__
#define __MEMORY_ALLOC_H__

#include <new>
#include "util/util.h"

namespace memory {

void *allocate(int size);
void deallocate(const void *addr, int size);

template <typename T>
class Allocater {
public:
    static T *New() {
        void *p = allocate(sizeof(T));
        return construct(util::IsTrivial<T>(), p);
    }
    
    template <typename P1>
    static T *New(const P1& p1) {
        void *p = allocate(sizeof(T));
        return construct(util::IsTrivial<T>(), p, p1);
    }
    
    template <typename P1, typename P2>
    static T *New(const P1 &p1, const P2 &p2) {
        void *p = allocate(sizeof(T));
        return construct(util::IsTrivial<T>(), p, p1, p2);
    }
    
    template <typename P1, typename P2, typename P3>
    static T *New(const P1 &p1, const P2 &p2, const P3 &p3) {
        void *p = allocate(sizeof(T));
        return construct(util::IsTrivial<T>(), p, p1, p2, p3);
    }
    
    template <typename P1, typename P2, typename P3, typename P4>
    static T *New(const P1 &p1, const P2 &p2, const P3 &p3, const P4 &p4) {
        void *p = allocate(sizeof(T));
        return construct(util::IsTrivial<T>(), p, p1, p2, p3, p4);
    }
    
    static void Delete(const T *addr) {
        destruct(util::IsTrivial<T>(), addr);
        deallocate(addr, sizeof(T));
    }
    
private:
    static T *construct(util::TrueType, void *p) { return (T*)p; }
    static T *construct(util::FalseType, void *p) { return new (p) T(); }
    template <typename P1>
    static T *construct(util::FalseType, void *p, const P1 &p1) { return new (p) T(p1); }
    template <typename P1, typename P2>
    static T *construct(util::FalseType, void *p, const P1 &p1, const P2 &p2) { return new (p) T(p1, p2); }
    template <typename P1, typename P2, typename P3>
    static T *construct(util::FalseType, void *p, const P1 &p1, const P2 &p2, const P3 &p3) { return new (p) T(p1, p2, p3); }
    template <typename P1, typename P2, typename P3, typename P4>
    static T *construct(util::FalseType, void *p, const P1 &p1, const P2 &p2, const P3 &p3, const P4 &p4) { return new (p) T(p1, p2, p3, p4); }
    static void destruct(util::TrueType, const T* p) { }
    static void destruct(util::FalseType, const T* p) { p->~T(); }
};

template <typename T>
class Allocater<T[]> {
public:
    static T *New(int size) {
        void *p = allocate(sizeof(T) * size);
        return construct(util::IsTrivial<T>(), p, size);
    }
    
    static void Delete(const T *addr, int size) {
        destruct(util::IsTrivial<T>(), addr, size);
        deallocate(addr, sizeof(T) * size);
    }
    
private:
    static T *construct(util::TrueType, void *p, int size) { return (T*)p; }
    static T *construct(util::FalseType, void *p, int size) { return new (p) T[size]; }
    static void destruct(util::TrueType, const T* p, int size) { }
    static void destruct(util::FalseType, const T* p, int size) {
        for (int i = 0; i < size; i++) {
            const T *pi = p + i;
            pi->~T();
        }
    }
};

} /* namespace memory */

#endif /* __UTIL_ALLOC_H__ */

