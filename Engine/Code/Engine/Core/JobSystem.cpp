#include "Engine/Core/JobSystem.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Game/GameCommon.hpp"	// #ToDo: Don't include game related header in engine code

extern JobSystem*	g_theJobSystem;

//-------------------------------------------------------------------------------------------------------------
JobSystemWorkerThread::JobSystemWorkerThread( int threadID )
	:m_threadID( threadID )
{
	m_threadObject = new std::thread( &JobSystemWorkerThread::WorkThreadMain, this, threadID );
}

//-------------------------------------------------------------------------------------------------------------
JobSystemWorkerThread::~JobSystemWorkerThread()
{
	delete m_threadObject;
	m_threadObject = nullptr;
}

//-------------------------------------------------------------------------------------------------------------
void JobSystemWorkerThread::WorkThreadMain( int threadID )
{
	//g_theConsole->Printf( "Started worker thread %i...", threadID );
	printf( "Started worker thread #%i...\n", threadID );

	while( !g_theJobSystem->IsQuitting() )
	{
		// Try to claim a job
		g_theJobSystem->m_jobsQueuedMutex.lock();
		if( !g_theJobSystem->m_jobsQueued.empty() )
		{
			Job* jobAtFrontOfQueue = g_theJobSystem->m_jobsQueued.front();
			g_theJobSystem->m_jobsQueued.pop_front();
			g_theJobSystem->m_jobsQueuedMutex.unlock();
			printf( "Thread #%i is working on job#%i\n", threadID, jobAtFrontOfQueue->m_jobID );
			jobAtFrontOfQueue->Execute();
			g_theJobSystem->OnJobCompleted( jobAtFrontOfQueue );
		}
		else
		{
			g_theJobSystem->m_jobsQueuedMutex.unlock();
			std::this_thread::sleep_for( std::chrono::microseconds(10) );
		}
	}

	//g_theConsole->Printf( "Exiting worker thread %i...", threadID );
	printf( "Exiting worker thread #%i...\n", threadID );
}

//-------------------------------------------------------------------------------------------------------------
JobSystem::JobSystem()
{
}

//-------------------------------------------------------------------------------------------------------------
JobSystem::~JobSystem()
{
	m_isQuitting = true;
}

//-------------------------------------------------------------------------------------------------------------
void JobSystem::StartUp()
{
	StartWorkerThreads();

	//Sleep( 5000 );

	StopWorkerThreads();
}

void JobSystem::ShutDown()
{
}

//-------------------------------------------------------------------------------------------------------------
void JobSystem::StartWorkerThreads()
{
	m_workerThreads.reserve( NUM_WORKER_THREADS );
	for( int i = 0; i < NUM_WORKER_THREADS; ++i )
	{
		JobSystemWorkerThread* newThread = new JobSystemWorkerThread( i + 1 );
		m_workerThreads.push_back( newThread );
	}
}

//-------------------------------------------------------------------------------------------------------------
void JobSystem::StopWorkerThreads()
{
	m_isQuitting = true;
	for( int i = 0; i < NUM_WORKER_THREADS; ++i )
	{
		m_workerThreads[i]->m_threadObject->join();
	}

	for( int i = 0; i < NUM_WORKER_THREADS; ++i )
	{
		delete m_workerThreads[i];
		m_workerThreads[i] = nullptr;
	}
}

//-------------------------------------------------------------------------------------------------------------
void JobSystem::PostJob( Job* job )
{
	m_jobsQueuedMutex.lock();
	m_jobsQueued.push_back( job );
	m_jobsQueuedMutex.unlock();
}

void JobSystem::OnJobCompleted( Job* job )
{
	m_jobsCompletedMutex.lock();
	m_jobsCompleted.push_back( job );
	m_jobsCompletedMutex.unlock();
}

//-------------------------------------------------------------------------------------------------------------
void JobSystem::ClaimAndDeleteAllCompletedJobs()
{
	std::deque<Job*> claimedJobs;

	m_jobsCompletedMutex.lock();
	m_jobsCompleted.swap( claimedJobs );
	m_jobsCompletedMutex.unlock();

	//m_clientJobClaimCount = claimedJobs.size(); // New Added

	for( auto iter = claimedJobs.begin(); iter != claimedJobs.end(); ++iter )
	{
		m_clientJobClaimCount++;

		Job* job = *iter;
		job->OnCompleteCallback();
		delete job;
	}
}

//-------------------------------------------------------------------------------------------------------------
Job* JobSystem::ClaimNextCompletedJob()
{
	Job* job = nullptr;
	m_jobsCompletedMutex.lock();
	if( !m_jobsCompleted.empty() )
	{
		job = m_jobsCompleted.front();
		m_jobsCompleted.pop_front();
	}
	m_jobsCompletedMutex.unlock();
	if( job )
	{
		m_clientJobClaimCount++;

		job->OnCompleteCallback();
	}

	return job;
}

Job* JobSystem::GetBestAvailableJob()
{
	// #ToDo: Implement
	return nullptr;
}

//-------------------------------------------------------------------------------------------------------------
ExampleJob::ExampleJob( int maxNumber )
	: Job(),
	m_maxNumber( maxNumber )
{
	
}

//-------------------------------------------------------------------------------------------------------------
ExampleJob::~ExampleJob()
{
}

//-------------------------------------------------------------------------------------------------------------
void ExampleJob::Execute()
{
	m_odds.reserve( m_maxNumber );
	for( int i = 0; i <= m_maxNumber; ++i )
	{
		if( i % 2 != 0 )
		{
			m_odds.push_back( i );
		}
	}
}

//-------------------------------------------------------------------------------------------------------------
void ExampleJob::OnCompleteCallback()
{
	printf( " Job #%i completed\n", m_jobID );
}

//-------------------------------------------------------------------------------------------------------------
Job::Job()
{
	static int s_nextJobID = 1;
	m_jobID = s_nextJobID++;
}

//-------------------------------------------------------------------------------------------------------------
Job::~Job()
{
}
