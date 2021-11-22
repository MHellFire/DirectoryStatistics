#pragma once

#include <future>
#include <vector>
#include <thread>
#include <queue>
#include <condition_variable>

class ThreadPool
{
public:
	using Task = std::function<void()>;

	explicit ThreadPool(std::size_t numThreads)
	{
		start(numThreads);
	}

	~ThreadPool()
	{
		stop();
	}

	//template<class T>
	template<class T, class... Args>
	//	auto enqueue(T task)->std::future<decltype(task())>
	auto enqueue(T&& task, Args&&... args)->std::future<decltype(task(args...))>
	{
		auto wrapper = std::make_shared <std::packaged_task <decltype(task(args...)) () > >(std::bind(std::forward<T>(task), std::forward<Args>(args)...));

		{
			std::unique_lock<std::mutex> lock{ eventMutex };
			tasksQueue.emplace([=] {
				(*wrapper)();
				});
		}

		eventVar.notify_one();
		return wrapper->get_future();
	}

private:
	std::vector<std::thread> threads;

	std::condition_variable eventVar;

	std::mutex eventMutex;
	bool stopping = false;

	std::queue<Task> tasksQueue;

	void start(std::size_t numThreads)
	{
		for (auto i = 0u; i < numThreads; ++i)
		{
			threads.emplace_back([=] {
				while (true)
				{
					Task task;

					{
						std::unique_lock<std::mutex> lock{ eventMutex };

						eventVar.wait(lock, [=] { return stopping || !tasksQueue.empty(); });

						if (stopping && tasksQueue.empty())
							break;

						task = std::move(tasksQueue.front());
						tasksQueue.pop();
					}

					task();
				}
				});
		}
	}

	void stop() noexcept
	{
		{
			std::unique_lock<std::mutex> lock{ eventMutex };
			stopping = true;
		}

		eventVar.notify_all();

		for (auto& thread : threads)
			thread.join();
	}
};