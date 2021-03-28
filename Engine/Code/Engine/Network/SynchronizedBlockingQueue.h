#pragma once
#include <mutex>
#include <queue>
#include <condition_variable>

template<typename T>
class SynchronizedBlockingQueue : protected std::queue<T>
{
public:
	using value_type = T;
protected:
	using base = typename std::queue<T>;

public:
	SynchronizedBlockingQueue(): base() { }
	SynchronizedBlockingQueue(SynchronizedBlockingQueue const&&) = delete;
	SynchronizedBlockingQueue(SynchronizedBlockingQueue const&) = delete;
	~SynchronizedBlockingQueue() = default;

	SynchronizedBlockingQueue& operator=(SynchronizedBlockingQueue const&&) = delete;
	SynchronizedBlockingQueue& operator=(SynchronizedBlockingQueue const&) = delete;

	void Push(const value_type& value);
	value_type Pop();

private:
	std::mutex m_lock;
	std::condition_variable m_condition;
};

template<typename T>
void SynchronizedBlockingQueue<T>::Push( const value_type& value )
{
	std::lock_guard<std::mutex> guard(m_lock);
	base::push(value);
	m_condition.notify_all();
}

template<typename T>
typename SynchronizedBlockingQueue<T>::value_type SynchronizedBlockingQueue<T>::Pop()
{
	value_type value = value_type();

	std::unique_lock<std::mutex> uniqueLock(m_lock);
	if ( base::empty() )
	{
		m_condition.wait(uniqueLock);
	}
	value = base::front();
	base::pop();
	return value;
}