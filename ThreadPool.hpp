#ifndef THREAD_POOL_HPP
#define THREAD_POOL_HPP

#include <vector>
#include <thread>
#include <functional>
#include <shared_mutex>



class ThreadPool
{
	public:
		ThreadPool() = default;
		explicit ThreadPool(const std::size_t& size);
		ThreadPool(const ThreadPool& obj) = delete;
		ThreadPool(ThreadPool&& obj) = delete;
		~ThreadPool();

		ThreadPool& operator=(const ThreadPool& obj) = delete;
		ThreadPool& operator=(ThreadPool&& obj) = delete;

		std::size_t addTask(const std::function<void()>& function);
		std::size_t addTask(std::function<void()>&& function);
		bool isFreeThread() const;
		std::size_t freeThreads() const;
		std::size_t size() const;


	private:
		void execute(const std::size_t index);
		std::vector<std::function<void()>>::iterator getFreeThread(std::size_t& index);
	
		bool isRunning_ = false;
		std::vector<std::thread> threads_;
		std::vector<bool> threadsToExecute_;
		std::vector<std::function<void()>> functionsToExecute_;
		mutable std::shared_mutex threadLock_;
};


#endif  // THREAD_POOL_HPP