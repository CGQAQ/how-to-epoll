# how-to-epoll

## what is epoll
> [epoll - I/O event notification facility](https://man7.org/linux/man-pages/man7/epoll.7.html) <br/>

The epoll API performs a similar task to poll(2): monitoring
   multiple file descriptors to see if I/O is possible on any of
   them.  The epoll API can be used either as an edge-triggered or a
   level-triggered interface and scales well to large numbers of
   watched file descriptors.
The central concept of the epoll API is the epoll instance, an
   in-kernel data structure which, from a user-space perspective,
   can be considered as a container for two lists:

   - The interest list (sometimes also called the epoll set): the
     set of file descriptors that the process has registered an
     interest in monitoring.

   - The ready list: the set of file descriptors that are "ready"
     for I/O.  The ready list is a subset of (or, more precisely, a
     set of references to) the file descriptors in the interest
     list.  The ready list is dynamically populated by the kernel as
     a result of I/O activity on those file descriptors.

## header file
`sys/epoll.h`

## epoll vs select

Quote from [David Schwartz](https://stackoverflow.com/a/17355702), license
see [CC BY-SA 3.0](https://creativecommons.org/licenses/by-sa/3.0/)
> There's a lot of misinformation about this, but the real reason is this:
>
> A typical server might be dealing with, say, 200 connections. It will service every connection that needs to have data
> written or read, and then it will need to wait until there's more work to do. While it's waiting, it needs to be
> interrupted if data is received on any of those 200 connections.
> With select, the kernel has to add the process to 200 wait lists, one for each connection. To do this, it needs a "
> thunk" to attach the process to the wait list. When the process finally does wake up, it needs to be removed from all
> 200 wait lists and all those thunks need to be freed.
>
> By contrast, with epoll, the epoll socket itself has a wait list. The process needs to be put on only that one wait
> list using only one thunk. When the process wakes up, it needs to be removed from only one wait list and only one
> thunk
> needs to be freed.
> To be clear, with epoll, the epoll socket itself has to be attached to each of those 200 connections. But this is done
> once, for each connection, when it is accepted in the first place. And this is torn down once, for each connection,
> when
> it is removed. By contrast, each call to select that blocks must add the process to every wait queue for every socket
> being monitored.
>
> Ironically, with select, the largest cost comes from checking if sockets that have had no activity have had any
> activity. With epoll, there is no need to check sockets that have had no activity because if they did have activity,
> they would have informed the epoll socket when that activity happened. In a sense, select polls each socket each time
> you call select to see if there's any activity while epoll rigs it so that the socket activity itself notifies the
> process.

## interface

The epoll interface is declared at `sys/epoll.h` header file, and was fist introduced in
the [Linux kernel version 2.5.44](https://en.wikipedia.org/wiki/Epoll#:~:text=first%20introduced%20in%20version%202.5.44%20of%20the%20Linux%20kernel.%5B1%5D)

The epoll interface conains three major function:

```c
// Creates an epoll object and returns its file descriptor. The flags parameter allows epoll behavior to be modified. It has only one valid value, EPOLL_CLOEXEC. epoll_create() is an older variant of 
// epoll_create1() and is deprecated as of Linux kernel version 2.6.27 and glibc version 2.9.[4]
int epoll_create1(int flags);

// Controls (configures) which file descriptors are watched by this object, and for which events. op can be ADD, MODIFY or DELETE.
int epoll_ctl(int epfd, int op, int fd, struct epoll_event *event);

// Waits for any of the events registered for with epoll_ctl, until at least one occurs or the timeout elapses. Returns the occurred events in events, up to maxevents at once.
int epoll_wait(int epfd, struct epoll_event *events, int maxevents, int timeout);
```

### epoll_create vs epoll_create1, which should I use

Quote from [paxdiablo](https://stackoverflow.com/a/10011348), license
see [CC BY-SA 3.0](https://creativecommons.org/licenses/by-sa/3.0/)
> With `epoll_wait()`, `maxevents` tells you you maximum number of events that will be returned to you. It has nothing
> to do with how many are maintained within the kernel.
>
> Older versions of `epoll_create()` used the size to set certain limits but that's no longer done, hence the comment
> that the `size` argument is obsolete. This is made evident by the source code (
> in [fs/eventpoll.c](https://github.com/torvalds/linux/blob/master/fs/eventpoll.c#L2093) as at the time of this
> answer):
> ```c
> SYSCALL_DEFINE1(epoll_create1, int, flags) {
>     return do_epoll_create(flags);
> }
> SYSCALL_DEFINE1(epoll_create, int, size) {
>     if (size <= 0) return -EINVAL;
>     return do_epoll_create(0);
> }
> ```
> You can see that they're almost identical except that:
> - `epoll_create1()` accepts `flags`, passing them on to `do_epoll_create()`;
> - `epoll_create()` accepts `size`, checking it, but otherwise ignoring it;
> - `epoll_create()` passes default flags (none) to `do_epoll_create()`.
    > Hence the advantage of using `epoll_create1()` is that it allows you to specify the flags, which I think are
    currently limited to close-on-exec (so that the file descriptor is automatically closed when `exec`-ing another
    program).

So the `size` if only checked and had been ignored immediately, I think you should always use the epoll_create1 for such
reason

## example

See [here](./main.c)
Run with `make run-dir`, do file operations in the `test_data` folder, and see what file operations have been made in the terminal <br/>
![image](https://github.com/CGQAQ/how-to-epoll/assets/15936231/62e0da0c-74a2-4801-842b-48f8d6788ef0)
