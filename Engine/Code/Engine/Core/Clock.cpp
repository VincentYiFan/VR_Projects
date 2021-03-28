#include "Engine/Core/Clock.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

static Clock* g_masterClock = nullptr; 

Clock::Clock()
{
	if( g_masterClock != nullptr ) {
		m_parent = g_masterClock;
		g_masterClock->m_children.push_back( this );
	}
	else {
		g_masterClock = this;
	}
}

Clock::Clock( Clock* parent )
{
	if( parent == nullptr ) {
		ERROR_AND_DIE( "Failed to create clock: parent in constructor is nullptr!" );
	}

	else {
		m_parent = parent;
		m_parent->m_children.push_back( this );
	}
}

Clock::~Clock()
{
	if( m_children.size() > 0 ) {
		for( int idx = 0; idx < (int)m_children.size(); ++idx ) {
			if( m_children[idx] != nullptr ) {
				if( m_parent != nullptr ) {
					m_children[idx]->SetParent( m_parent );
				} else {
					g_masterClock = m_children[idx];
				}
			}
		}
	}

	if( m_parent != nullptr ) {		// Remove myself from my parent
		for( int idx = 0; idx < (int)m_parent->m_children.size(); ++idx ) {
			if( m_parent->m_children[idx] != nullptr && m_parent->m_children[idx] == this ) {
				m_parent->m_children[idx] = nullptr;
			}
		}
	}
}

void Clock::Update( double deltaSeconds )
{
	if( IsPaused() ) {
		deltaSeconds = 0;
	} else {
		deltaSeconds *= m_scale;
	}

	// Update my clock concept of time
	m_frameTime = deltaSeconds;
	m_totalTime += deltaSeconds;

	// After transform
	for( int i = 0; i < (int)m_children.size(); ++i ) {
		m_children[i]->Update( deltaSeconds );
	}
}

void Clock::Reset()
{
	m_totalTime = 0;
}

void Clock::Pause()
{
	m_isPaused = true;
}

void Clock::Resume()
{
	m_isPaused = false;
}

void Clock::SetScale( double scale )
{
	m_scale = scale;
}


void Clock::SetParent( Clock* clock )
{
	m_parent = clock;
}

void Clock::SystemStartup()
{
	g_masterClock = new Clock();
	g_masterClock->Reset(); 
}

void Clock::SystemShutdown()
{
	delete g_masterClock;
	g_masterClock = nullptr;
}

void Clock::BeginFrame()
{
	static double timePreviousFrame = GetCurrentTimeSeconds(); 
	double timeThisFrame = GetCurrentTimeSeconds(); 

	double dt = timeThisFrame - timePreviousFrame;
	timePreviousFrame = timeThisFrame;

	g_masterClock->Update(dt);
}

Clock* Clock::GetMaster()
{
	return g_masterClock;
}
