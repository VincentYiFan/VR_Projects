#pragma once
#include <vector>

class Clock
{
public:
	Clock();                   // defaults to being owned by master
	Clock( Clock* parent );    // passing nullptr will create a root clock (root clock is something you'd have to advance yourself)
	~Clock();                  // be sure to correcty re-parent my children to my parent

	void Update( double deltaSeconds );    // usually do not need to call unless you create a new root clock
	void Reset();                       // set total time back to 0.0, does not reset children

	// Controls
	void Pause();
	void Resume();
	void SetScale( double scale );

	// accessors
	double GetTotalElapsedSeconds() const{ return m_totalTime; }
	double GetLastDeltaSeconds() const { return m_frameTime; }
	//double GetCurrentTimeSeconds() { return m_currentTime; }

	double GetScale() const { return m_scale; }
	bool IsPaused() const { return m_isPaused; }

	// OPTIONAL - but useful
	// void SetFrameLimits( double minFrameTime, double maxFrameTime ); // useful if you never want a simluation jumping too far (for exapmle, while stuck on a breakpoint)

public:
	// used more internally
	void SetParent( Clock* clock );
	// void AddChild( Clock* clock );  // can implement both, or just one or the other can 


public:
	// need a way to...
	// ...current time state (current time, delta, etc...)
	double	m_frameTime = 0;
	double	m_totalTime = 0;
	double	m_currentTime = 0;

	bool	m_isPaused = false;			// track pause state
	double	m_scale = 1;				// ...track scale
	Clock*	m_parent = nullptr;			// know my parent
	std::vector<Clock*> m_children;		// know my children

public: // accessor for the master clock of our engine
	static void SystemStartup();  // create/reset master clock
	static void SystemShutdown();
	static void BeginFrame();     // advance master clock (which immediately propogates to children)

	static Clock* GetMaster();
};
