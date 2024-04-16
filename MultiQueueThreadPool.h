#pragma once

#include <vector>
#include <functional>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <memory>
#include <atomic>
#include <queue>


class MultiQueueThreadPool {
public:
	MultiQueueThreadPool(size_t size);
	~MultiQueueThreadPool();

	template<class F, class... Args>
	void enqueue(long long task_id, F&& func, Args&&... args);

	MultiQueueThreadPool(const MultiQueueThreadPool&) = delete;
	MultiQueueThreadPool& operator=(const MultiQueueThreadPool&) = delete;

private:
	size_t m_size;
	std::atomic<size_t> m_cur_qid;
	std::atomic<bool> m_stop;
	std::vector< std::unique_ptr<std::mutex> > m_mtxs;
	std::vector< std::unique_ptr<std::condition_variable> > m_conds;
	std::vector< std::queue< std::function<void()> > > m_task_queues;
	std::vector<std::thread> m_workers;
};

MultiQueueThreadPool::MultiQueueThreadPool(size_t size = 5) 
 : m_size(size)
{
	m_cur_qid.store(0);
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
		m_workers.emplace_back(
			[this, i]{
				// qid equale to thread id(i)
				size_t qid = i;
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
			}
		);
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
	if (task_id < 0) {
		qid = m_cur_qid.load();
		m_cur_qid.store((qid + 1) % m_size);
	} else {
		qid = task_id % m_size;
	}
	std::function<void()> task = std::bind(std::forward<F>(func), std::forward<Args>(args)...);

	std::unique_lock<std::mutex> lock(*m_mtxs[qid]);
	m_task_queues[qid].push(std::move(task));
	m_conds[qid]->notify_one();
}


