# MultiQueueThreadpool

implementation of multi queue thread pool by C++11

if you want thread pool with one task queue, you can code as follow:
```cpp

MultiQueueThreadPool tp(7);
tp.enqueue(0, taskFunc, Args);  // fixed taskid 0

```

if you want thread pool with multi queue, you can code as follow:

```cpp
MultiQueueThreadPool tp(3);
tp.enqueue(-1, taskFunc, Args);

// or
tp.enqueue(task_id, taskFunc, Args); // every task has a task id
``` 
