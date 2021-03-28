#pragma once
#include <vector>
#include <deque>
#include <mutex>
#include <atomic>
#include <thread>

//-------------------------------------------------------------------------------------------------------------
class Job
{
public:
	Job();
	virtual ~Job();
	virtual void Execute() = 0; // executed by WORKER threads
	virtual void OnCompleteCallback() = 0; // called by CLIENT (e.g. main thread); delivers results of job

//protected:
	int				m_jobID = 0;
	//unsigned int	m_jobType = 0;
	//unsigned int	m_jobFlags = 0;
};

//-------------------------------------------------------------------------------------------------------------
class ExampleJob : public Job
{
public:
	ExampleJob( int maxNumber );
	~ExampleJob();

	virtual void Execute() override; // execute by WORKER threads
	virtual void OnCompleteCallback() override; // Called by CLIENT (e.g. main thread). Delivers results of job

public:
	int					m_maxNumber = 0;
	std::vector<int>	m_odds;
};

//-------------------------------------------------------------------------------------------------------------
class JobSystemWorkerThread
{
public:
	JobSystemWorkerThread( int threadID );
	~JobSystemWorkerThread();

	void WorkThreadMain( int threadID );

//private:
	std::thread*	m_threadObject = nullptr;
	int				m_threadID = -1;
};

//-------------------------------------------------------------------------------------------------------------
class JobSystem
{
public:
	JobSystem();
	~JobSystem();	// signal threads to quit, and join them (block on them finishing)

	void	StartUp();
	void	ShutDown();

	void	StartWorkerThreads();
	void	StopWorkerThreads();
	void	PostJob( Job* job );
	void	OnJobCompleted( Job* job );
	void	ClaimAndDeleteAllCompletedJobs();	// called by CLIENT thread; calls the Callback method on every completed job
	
	Job*	ClaimNextCompletedJob();
	Job*	GetBestAvailableJob();
	
	bool	IsQuitting() const { return m_isQuitting; }

//protected:
public:
	std::deque< Job* >	m_jobsQueued;
	std::deque< Job* >	m_jobsCompleted;
	std::mutex			m_jobsQueuedMutex;
	std::mutex			m_jobsCompletedMutex;
	std::atomic<bool>	m_isQuitting = false;
	std::atomic<int>	m_clientJobClaimCount = 0;

	std::vector< JobSystemWorkerThread* > m_workerThreads;
};