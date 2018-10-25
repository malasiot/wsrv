/**
 * Multithreaded, libevent 2.x-based socket server.
 * Copyright (c) 2012 Qi Huang
 * This software is licensed under the BSD license.
 * See the accompanying LICENSE.txt for details.
 *
 * To compile: ./make
 * To run: ./echoserver_threaded
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <err.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/listener.h>
#include <event2/util.h>
#include <event2/event.h>
#include <signal.h>

#include <iostream>

#include "workqueue.hpp"
#include "threadpool.hpp"

using namespace std ;

/* Port to listen on. */
#define SERVER_PORT  5000
/* Connection backlog (# of backlogged connections to accept). */
#define CONNECTION_BACKLOG 1
/* Number of worker threads.  Should match number of CPU cores reported in
 * /proc/cpuinfo. */
#define NUM_THREADS 8

/* Behaves similarly to fprintf(stderr, ...), but adds file, line, and function
 information. */
#define errorOut(...) {\
    fprintf(stderr, "%s:%d: %s():\t", __FILE__, __LINE__, __FUNCTION__);\
    fprintf(stderr, __VA_ARGS__);\
}




struct Connection {

    void close() {
        if (fd_ >= 0) {
            ::close(fd_);
            fd_ = -1;
        }
    }

    ~Connection() {
        cout << "closing connection" << endl ;
        close();
        if ( buf_ev_ != nullptr ) {
            bufferevent_free(buf_ev_);
            buf_ev_ = nullptr;
        }
        if ( evbase_ != nullptr ) {
            event_base_free(evbase_);
            evbase_ = nullptr;
        }
        if ( output_buffer_ != nullptr ) {
            evbuffer_free(output_buffer_);
            output_buffer_ = nullptr;
        }
    }

    static void buffered_on_read(struct bufferevent *bev, void *arg) {
        Connection *client = (Connection *)arg;

        client->onRead(bev) ;

    }

    /**
     * Called by libevent when the write buffer reaches 0.  We only
     * provide this because libevent expects it, but we don't use it.
     */
    static void buffered_on_write(struct bufferevent *bev, void *arg) {
    }

    /**
     * Called by libevent when there is an error on the underlying socket
     * descriptor.
     */
    static void buffered_on_error(struct bufferevent *bev, short what, void *arg) {
        ((Connection *)arg)->close();
    }


    void onRead(bufferevent *bev) {

        char data[4096];
        int nbytes;

        /* If we have input data, the number of bytes we have is contained in
         * bev->input->off. Copy the data from the input buffer to the output
         * buffer in 4096-byte chunks. There is a one-liner to do the whole thing
         * in one shot, but the purpose of this server is to show actual real-world
         * reading and writing of the input and output buffers, so we won't take
         * that shortcut here. */
        struct evbuffer *input;
        input = bufferevent_get_input(bev);
        while (evbuffer_get_length(input) > 0) {
            /* Remove a chunk of data from the input buffer, copying it into our
             * local array (data). */
            nbytes = evbuffer_remove(input, data, 4096);
            /* Add the chunk of data from our local array (data) to the client's
             * output buffer. */
            evbuffer_add(output_buffer_, data, nbytes);

        }

        /* Send the results to the client.  This actually only queues the results
         * for sending. Sending will occur asynchronously, handled by libevent. */
        if (bufferevent_write_buffer(bev, output_buffer_)) {
            errorOut("Error sending data to client on fd %d\n", fd_);
            close() ;
        }

        close() ;
    }

    bool accept(int fd) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);

        fd_ = ::accept(fd, (struct sockaddr *)&client_addr, &client_len);

        if ( fd_ < 0 ) {
            warn("accept failed");
            return false ;
        }

        /* Set the client socket to non-blocking mode. */
        if ( evutil_make_socket_nonblocking(fd_) < 0 ) {
            warn("failed to set client socket to non-blocking");
            close();
            return false ;
        }


        /* Add any custom code anywhere from here to the end of this function
         * to initialize your application-specific attributes in the client struct.
         */

        if ((output_buffer_ = evbuffer_new()) == nullptr) {
            warn("client output buffer allocation failed");

            return false ;
        }

        if ((evbase_ = event_base_new()) == nullptr) {
            warn("client event_base creation failed");
            return false ;
        }

        /* Create the buffered event.
         *
         * The first argument is the file descriptor that will trigger
         * the events, in this case the clients socket.
         *
         * The second argument is the callback that will be called
         * when data has been read from the socket and is available to
         * the application.
         *
         * The third argument is a callback to a function that will be
         * called when the write buffer has reached a low watermark.
         * That usually means that when the write buffer is 0 length,
         * this callback will be called.  It must be defined, but you
         * don't actually have to do anything in this callback.
         *
         * The fourth argument is a callback that will be called when
         * there is a socket error.  This is where you will detect
         * that the client disconnected or other socket errors.
         *
         * The fifth and final argument is to store an argument in
         * that will be passed to the callbacks.  We store the client
         * object here.
         */
        buf_ev_ = bufferevent_socket_new(evbase_, fd_, BEV_OPT_CLOSE_ON_FREE);
        if (buf_ev_ == nullptr) {
            warn("client bufferevent creation failed");
            return false ;
        }

        bufferevent_setcb(buf_ev_, buffered_on_read, buffered_on_write,
                          buffered_on_error, this);

        /* We have to enable it before our callbacks will be
         * called. */
        bufferevent_enable(buf_ev_, EV_READ);

        return true ;

    }

    void dispatch() {
        event_base_dispatch(evbase_);
    }
    /* The client's socket. */
    int fd_ = -1 ;

    /* The event_base for this client. */
    struct event_base *evbase_ = nullptr;

    /* The bufferedevent for this client. */
    struct bufferevent *buf_ev_ = nullptr;

    /* The output buffer for this client. */
    struct evbuffer *output_buffer_ = nullptr;
};

