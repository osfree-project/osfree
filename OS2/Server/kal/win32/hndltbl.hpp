#ifndef __HANDLETABLE_HPP
#define __HANDLETABLE_HPP

class HandleTable_ {
public:
        void **table;
        HandleTable_(int g, int s);
        virtual ~HandleTable_();

        void lockAll();
        void unlockAll();

        void lock(int idx);
        void unlock(int idx);
        int findAndLockFree();
private:
        unsigned long *hMutex;
        int gran, maxsize;
        int sidx;
};

template<class T, int granularity, int size>
class HandleTable : public HandleTable_ {
public:
        HandleTable() : HandleTable_(granularity,size) {}
        T *&operator[](int idx) { return (T*&)table[idx]; }
};

#endif

