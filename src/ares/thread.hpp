// Copyright (C) 2002-2007 Daniel Cowgill
//
// Usage of the works is permitted provided that this instrument is retained
// with the works, so that any entity that uses the works is notified of this
// instrument.
//
// DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

#ifndef included_ares_thread
#define included_ares_thread

#include "ares/platform.hpp"
#include "ares/utility.hpp"

namespace ares {

class Thread : boost::noncopyable {
  public:
    // This interface represents the thread "main" function.
    struct Runnable {
        virtual ~Runnable() {}
        virtual void run() = 0;
    };

    Thread(Runnable* r);
    ~Thread();
    void start();
    void wait_for_exit(int millis);
    void join();
    void suspend();
    void resume();
    bool is_running() const;
    Thread_id id() const;
    static Thread_id current_thread_id();

  private:
    pthread_t m_thread;     // platform-specific thread type
    pthread_attr_t m_attr;  // thread attribute type
    Runnable* m_runnable;   // runnable object handle
    bool m_running;         // true if thread is in Runnable::run

    static void* thread_wrapper(void*);
};

// Container for per-thread global data.
template<typename T>
class Thread_specific_value : boost::noncopyable {
  public:
    Thread_specific_value();
    ~Thread_specific_value();
    T* get();
    T* operator->() { return get(); }
    T& operator*() { return *get(); }
    void reset(T* p = 0);

  private:
    pthread_key_t m_key;
};


// #########################################################################
// The following consists of inline function definitions for this component.
// #########################################################################

template<typename T>
Thread_specific_value<T>::Thread_specific_value()
{
    if (pthread_key_create(&m_key, 0))
        throw System_error("pthread_key_create", errno);
}

template<typename T>
Thread_specific_value<T>::~Thread_specific_value()
{
    pthread_key_delete(m_key);
}

template<typename T>
T* Thread_specific_value<T>::get()
{
    return static_cast<T*>(pthread_getspecific(m_key));
}

template<typename T>
void Thread_specific_value<T>::reset(T* p)
{
    if (pthread_setspecific(m_key, p))
        throw System_error("pthread_setspecific", errno);
}

} // namespace ares

#endif
