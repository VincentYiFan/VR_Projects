#pragma once
#include <queue>
#include <atomic>

template <typename T>
class SynchronizedNonBlockingQueue : protected std::queue<T>
{
public:
	using value_type = T;
protected:
	using base = typename std::queue<T>;

public:
	SynchronizedNonBlockingQueue(): base() { }
	SynchronizedNonBlockingQueue( SynchronizedNonBlockingQueue const&& ) = delete;
	SynchronizedNonBlockingQueue( SynchronizedNonBlockingQueue const& ) = delete;
	~SynchronizedNonBlockingQueue() = default;

	SynchronizedNonBlockingQueue& operator=( SynchronizedNonBlockingQueue const&& ) = delete;
	SynchronizedNonBlockingQueue& operator=( SynchronizedNonBlockingQueue const& ) = delete;

	void Push( const value_type& value );
	value_type Pop();

protected:
	void Lock();
	void Unlock();

private:
	const int UNLOCKED = 0;
	const int LOCKED = 1;
	std::atomic<int> m_atomic;
};

template <typename T>
void SynchronizedNonBlockingQueue<T>::Lock()
{
	int expected = UNLOCKED;
	while( !m_atomic.compare_exchange_strong(expected, LOCKED) );
}

template <typename T>
void SynchronizedNonBlockingQueue<T>::Unlock()
{
	int expected = LOCKED;
	while( !m_atomic.compare_exchange_strong( expected, UNLOCKED ) );
}

template <typename T>
void SynchronizedNonBlockingQueue<T>::Push( const value_type& value )
{
	Lock();
	base::push(value);
	Unlock();
}

template <typename T>
typename SynchronizedNonBlockingQueue<T>::value_type SynchronizedNonBlockingQueue<T>::Pop()
{
	value_type value = value_type();

	Lock();
	if ( !base::empty() )
	{
		value = base::front();
		base::pop();
	}
	Unlock();
	return value;
}