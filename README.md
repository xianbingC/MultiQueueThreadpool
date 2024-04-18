# MultiQueueThreadpool

implementation of multi queue thread pool by C++17

if you want thread pool with one task queue, you can code as follow:
```cpp

MultiQueueThreadPool tp(7, SINGLE_QUEUE);
tp.enqueue(0, taskFunc, Args);

```

if you want thread pool with multi queue, you can code as follow:

```cpp
MultiQueueThreadPool tp(3, MULTIPLE_QUEUE);
// or
// MultiQueueThreadPool tp(3);


tp.enqueue(-1, taskFunc, Args); // task id automatic increase
// or
// tp.enqueue(task_id, taskFunc, Args); // specify task id
``` 
