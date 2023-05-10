# how-to-epoll


## epoll vs select
Quote from [David Schwartz](https://stackoverflow.com/a/17355702), license see [CC BY-SA 3.0](https://creativecommons.org/licenses/by-sa/3.0/)
> There's a lot of misinformation about this, but the real reason is this:
> 
> A typical server might be dealing with, say, 200 connections. It will service every connection that needs to have data written or read and then it will need to wait until there's more work to do. While it's waiting, it needs to be interrupted if data is received on any of those 200 connections.
With select, the kernel has to add the process to 200 wait lists, one for each connection. To do this, it needs a "thunk" to attach the process to the wait list. When the process finally does wake up, it needs to be removed from all 200 wait lists and all those thunks need to be freed.
>
> By contrast, with epoll, the epoll socket itself has a wait list. The process needs to be put on only that one wait list using only one thunk. When the process wakes up, it needs to be removed from only one wait list and only one thunk needs to be freed.
To be clear, with epoll, the epoll socket itself has to be attached to each of those 200 connections. But this is done once, for each connection, when it is accepted in the first place. And this is torn down once, for each connection, when it is removed. By contrast, each call to select that blocks must add the process to every wait queue for every socket being monitored.
>
> Ironically, with select, the largest cost comes from checking if sockets that have had no activity have had any activity. With epoll, there is no need to check sockets that have had no activity because if they did have activity, they would have informed the epoll socket when that activity happened. In a sense, select polls each socket each time you call select to see if there's any activity while epoll rigs it so that the socket activity itself notifies the process.
