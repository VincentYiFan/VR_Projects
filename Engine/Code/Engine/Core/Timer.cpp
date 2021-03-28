#include "Timer.hpp"

Timer::Timer()
{
}

Timer::~Timer()
{
}

void Timer::SetSeconds( Clock* clock, double timeToWait )
{
	m_clock = clock;
	m_startSeconds = clock->GetTotalElapsedSeconds();
	m_durationSeconds = timeToWait;
}

void Timer::SetSeconds( double timeToWait )
{
	m_clock = Clock::GetMaster();
	if( m_clock != nullptr ) {
		m_startSeconds = m_clock->GetTotalElapsedSeconds();
		m_durationSeconds = timeToWait;
	}
}

void Timer::Reset()
{
	m_startSeconds = 0;
	m_durationSeconds = -1;
	m_elapsedTime = 0;
}

void Timer::Stop()
{
	m_durationSeconds = -1;
}

double Timer::GetElapsedSeconds() const
{
	return m_elapsedTime;
}

double Timer::GetSecondsRemaining() const
{
	return m_durationSeconds - m_elapsedTime;
}

bool Timer::HasElapsed() const
{
	if( !m_clock ) {
		return false;
	}

	double currentTime = m_clock->GetTotalElapsedSeconds();
	return ( currentTime > ( m_startSeconds + m_durationSeconds ) );
}

bool Timer::CheckAndDecrement()
{
	if( HasElapsed() ) {
		// remove an interval of time
		m_startSeconds += m_durationSeconds;
		return true;
	}
	else {
		return false;
	}
}

int Timer::CheckAndDecrementAll()
{
	while( CheckAndDecrement() ) {
		m_intervals++; 
	}

	return m_intervals;
}

bool Timer::CheckAndReset()
{
	if( HasElapsed() ) {
		//Reset();
		m_startSeconds = m_durationSeconds;
		return true;
	}
	else {
		return false;
	}
}

bool Timer::IsRunning() const
{
	return false;
}
