#include "Stopper.h"

#include <deque>
#include <mutex>
#include <condition_variable>


// ===== Blocking [thread safe] queue
template <typename T>
class ConcurrentQueue
{
	ConcurrentQueue(const ConcurrentQueue&);
	const ConcurrentQueue& operator= (const ConcurrentQueue&);

public:
	ConcurrentQueue() : m_stopper(NoStop::Instance()) {};
	ConcurrentQueue(IStopper& stopper) : m_stopper(stopper) {};

	void push(T&& value)
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		m_queue.push_back(std::move(value));
		m_cvHasData.notify_one();
	}

	bool pop(T& value)
	{
		std::unique_lock<std::mutex> lock(m_mutex);
		m_cvHasData.wait(lock, [&] { return !m_queue.empty() || m_stopper; });
		if (m_stopper)
			return false;
		value = std::move(m_queue.front());
		m_queue.pop_front();
		return true;
	}

	void stop()
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		m_stopper.stop();
		m_cvHasData.notify_all();
	}

	size_t size()
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		return m_queue.size();
	}

	void clear()
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		m_queue.clear();
	}

private:
	std::mutex m_mutex;
	std::condition_variable m_cvHasData;
	std::deque< T > m_queue;
	IStopper& m_stopper;
};

