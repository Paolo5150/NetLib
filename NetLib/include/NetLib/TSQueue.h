#pragma once
#include <mutex>
#include <deque>

template<class T>
class TSQueue
{
public:
	TSQueue() = default;
	TSQueue(const TSQueue<T>& other) = delete;
	~TSQueue() { Clear(); }

	const T& Front()
	{
		std::unique_lock<std::mutex> l(m_mutex);
		return m_q.front();
	}

	const T& Back()
	{
		std::unique_lock<std::mutex> l(m_mutex);
		return m_q.back();
	}

	void PushBack(const T& e)
	{
		std::unique_lock<std::mutex> l(m_mutex);
		m_q.emplace_back(std::move(e));

		std::unique_lock<std::mutex> l2(m_waitMtx);
		m_cv.notify_one();
	}

	void PushFront(const T& e)
	{
		std::unique_lock<std::mutex> l(m_mutex);
		m_q.emplace_front(std::move(e));

		std::unique_lock<std::mutex> l2(m_waitMtx);
		m_cv.notify_one();
	}

	size_t Size()
	{
		std::unique_lock<std::mutex> l(m_mutex);
		return m_q.size();
	}

	void Clear()
	{
		std::unique_lock<std::mutex> l(m_mutex);
		m_q.clear();
	}

	bool Empty()
	{
		std::unique_lock<std::mutex> l(m_mutex);
		return m_q.empty();
	}

	T PopFront()
	{
		std::unique_lock<std::mutex> l(m_mutex);
		auto t = std::move(m_q.front());
		m_q.pop_front();
		return t;
	}

	T PopBack()
	{
		std::unique_lock<std::mutex> l(m_mutex);
		auto t = std::move(m_q.back());
		m_q.pop_back();
		return t;
	}

	void Wait()
	{
		std::unique_lock<std::mutex> l(m_waitMtx);
		m_cv.wait(l, [this]() {

			return m_forceWake.load() || !Empty();
			});

		m_forceWake.store(false);

	}

	void ForceWake()
	{
		m_forceWake.store(true);
		std::unique_lock<std::mutex> l(m_waitMtx);
		m_cv.notify_one();

	}

private:
	std::mutex m_mutex;
	std::deque<T> m_q;

	std::condition_variable m_cv;
	std::mutex m_waitMtx;

	std::atomic<bool> m_forceWake = false;

};