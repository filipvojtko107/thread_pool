#include "ThreadPool.hpp"



ThreadPool::ThreadPool(const std::size_t& size) :
	isRunning_(true),
	threads_(size),
	threadsToExecute_(size, false),
	functionsToExecute_(size)
{
	std::size_t index = 0;
	for (auto& thread : threads_)
	{
		thread = std::thread(&ThreadPool::execute, this, index);
		index += 1;
	}
}


ThreadPool::~ThreadPool()
{
	std::size_t index = 0;
	std::unique_lock lock(threadLock_);

	for (auto& thread : threads_)
	{
		threadsToExecute_.at(index) = false;
		thread.join();
		index += 1;
	}
}


std::size_t ThreadPool::addTask(const std::function<void()>& function)
{
	std::size_t freeThreadIndex = 0;
	std::vector<std::function<void()>>::iterator freeThread;

	threadLock_.lock_shared();
	while ((freeThread = getFreeThread(freeThreadIndex)) 
			== functionsToExecute_.end()) {  }
	threadLock_.unlock_shared();

	threadLock_.lock();
	*freeThread = function;
	threadsToExecute_.at(freeThreadIndex) = true;
	threadLock_.unlock();

	return freeThreadIndex;
}

 
std::size_t ThreadPool::addTask(std::function<void()>&& function)
{
	std::size_t freeThreadIndex = 0;
	std::vector<std::function<void()>>::iterator freeThread;

	threadLock_.lock_shared();
	while ((freeThread = getFreeThread(freeThreadIndex)) 
			== functionsToExecute_.end()) {  }
	threadLock_.unlock_shared();

	threadLock_.lock();
	*freeThread = std::move(function);
	threadsToExecute_.at(freeThreadIndex) = true;
	threadLock_.unlock();

	return freeThreadIndex;
}


bool ThreadPool::isFreeThread() const
{
	std::shared_lock lock(threadLock_);
    for (const bool threadToExecute : threadsToExecute_)
    {
        if (threadToExecute == false) { threadLock_.unlock(); return true; }
    }

    return false;
}


std::size_t ThreadPool::freeThreads() const
{
	std::shared_lock lock(threadLock_);
	std::size_t count = 0;

	for (const bool item : threadsToExecute_)
	{
		if (item == true) { count += 1; }
	}

	return count;
}


std::size_t ThreadPool::size() const
{
	std::shared_lock lock(threadLock_);
	return threads_.size();
}


std::vector<std::function<void()>>::iterator ThreadPool::getFreeThread(std::size_t& index)
{
	for (auto thread = functionsToExecute_.begin(); thread != functionsToExecute_.end(); thread += 1)
	{
		if (threadsToExecute_.at(index) == false) { return thread; }
        index += 1;
	}

    index = 0;
	return functionsToExecute_.end();
}


void ThreadPool::execute(const std::size_t index)
{
	bool isRunning = true;
	bool threadToExecute = false;
	bool stopTask = false;

	while (isRunning == true)
	{
		if (threadToExecute == true)
		{
			functionsToExecute_.at(index)();
			stopTask = true;
		}

		if (stopTask == true)
		{
			stopTask = false;
			threadToExecute = false;

		 	std::unique_lock lock(threadLock_);
			threadsToExecute_.at(index) = false;
			isRunning = isRunning_;
		}

		else
		{
			std::shared_lock lock(threadLock_);
			threadToExecute = threadsToExecute_.at(index);
			isRunning = isRunning_;
		}
	}
}