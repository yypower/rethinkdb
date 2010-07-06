
#ifndef __IO_CALLS_HPP__
#define __IO_CALLS_HPP__

#include <libaio.h>
#include <vector>
#include <assert.h>
#include "config/args.hpp"
#include "config/code.hpp"
#include "utils.hpp"
#include "arch/resource.hpp"
#include "event.hpp"
#include "corefwd.hpp"

struct iocallback_t {
    virtual void on_io_complete(event_t *event) = 0;
};

struct io_calls_t {
public:
    io_calls_t(event_queue_t *_queue);
    virtual ~io_calls_t() {
        assert(n_pending == 0);
    }
    
    /**
     * Synchronous (but possibly non-blocking, depending on fd) API
     */
    static ssize_t read(resource_t fd, void *buf, size_t count);
    static ssize_t write(resource_t fd, const void *buf, size_t count);

    /**
     * Asynchronous API
     */
    // Submit an asynchronous read request to the OS
    void schedule_aio_read(resource_t resource,
                           size_t offset, size_t length, void *buf,
                           event_queue_t *notify_target, iocallback_t *callback);

    // Submit asynchronous write requests to the OS
    struct aio_write_t {
    public:
        resource_t      resource;
        size_t          offset;
        size_t          length;
        void            *buf;
        iocallback_t    *callback;
    };
    void schedule_aio_write(aio_write_t *writes, int num_writes, event_queue_t *notify_target);

    /**
     * AIO notification support (this is meant to be called by the
     * event queue).
     */
    void aio_notify(iocb *event, int result);

private:
    typedef std::vector<iocb*, gnew_alloc<iocb*> > request_vector_t;

    void process_requests();
    int process_request_batch(request_vector_t *requests);

private:
    event_queue_t *queue;

    request_vector_t r_requests, w_requests;

    int n_pending;

#ifndef NDEBUG
    // We need the extra pointer in debug mode because
    // tls_small_obj_alloc_accessor creates the pools to expect it
    static const size_t iocb_size = sizeof(iocb) + sizeof(void*);
#else
    static const size_t iocb_size = sizeof(iocb);
#endif

};

#endif // __IO_CALLS_HPP__

