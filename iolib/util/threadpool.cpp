//
//  threadpool.cpp
//  musculus/iolib/util
//
// 线程池类
//
//  Created by umiringo on 15/7/8.
//

#include <string.h>
#include <unistd.h>
#include <algorithm>

#include "threadpool.h"
#include "itimer.h"

using namespace std;
using namespace MNET;

#define usecPerSec 1000000

namespace MNET
{
Thread::Pool::Policy	Thread::Pool::Policy::sPolicy;

Thread::Mutex			Thread::Pool::mutexTask("Thread::Pool::mutexTask");
Thread::Mutex 			Thread::Pool::mutexThread("Thread::Pool::mutexThread");
Thread::Condition		Thread::Pool::condThread;
size_t					Thread::Pool::allThreadCount = 0;
Thread::Pool::CountMap	Thread::Pool::threadCountMap;
bool					Thread::Pool::priorStrict = false;
Thread::Pool::TaskQueue Thread::Pool::taskQueue;
Thread::Pool::Policy*	Thread::Pool::pPolicy = &(Thread::Pool::Policy::sPolicy);

//Thread::Pool::Policy functions
void Thread::Pool::Policy::setState( int state )
{
	policyState = state;
	if( stateQuitAtOnce == state){
		Pool::condThread.notifyAll();
	}
}
void Thread::Pool::Policy::setMaxQueueSize( size_t taskQueueSize )
{
	maxTaskQueueSize = taskQueueSize;
}
Thread::Pool::CountMap& Thread::Pool::Policy::getPolicyThreadCountMap()
{
	return policyThreadCountMap;
}
std::string Thread::Pool::Policy::indentification()
{
	return "MusculusThreadPool";
}
size_t Thread::Pool::Policy::getThreadCount( int prior )
{
	if( stateQuitAtOnce == policyState ) return 0;
	return std::max( 1, policyThreadCountMap[prior] );
}
size_t Thread::Pool::Policy::getAllThreadCount() const
{
	if( stateQuitAtOnce == policyState ) return 0;
	return policyThreadCount;
}
bool Thread::Pool::Policy::onAddTask( Thread::RunnableTask* pTask, size_t qSize, bool bForced)
{
	if( stateNormal == policyState){
		if( pTask && ( qSize < maxTaskQueueSize || bForced) )
			return true;
	}
	delete pTask;
	return false;
}
void Thread::Pool::Policy::loadConfig()
{
	policyThreadCount = 0;
	policyThreadCountMap.clear();

	//加载配置项 TODO，先手动填一些默认值
	policyThreadCount = 10;
	policyThreadCountMap[1] = 5;
	policyThreadCountMap[2] = 5;
}

//Thread::Pool function
void Thread::Pool::sigNullHandler( int signum )
{ }
void Thread::Pool::sigusr1Handler( int signum )
{
	if( SIGUSR1 == signum ){
		//纪录log
		//退出
		pPolicy->onQuit();
		pPolicy->setState( stateQuitAtOnce );

		//PollIO?
	}
}
Thread::RunnableTask* Thread::Pool::fetchTask( int nThreadPrior )
{
	Mutex::Scoped lock( mutexTask );
	if( priorStrict ){ 		//取到对应的prior的task(严格模式)
		Thread::RunnableTask* pTask = NULL;
		if( !taskQueue.empty() ) {
			TaskQueue::iterator it = taskQueue.find( nThreadPrior );
			if( it != taskQueue.end() ){
				pTask = it->second;
				taskQueue.erase( it );
			}
		}
		return pTask;
	} else {
		Thread::RunnableTask * pTask = NULL;
		if( !taskQueue.empty() ){
			TaskQueue::iterator it = taskQueue.lower_bound( nThreadPrior ); //取个差不多的就行了
			if( it != taskQueue.end() ){ //没找到
				it = taskQueue.begin(); //直接取第一个
			}
			pTask = it->second;
			taskQueue.erase(it);
		}
		return pTask;
	}	
}
void * Thread::Pool::routine( void* pParam)
{
	int nThreadPrior = (long)pParam;

	pthread_detach( pthread_self() ); //设定成unjoin，保证资源的销毁

	sigset_t sigs;
	sigemptyset( &sigs );
	sigaddset( &sigs, SIGUSR1 );
	sigaddset( &sigs, SIGUSR2 );
	sigaddset( &sigs, SIGHUP ); //中止信号，比如kill
	pthread_sigmask( SIG_BLOCK, &sigs, NULL); //将这些信号在子线程中屏蔽，只在主线程中处理

	while(1)
	{
		try{
			RunnableTask* pTask = fetchTask( nThreadPrior );
			if( pTask ){
				try{
					pTask->run(); // go! runtine!
				} catch(...){
					delete pTask;
				}
			}

			{
				Mutex::Scoped lock(mutexThread);
				size_t tCountPolicy = pPolicy->getThreadCount( nThreadPrior ); //配置里面设定的该prior的线程数
				size_t tCountNow = threadCountMap[ nThreadPrior ]; //获取了当前的线程数量

				if( tCountNow < tCountPolicy ){
					threadCountMap[ nThreadPrior ]++;
					allThreadCount++;

					pthread_t th;
					pthread_create( &th, NULL, &Pool::routine, NULL);

				} else if( tCountNow > tCountPolicy ){
					threadCountMap[ nThreadPrior ]--;
					allThreadCount--;
					break;
				}

				if( NULL == pTask )
					condThread.wait( mutexThread ); //线程进入等待
			}
		} catch(...) { continue; } //出现异常则跳过继续
		break;
	}
	return NULL;
}

void Thread::Pool::setupDeamon()
{
	switch( fork() )
	{
	case 0:
		break;
	case -1:
		exit(-1);
	default:
		exit(0);
	}
	setsid(); //将此进程作为领头进程	
}
void Thread::Pool::setPolicy(Policy *policy)
{
	pPolicy = policy;
}
size_t Thread::Pool::taskQueueSize()
{
	Mutex::Scoped lock(mutexTask);
	return taskQueue.size();
}
size_t Thread::Pool::threadSize()
{
	return allThreadCount;
}
void Thread::Pool::addTask( Thread::RunnableTask  *task, bool bForced)
{
	if( !pPolicy || pPolicy->onAddTask( task, taskQueueSize(), bForced) ){
		{
			Mutex::Scoped lock(mutexTask);
			taskQueue.insert( std::make_pair( task->getPriority(), task) );
		}
		condThread.notifyAll(); //通知全部,惊群，但是可以防止死锁
	}	
}
void Thread::Pool::start( Policy* policy, bool wReturn)
{
	{
		Mutex::Scoped lock(mutexThread);

		pPolicy = policy;
		allThreadCount = 0;
		pPolicy->loadConfig();

		pthread_t th;
		CountMap& cMap = pPolicy->getPolicyThreadCountMap();
		for(CountMap::const_iterator it = cMap.begin(); it != cMap.end(); ++it){
			int prior = it->first;
			size_t size = it->second;
			for( size_t i = 0; i < size; ++i){
				//创建线程
				pthread_create( &th, NULL, &Pool::routine, (void *)prior );
			}
			if(size > 0){
				threadCountMap[prior] = size;
				allThreadCount += size;
			}
		}
	}

	if(wReturn) return; //主线程退出

	//否则起一个定时器
	struct sigaction act;
	memset( &act, 0, sizeof(act) );
	act.sa_handler = sigNullHandler;
	sigemptyset( &act.sa_mask );
	sigaddset( &act.sa_mask, SIGALRM );
	sigaddset( &act.sa_mask, SIGUSR1 );
	sigaddset( &act.sa_mask, SIGUSR2 );
	sigaddset( &act.sa_mask, SIGHUP );
	sigaction( SIGALRM, &act, NULL );
	sigaction( SIGUSR1, &act, NULL );
	sigaction( SIGUSR2, &act, NULL );
	sigaction( SIGHUP, &act, NULL);

	sigset_t set;
	sigemptyset( &set );
	sigaddset( &set, SIGALRM );
	sigaddset( &set, SIGUSR1 );
	sigaddset( &set, SIGUSR2 );
	pthread_sigmask( SIG_BLOCK, &set, NULL);

	int updateTime = time(NULL);
	int interval = IntervalTimer::getInterval();
	struct itimerval value;
	value.it_interval.tv_sec = interval / usecPerSec;
	value.it_interval.tv_usec = interval % usecPerSec;
	value.it_value.tv_sec = interval / usecPerSec;
	value.it_value.tv_usec = interval % usecPerSec;
	setitimer(ITIMER_REAL, &value, NULL);

	int sigVal;
	while(1){
		sigwait(&set, &sigVal);
		if( SIGALRM == sigVal ){
			IntervalTimer::update();
			time_t now = time(NULL);
			if( now > updateTime ){
				Timer::update(); //now there is timer....
				updateTime = now;
			}
			if( 0 == threadSize() ){
				usleep(5000);
				break;
			}
		} else if( SIGUSR1 == sigVal ){
			sigusr1Handler( SIGUSR1 );
		} else if( SIGUSR2 == sigVal ){
			pPolicy->onSIGUSR2();
		} else if( SIGHUP == sigVal ){
			pPolicy->onSIGHUP();
		}
	}
}

//Thread::HouseKeeper function
void Thread::HouseKeeper::addTask( Thread::RunnableTask* pTask, int waitSecs )
{
	Thread::Mutex::Scoped lock(locker);
	timerTaskQueue.insert( std::make_pair( timer.elapse() + waitSecs, pTask ) );
}
void Thread::HouseKeeper::update()
{
	int nElapse = timer.elapse();
	Thread::Mutex::Scoped lock( locker );
	TimerTaskQueue::iterator it = timerTaskQueue.begin();
	while( it != timerTaskQueue.end() ){
		if( it->first > nElapse)
			break;
		Thread::Pool::addTask( it->second, true );
		timerTaskQueue.erase( it );
		it = timerTaskQueue.begin();
	}

}
void Thread::HouseKeeper::addTimerTask( Thread::RunnableTask* pTask, int waitSecs )
{
	getInstance().addTask( pTask, waitSecs );
}

}; //MNET