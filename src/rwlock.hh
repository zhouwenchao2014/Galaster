#ifndef _RWLOCK_H_
#define _RWLOCK_H_


#include <pthread.h>
#include <system_error>
#include <cerrno>


// 
// A reader writer lock allows multiple concurrent readers or one exclusive writer
// 
// TODO: roll an implementation for Win32
// 
class rw_lock
{
public:
    rw_lock(void) {
        if (pthread_rwlock_init(&_rwl, NULL))
            throw std::system_error(
                ENOLCK, std::system_category(), 
                "cannot create reader/writer lock");
    }

    ~rw_lock(void) {
        pthread_rwlock_destroy(&_rwl);
    }

    // Acquires a read lock. If another thread currently holds a write lock, waits
    // until the write lock is released.
    void read_lock(void) {
        if (pthread_rwlock_rdlock(&_rwl)) 
            throw std::system_error(
                ENOLCK, std::system_category(), "cannot lock reader/writer lock");
    }

    // Tries to acquire a read lock. Immediately returns true if successful, or false
    // if another thread currently holds a write lock.
    bool read_trylock(void) {
        int rc = pthread_rwlock_tryrdlock(&_rwl);
        if (rc == 0) return true;
        else if (rc == EBUSY) return false;
        else throw std::system_error(
            ENOLCK, std::system_category(), "cannot lock reader/writer lock");
    }

    // Acquires a write lock. If one or more other threads currently hold locks,
    // waits until all locks are released. The results are undefined if the same
    // thread already holds a read or write lock
    void write_lock(void) {
        if (pthread_rwlock_wrlock(&_rwl)) 
            throw std::system_error(
                ENOLCK, std::system_category(), "cannot lock reader/writer lock");
    }

    // Tries to acquire a write lock. Immediately returns true if successful, or
    // false if one or more other threads currently hold locks. The result is
    // undefined if the same thread already holds a read or write lock.
    bool write_trylock(void) {
        int rc = pthread_rwlock_trywrlock(&_rwl);
        if (rc == 0) return true;
        else if (rc == EBUSY) return false;
        else throw std::system_error(
            ENOLCK, std::system_category(), "cannot lock reader/writer lock");
    }

    // Releases the read or write lock.
    void unlock(void) {
        if (pthread_rwlock_unlock(&_rwl))
            throw std::system_error(
                ENOLCK, std::system_category(), "cannot unlock mutex");
    }

private:
	pthread_rwlock_t _rwl;
};


// 
// Classes that simplifies thread synchronization with a mutex. The constructor
// accepts a mutex and locks it; the destructor unlocks the mutex.
// 
class read_lock_guard {
public:
    read_lock_guard(rw_lock &l):l(l) {
        l.read_lock();
    }
    ~read_lock_guard(void) {
        l.unlock();
    }
    rw_lock &l;
};

class write_lock_guard {
public:
    write_lock_guard(rw_lock &l):l(l) {
        l.write_lock();
    }
    ~write_lock_guard(void) {
        l.unlock();
    }
    rw_lock &l;
};


#endif /* _RWLOCK_H_ */