static struct event_base *evbase_accept;
static workqueue_t workqueue;

/* Signal handler function (defined below). */
static void sighandler(int signal);



void on_accept(evutil_socket_t fd, short ev, void *arg) {

    dispatch_queue *pool = (dispatch_queue *)arg ;
    Connection *client = new Connection();
    if ( !client->accept(fd) )  {
        delete client ;
        return ;
    }


    pool->dispatch([client](){
 event_base_dispatch(client->evbase_);

cout << "ok here" << endl ;
    });



#if 0
    /* Create a job object and add it to the work queue. */
    if ((job = (job_t *)malloc(sizeof(*job))) == NULL) {
        warn("failed to allocate memory for job state");
        closeAndFreeClient(client);
        return;
    }
    job->job_function = server_job_function;
    job->user_data = client;

    workqueue_add_job(workqueue, job);
 #endif
}

/**
 * Run the server.  This function blocks, only returning when the server has
 * terminated.
 */

int runServer(void) {

    dispatch_queue pool(10) ;

    evutil_socket_t listenfd;
    struct sockaddr_in listen_addr;
    struct event *ev_accept;
    int reuseaddr_on;

    /* Set signal handlers */
    sigset_t sigset;
    sigemptyset(&sigset);
    struct sigaction siginfo = {
     sighandler,
        sigset,
        SA_RESTART,
    };
    sigaction(SIGINT, &siginfo, NULL);
    sigaction(SIGTERM, &siginfo, NULL);

    /* Create our listening socket. */
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd < 0) {
        err(1, "listen failed");
    }

    memset(&listen_addr, 0, sizeof(listen_addr));
    listen_addr.sin_family = AF_INET;
    listen_addr.sin_addr.s_addr = INADDR_ANY;
    listen_addr.sin_port = htons(SERVER_PORT);
    if (bind(listenfd, (struct sockaddr *)&listen_addr, sizeof(listen_addr))
        < 0) {
        err(1, "bind failed");
    }
    if (listen(listenfd, CONNECTION_BACKLOG) < 0) {
        err(1, "listen failed");
    }
    reuseaddr_on = 1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &reuseaddr_on,
               sizeof(reuseaddr_on));

    /* Set the socket to non-blocking, this is essential in event
     * based programming with libevent. */
    if (evutil_make_socket_nonblocking(listenfd) < 0) {
        err(1, "failed to set server socket to non-blocking");
    }

    if ((evbase_accept = event_base_new()) == NULL) {
        perror("Unable to create socket accept event base");
        close(listenfd);
        return 1;
    }


    /* Initialize work queue. */
#if 0
    if (workqueue_init(&workqueue, NUM_THREADS)) {
        perror("Failed to create work queue");
        close(listenfd);
        workqueue_shutdown(&workqueue);
        return 1;
    }
#endif
    /* We now have a listening socket, we create a read event to
     * be notified when a client connects. */
    ev_accept = event_new(evbase_accept, listenfd, EV_READ|EV_PERSIST,
                          on_accept, (void *)&pool);
    event_add(ev_accept, NULL);

    printf("Server running.\n");

    /* Start the event loop. */
    event_base_dispatch(evbase_accept);

    event_base_free(evbase_accept);
    evbase_accept = NULL;

    close(listenfd);

    printf("Server shutdown.\n");

    return 0;
}

/**
 * Kill the server.  This function can be called from another thread to kill
 * the server, causing runServer() to return.
 */
void killServer(void) {
    fprintf(stdout, "Stopping socket listener event loop.\n");
    if (event_base_loopexit(evbase_accept, NULL)) {
        perror("Error shutting down server");
    }
    fprintf(stdout, "Stopping workers.\n");
    workqueue_shutdown(&workqueue);
}

static void sighandler(int signal) {
    fprintf(stdout, "Received signal %d: %s.  Shutting down.\n", signal,
            strsignal(signal));
    killServer();
}

/* Main function for demonstrating the echo server.
 * You can remove this and simply call runServer() from your application. */
int main(int argc, char *argv[]) {
    return runServer();
}
