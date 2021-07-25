#ifndef THREAD_POOL_HPP
#define THREAD_POOL_HPP

#include <iostream>
#include <cinttypes>
#include <vector>
#include <thread>
#include <functional>



class ThreadPool
{
	public:
		ThreadPool() = default;
		explicit ThreadPool(const std::uint32_t& size);
		ThreadPool(const ThreadPool& obj) = delete;
		ThreadPool(ThreadPool&& obj) = delete;
		~ThreadPool();

		ThreadPool& operator=(const ThreadPool& obj) = delete;
		ThreadPool& operator=(ThreadPool&& obj) = delete;


		bool running() const;
		std::uint32_t size() const;
		void reset();
		template<typename Function> void add(Function&& function);


	private:
		void execute(const std::uint32_t index);
		std::uint32_t getFreeThread();


	private:
		bool isRunnning = false;

		std::vector<std::thread> threads;
		std::vector<bool> threadsToExecute;
		std::vector<std::function<void()>> functionsToExecute;
};


ThreadPool::ThreadPool(const std::uint32_t& size)
{
	threads.resize(size);
	functionsToExecute.resize(size);
	threadsToExecute.resize(size);

	isRunnning = true;

	for (std::uint32_t i = 0; i < size; ++i)
	{
		threadsToExecute.at(i) = false;
		threads.at(i) = std::thread(&ThreadPool::execute, this, i);
	}
}


ThreadPool::~ThreadPool()
{
	isRunnning = false;

	for (std::thread& singleThread : threads)
	{
		singleThread.join();
	}
}


void ThreadPool::reset()
{
	isRunnning = false;

	for (std::thread& singleThread : threads)
	{
		singleThread.join();
	}

	threads.clear();
	threadsToExecute.clear();
	functionsToExecute.clear();
}


template<typename Function>
void ThreadPool::add(Function&& function)
{
	std::uint32_t freeThread = 0;
	while (freeThread == 0) { freeThread = getFreeThread(); }

	freeThread -= 1;
	functionsToExecute.at(freeThread) = std::move(std::function<void()>(function));
	threadsToExecute.at(freeThread) = true;
}


bool ThreadPool::running() const
{
	if (threads.size() == 0) { return false; }
	
	for (const std::thread& singleThread : threads)
	{
		if (singleThread.joinable() == false) { return false; }
	}

	return true;
}


std::uint32_t ThreadPool::size() const
{
	return threads.size();
}


std::uint32_t ThreadPool::getFreeThread()
{
	for (std::uint32_t i = 0; i < threadsToExecute.size(); ++i)
	{
		if (threadsToExecute.at(i) == false) { return (i + 1); }
	}

	return 0;
}


void ThreadPool::execute(const std::uint32_t index)
{
	while (isRunnning == true)
	{
		if (threadsToExecute.at(index) == true)
		{
			functionsToExecute.at(index)();
			threadsToExecute.at(index) = false;
		}
	}
}



#endif  // THREAD_POOL_HPP