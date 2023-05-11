#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/inotify.h>
#include <unistd.h>

#include "utils.h"

#define MAX_EVENTS (1024)
#define EVENT_SIZE (sizeof(struct inotify_event))
#define BUF_LEN (MAX_EVENTS * (EVENT_SIZE + 16))

int main(int argc, char* argv[]) {
    int result = 0;
    const int inotify_fd = inotify_init();

    // 1. create the `epoll file descriptor`
    // EPOLL_CLOEXEC
    //  Set the close-on-exec (FD_CLOEXEC) flag on the new file
    //  descriptor.  See the description of the O_CLOEXEC flag in
    //  open(2) for reasons why this may be useful.
    // On success, these system calls return a file descriptor (a
    // nonnegative integer).  On error, -1 is returned, and `errno` is set
    // to indicate the error.
    int epfd = epoll_create1(0);

    if (epfd == -1) {
        // ERRORS         top
        //    EINVAL size is not positive.
        //
        //    EINVAL (epoll_create1()) Invalid value specified in flags.
        //
        //    EMFILE The per-user limit on the number of epoll instances
        //   imposed by /proc/sys/fs/epoll/max_user_instances was
        //   encountered.  See epoll(7) for further details.
        //
        //    EMFILE The per-process limit on the number of open file
        //   descriptors has been reached.
        //
        //    ENFILE The system-wide limit on the total number of open
        //    files
        //   has been reached.
        //

        //    ENOMEM There was insufficient memory to create the kernel
        //    object.
        switch (errno) {
            case EINVAL:
                eprintf(
                    "EINVAL (epoll_create1()) Invalid value specified in "
                    "flags.");
                break;
            case EMFILE:
                eprintf("Limit has reached");
                break;

            case ENOMEM:
                eprintf(
                    "ENOMEM There was insufficient memory to create the kernel "
                    "object.");
                break;
            default:
                unreachable("epoll_create1 return unexpected errno");
                break;
        }
        return RET_CODE_ERROR;
    }

    // // 2. get the file descriptor you interested in.
    // int fd = open("./test.txt", O_CREAT);

    // if (fd == -1) {
    //     goto error;
    // }

    int watch_descriptor = inotify_add_watch(
        inotify_fd, "./test.txt",
        IN_ACCESS | IN_ATTRIB | IN_MODIFY | IN_OPEN | IN_CREATE | IN_DELETE |
            IN_CLOSE_WRITE | IN_CLOSE_NOWRITE);

    if (watch_descriptor == -1) goto error;

    // 3. put the inotify file descriptor you interested in into the epfd that
    // we created before's interest list so that we can be notified as soon as
    // the inotify receives events
    struct epoll_event ep_ev_file;
    ep_ev_file.events = EPOLLIN;
    ep_ev_file.data.fd = inotify_fd;
    result = epoll_ctl(epfd, EPOLL_CTL_ADD, inotify_fd, &ep_ev_file);
    if (result == -1) {
        // ref: https://man7.org/linux/man-pages/man2/epoll_ctl.2.html
        // ERRORS section
        switch (errno) {
            case EBADF:
                eprintf("EBADF  epfd or fd is not a valid file descriptor.");
                break;
            case EEXIST:
                eprintf(
                    "EEXIST op was EPOLL_CTL_ADD, and the supplied file "
                    "descriptor fd");
                break;
            case EINVAL:
                eprintf(
                    "EINVAL epfd is not an epoll file descriptor, or fd is the "
                    "same as"
                    "epfd, or the requested operation op is not supported by"
                    "this interface.");
                eprintf("\nOR\n");
                eprintf(
                    "EINVAL An invalid event type was specified along with"
                    "EPOLLEXCLUSIVE in events.");
                eprintf("\nOR\n");
                eprintf(
                    "EINVAL op was EPOLL_CTL_MOD and events included "
                    "EPOLLEXCLUSIVE.");
                eprintf("\nOR\n");
                eprintf(
                    "EINVAL op was EPOLL_CTL_MOD and the EPOLLEXCLUSIVE flag "
                    "has"
                    "previously been applied to this epfd, fd pair.");
                eprintf("\nOR\n");
                eprintf(
                    "EPOLLEXCLUSIVE was specified in event and fd refers to an"
                    "epoll instance.");
                break;
            case ELOOP:
                eprintf(
                    "fd refers to an epoll instance and this EPOLL_CTL_ADD"
                    "operation would result in a circular loop of epoll"
                    "instances monitoring one another or"
                    "a nesting depth of epoll instances greater than 5. ");
                break;
            case ENOENT:
                eprintf(
                    "ENOMEM There was insufficient memory to handle the "
                    "requested op"
                    "control operation.");
                break;
            case ENOSPC:
                eprintf(
                    "ENOSPC The limit imposed by "
                    "/proc/sys/fs/epoll/max_user_watches"
                    "was encountered while trying to register "
                    "(EPOLL_CTL_ADD) a"
                    "new file descriptor on an epoll instance.  See "
                    "epoll(7)"
                    "for further details.");
                break;
            case EPERM:
                eprintf(
                    "EPERM  The target file fd does not support epoll.  This "
                    "error can"
                    "occur if fd refers to, for example, a regular file or a"
                    "directory.");
                break;
            default:
                unreachable("epoll_ctl return unexpected errno");
        }
        return RET_CODE_ERROR;
    }

    // struct epoll_event ep_ev_stdin;
    // ep_ev_stdin.events = EPOLLIN;
    // ep_ev_stdin.data.fd = STDIN_FILENO;
    // // for Ctrl+C and more...
    // result = epoll_ctl(epfd, EPOLL_CTL_ADD, STDIN_FILENO, &ep_ev_stdin);
    // if (result == -1) {
    //     // ref: https://man7.org/linux/man-pages/man2/epoll_ctl.2.html
    //     // ERRORS section
    //     switch (errno) {
    //         case EBADF:
    //             eprintf("EBADF  epfd or fd is not a valid file descriptor.");
    //             break;
    //         case EEXIST:
    //             eprintf(
    //                 "EEXIST op was EPOLL_CTL_ADD, and the supplied file "
    //                 "descriptor fd");
    //             break;
    //         case EINVAL:
    //             eprintf(
    //                 "EINVAL epfd is not an epoll file descriptor, or fd is
    //                 the " "same as" "epfd, or the requested operation op is
    //                 not supported by" "this interface.");
    //             eprintf("\nOR\n");
    //             eprintf(
    //                 "EINVAL An invalid event type was specified along with"
    //                 "EPOLLEXCLUSIVE in events.");
    //             eprintf("\nOR\n");
    //             eprintf(
    //                 "EINVAL op was EPOLL_CTL_MOD and events included "
    //                 "EPOLLEXCLUSIVE.");
    //             eprintf("\nOR\n");
    //             eprintf(
    //                 "EINVAL op was EPOLL_CTL_MOD and the EPOLLEXCLUSIVE flag
    //                 " "has" "previously been applied to this epfd, fd
    //                 pair.");
    //             eprintf("\nOR\n");
    //             eprintf(
    //                 "EPOLLEXCLUSIVE was specified in event and fd refers to
    //                 an" "epoll instance.");
    //             break;
    //         case ELOOP:
    //             eprintf(
    //                 "fd refers to an epoll instance and this EPOLL_CTL_ADD"
    //                 "operation would result in a circular loop of epoll"
    //                 "instances monitoring one another or"
    //                 "a nesting depth of epoll instances greater than 5. ");
    //             break;
    //         case ENOENT:
    //             eprintf(
    //                 "ENOMEM There was insufficient memory to handle the "
    //                 "requested op"
    //                 "control operation.");
    //             break;
    //         case ENOSPC:
    //             eprintf(
    //                 "ENOSPC The limit imposed by "
    //                 "/proc/sys/fs/epoll/max_user_watches"
    //                 "was encountered while trying to register "
    //                 "(EPOLL_CTL_ADD) a"
    //                 "new file descriptor on an epoll instance.  See "
    //                 "epoll(7)"
    //                 "for further details.");
    //             break;
    //         case EPERM:
    //             eprintf(
    //                 "EPERM  The target file fd does not support epoll.  This
    //                 " "error can" "occur if fd refers to, for example, a
    //                 regular file or a" "directory.");
    //             break;
    //         default:
    //             unreachable("epoll_ctl return unexpected errno");
    //     }
    //     return RET_CODE_ERROR;
    // }

    // 4. waiting for events comes in
    // The maxevents argument must be greater than zero.

    // The timeout argument specifies the number of milliseconds that
    // epoll_wait()
    //  will block.Time is measured against the CLOCK_MONOTONIC
    //    clock.
    struct epoll_event events[MAX_EVENTS];
    char buffer[BUF_LEN];
    for (;;) {
        result = epoll_wait(
            epfd /* int __epfd */, events /* struct epoll_event * __events */,
            MAX_EVENTS /* int maxevents */, 50 /* int timeout */);

        if (result == -1) {
            // from stdin
            if (errno == EINTR) {
                continue;
            }
            goto error;
        }

        if (result > 0) {
            int length = 0;
            LOG("result: %d\n", result);
            for (int i = 0; i < result; ++i) {
                struct epoll_event* ev = &events[i];
                LOG("ev->events: %d\n", ev->events);

                // LOG("ev->data.fd: %d\n", ev->data.fd);
                // LOG("inotify_fd: %d\n", inotify_fd);
                if (ev->data.fd == inotify_fd) {
                    length = read(inotify_fd /*int fd*/, buffer /*void *buf*/,
                                  BUF_LEN /*size_t nbytes*/);

                    if (length == -1) {
                        goto error;
                    }

                    for (struct inotify_event* event =
                             (struct inotify_event*)buffer;
                         (char*)event < buffer + length;
                         event += EVENT_SIZE + event->len) {
                        if (event->len) {
                            if (event->mask & IN_CREATE) {
                                if (event->mask & IN_ISDIR) {
                                    printf("Directory %s was created.\n",
                                           event->name);
                                } else {
                                    printf("File %s was created.\n",
                                           event->name);
                                }
                            } else if (event->mask & IN_DELETE) {
                                if (event->mask & IN_ISDIR) {
                                    printf("Directory %s was deleted.\n",
                                           event->name);
                                } else {
                                    printf("File %s was deleted.\n",
                                           event->name);
                                }
                            } else if (event->mask & IN_MODIFY) {
                                if (event->mask & IN_ISDIR) {
                                    printf("Directory %s was modified.\n",
                                           event->name);
                                } else {
                                    printf("File %s was modified.\n",
                                           event->name);
                                }
                            }
                        }
                    }
                } else if (ev->data.fd == STDIN_FILENO) {
                    // LOG("STDIN_FILENO not handled yet \n");
                } else {
                    unreachable("unexpected fd found");
                }
            }
        }
    }

    return RET_CODE_SUCCESS;

error:
    // trivial error handler
    eprintf("Something went wrong %s", strerror(errno));
    return RET_CODE_ERROR;
}