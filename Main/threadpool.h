#pragma once

#include <future>
#include <vector>
#include <thread>
#include <queue>
#include <condition_variable>

class ThreadPool
{
	public:
		explicit ThreadPool(std::size_t);
		~ThreadPool();

		template<class T, class... Args>
		auto enqueue(T&& task, Args&&... args)->std::future<decltype(task(args...))>;

	private:

		using Task = std::function<void()>;
		//  the thread pool 
		std::vector<std::thread> threadPool;
		// tasks queue
		std::queue<Task> tasksQueue;
		std::condition_variable eventVar;
		std::mutex eventMutex;
		std::atomic<bool> stoped;
};

ThreadPool::ThreadPool(std::size_t numThreads) : stoped(false)
{
	for (size_t i = 0; i < numThreads; ++i)
	{
		threadPool.emplace_back([=] {
			while (true)
			{
				Task task;

				{
					std::unique_lock<std::mutex> lock{ eventMutex };

					eventVar.wait(lock, [=] {
						return stoped || !tasksQueue.empty();
						});

					// wait until there is task
					if (stoped && tasksQueue.empty())
						break;

					// take one, first task
					task = std::move(tasksQueue.front());
					tasksQueue.pop();
				}

				// run task
				task();
			}
			});
	}
}

ThreadPool::~ThreadPool()
{
	{
		std::unique_lock<std::mutex> lock{ eventMutex };
		stoped = true;
	}

	eventVar.notify_all();

	for (auto& thread : threadPool)
		thread.join();
}

template<class T, class... Args>

auto ThreadPool::enqueue(T&& task, Args&&... args)->std::future<decltype(task(args...))>
{
	auto wrapper = std::make_shared <std::packaged_task <decltype(task(args...)) () > >(std::bind(std::forward<T>(task), std::forward<Args>(args)...));

	{
		// add task to the tasks queue
		std::unique_lock<std::mutex> lock{ eventMutex };
		tasksQueue.emplace([=] {
			(*wrapper)();
			});
	}

	// unblock one of the waiting threads
	eventVar.notify_one();
	return wrapper->get_future();
}