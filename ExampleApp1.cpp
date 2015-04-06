// ExampleApp1.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <vector>
#include <memory>
#include <thread>
#include <chrono>

#include "LightLogger.h"
#include "Stopper.h"
#include "ConcurrentQueue.h"


/*===============================================================================================
Task:
Organize processing of the requests from the queue. All requests returned from GetRequest function should be placed into that queue.
Run several processing threads that will process requests from that queue using function ProcessRequest.
Work for 30 seconds.
Stop all threads correctly. Delete nonprocessed requests.
Stopper type should be defined as mechanism for early stopping the action performed (processing or producing requests).

GetRequest function may take some time to produce request.
ProcessRequest function may take some time to process request.

May use C++ (98/11) and STL.
====================================================================================*/

//Given:

class Request
{

};

class Stopper;

// returns NULL if object stopSignal asks to stop threads,
// otherwise pointer to allocated memory which should be freed after processing
Request* GetRequest(const Stopper& stopSignal) throw();

// process request, no deallocation of memory,
// may return immediately if object stopSignal asks to stop threads
void ProcessRequest(Request* request, const Stopper& stopSignal) throw();

// =================================================================================

// ===== Base example class CSomeQueue
class CSomeQueue
{
	typedef std::unique_ptr<Request> RequestItem;
	typedef ConcurrentQueue< RequestItem > RequestQueue;

public:
	CSomeQueue();
	~CSomeQueue();

	void start(size_t n_thread_count = 2);
	void stop();
	void clear();

private:
	//control
	void startFillQueue();
	void startProcessQueue(size_t n_thread_count);

	Request* getRequest() throw();
	void processRequest(Request* request) throw();

	//thread func
	void fSQFillProc();
	void fSQProcessProc();

private:
	Stopper m_stopper; //flag to stop all threads

	//threads:
	std::thread m_fillQueue; //producer of requests, filler of the queue
	std::vector<std::thread> m_processQueue; //consumers of requests
	//request queue
	RequestQueue m_requests;
};

CSomeQueue::CSomeQueue() : m_requests(m_stopper)
{
	logz("\nCSomeQueue()");
}

CSomeQueue::~CSomeQueue()
{
	logz("\n~CSomeQueue() entered");
	stop();
}

inline Request* CSomeQueue::getRequest()
{
	return GetRequest(m_stopper);
}

inline void CSomeQueue::processRequest(Request* request)
{
	ProcessRequest(request, m_stopper);
}

void CSomeQueue::fSQFillProc() {
	std::srand((unsigned int)time(0));
	while (false == m_stopper) {
		//obtain next request
		if (auto request = RequestItem(getRequest()))
		{
			logz("\nCSomeQueue::fSQFillProc() Add Request ", request.get());
			m_requests.push(std::move(request));
		}
		else
			logz("\nCSomeQueue::fSQFillProc() NULL Request");
	}
}

void CSomeQueue::fSQProcessProc() {
	std::srand((unsigned int)time(0));
	while (false == m_stopper) {
		RequestItem request;
		if (m_requests.pop(request))
		{
			logz("\nCSomeQueue::fSQProcessProc() Get Request ", request.get(), " in thread ", std::this_thread::get_id());
			processRequest(request.get());
			logz("\nCSomeQueue::fSQProcessProc() Process Request ", request.get(), " in thread ", std::this_thread::get_id());
		}
		else
		{
			logz("\nCSomeQueue::fSQProcessProc() no request in thread ", std::this_thread::get_id());
		}
	}
}

void CSomeQueue::start(size_t n_thread_count)
{
	logz("\nCSomeQueue::start() ", n_thread_count, " processing threads");
	m_stopper.stop(false); // reset stopper flag
	startFillQueue();
	startProcessQueue(n_thread_count);
}

inline void CSomeQueue::startFillQueue()
{
	//create producer threads
	m_fillQueue = std::thread(&CSomeQueue::fSQFillProc, this);
}

inline void CSomeQueue::startProcessQueue(size_t n_thread_count)
{
	//create consumer threads
	while (n_thread_count--)
	{
		m_processQueue.push_back(std::thread(&CSomeQueue::fSQProcessProc, this));
	}
}

void CSomeQueue::stop()
{
	logz("\nCSomeQueue::stop() entered");
	m_requests.stop();
	//no need in 'm_stopper.stop()' because it is called inside concurrentQueue

	//wait for producer thread
	if (m_fillQueue.joinable())
	{
		m_fillQueue.join();
		logz("\nCSomeQueue::stop() Fill Queue stopped");
	}

	//wait for consumer threads
	for (auto&& procQueue : m_processQueue)
	{
		if (procQueue.joinable())
			procQueue.join();
	}
	m_processQueue.clear();
	logz("\nCSomeQueue::stop() all Process Queues stopped");
	logz("\nCSomeQueue::stop() ", m_requests.size(), " unprocessed requests\n\n\n");
}

void CSomeQueue::clear()
{
	logz("\nCSomeQueue::clear() entered");
	m_requests.clear();
	logz("\nCSomeQueue::clear() passed");
}

//implement some
Request* GetRequest(const Stopper& stopSignal)
{
	std::this_thread::sleep_for(std::chrono::milliseconds(std::rand() % 200));
	return stopSignal ? NULL : new Request;
}

void ProcessRequest(Request* request, const Stopper& stopSignal)
{
	if (stopSignal)
		return;//stop
	std::this_thread::sleep_for(std::chrono::milliseconds(std::rand() % 2000));
}


int _tmain(int argc, _TCHAR* argv[])
{

	CSomeQueue queue;
	//one producer & 7 consumers
	queue.start(7);
	std::this_thread::sleep_for(std::chrono::milliseconds(30000));
	logz("\nmain: stop queue");
	queue.stop();
	//no deletion of unprocessed requests in the queue
		
	std::this_thread::sleep_for(std::chrono::milliseconds(3000));
	//one producer & 2 consumers
	queue.start(2);
	std::this_thread::sleep_for(std::chrono::milliseconds(30000));
	logz("\nmain: stop queue");
	queue.stop();

	queue.clear();
	std::this_thread::sleep_for(std::chrono::milliseconds(3000));
	//one producer & 5 consumers
	queue.start(5);
	std::this_thread::sleep_for(std::chrono::milliseconds(10000));

	return 0;
	
}

