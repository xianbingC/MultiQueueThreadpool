#pragma once

#include <vector>
#include <functional>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <memory>
#include <atomic>
#include <queue>

#define SINGLE_QUEUE   true
#define MULTIPLE_QUEUE false

class MultiQueueThreadPool {
public:
  MultiQueueThreadPool(size_t size, bool is_single_queue);
  ~MultiQueueThreadPool();

  template<class F, class... Args>
  void enqueue(long long task_id, F&& func, Args&&... args);

  MultiQueueThreadPool(const MultiQueueThreadPool&) = delete;
  MultiQueueThreadPool& operator=(const MultiQueueThreadPool&) = delete;

private:
  size_t m_size;
  size_t m_cur_qid;
	bool   m_is_single_queue;
  std::atomic<bool> m_stop;
  std::vector< std::unique_ptr<std::mutex> > m_mtxs;
  std::vector< std::unique_ptr<std::condition_variable> > m_conds;
  std::vector< std::queue< std::function<void()> > > m_task_queues;
  std::vector<std::thread> m_workers;
  std::mutex m_qid_mtx;
};

MultiQueueThreadPool::MultiQueueThreadPool(size_t size, bool is_single_queue = MULTIPLE_QUEUE)
 : m_size(size)
 , m_cur_qid(0)
 , m_is_single_queue(is_single_queue)
{
  m_stop.store(false);
  m_task_queues.resize(size);
  m_mtxs.resize(size);
  m_conds.resize(size);

  // create mutex/cond_var
  for (int i = 0; i < size; ++i) {
    m_mtxs[i] = std::unique_ptr<std::mutex>(new std::mutex); 
    m_conds[i] = std::unique_ptr<std::condition_variable>(new std::condition_variable);
  }

  // create worker thread
  for (size_t i = 0; i < size; ++i) {
    m_workers.emplace_back([this, i]{
      // qid equale to thread id(i)
      size_t qid = i;
			if (m_is_single_queue) {
				qid = 0; // fixed queue
			}
      while(true) {
        std::unique_lock<std::mutex> lock(*m_mtxs[qid]);
        m_conds[qid]->wait(lock, [this, qid]{ return m_stop.load() || !m_task_queues[qid].empty(); });
        
        if (m_stop.load()) {
          break;
        }
  
        auto task = std::move(m_task_queues[qid].front());
        m_task_queues[qid].pop();
        lock.unlock();

        // execute task
        task();
      }
    });
  }
}

MultiQueueThreadPool::~MultiQueueThreadPool()
{
  m_stop.store(true);
  for (size_t i = 0; i < m_size; ++i) {
    m_conds[i]->notify_all();
    m_workers[i].join();
  }
}

template<class F, class... Args>
void MultiQueueThreadPool::enqueue(long long task_id, F&& func, Args&&... args)
{
  size_t qid = 0;
	if (m_is_single_queue) {
		qid = 0;
	} else if (task_id < 0) {
    m_qid_mtx.lock();
    qid = m_cur_qid;
    m_cur_qid = (qid + 1) % m_size;
    m_qid_mtx.unlock();
  } else {
    qid = task_id % m_size;
  }
  std::function<void()> task = std::bind(std::forward<F>(func), std::forward<Args>(args)...);

  std::unique_lock<std::mutex> lock(*m_mtxs[qid]);
  m_task_queues[qid].push(std::move(task));
  m_conds[qid]->notify_one();
}


