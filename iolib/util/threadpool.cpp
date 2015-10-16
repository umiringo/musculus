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

#include "threadpool.h"
#include "itimer.h"

using namespace std;
using namespace MNET;

namespace MNET
{
	Thread::Pool::Policy	Thread::Pool::Policy::sPolicy;

	Thread::Mutex			Thread::Pool::sMutexTasks("Thread::Pool::sMutexTasks");
	Thread::Mutex 			Thread::Pool::sMutexThreads("Thread::Pool::sMutexThreads");
	Thread::Condition		Thread::Pool::sCondThreads;
	size_t					Thread::Pool::sThreadCount = 0;
	Thread::Pool::CountMap	Thread::Pool::sThreadCounts;
	bool					Thread::Pool::sPriorStrict = false;
	Thread::Pool::TaskQueue Thread::Pool::sTasks;
	Thread::Pool::Policy*	Thread::Pool::pPolicy = &(Thread::Pool::Policy::sPolicy);

	void Thread::Pool::Policy::loadConfig()
	{
		//加载配置项
		//设定初始化的各项值
	}

	static void sigusr1Handler( int signum )
	{
		if( SIGUSR1 == signum )
		{
			//纪录log
			//退出
			pPolicy->onQuit();
			pPolicy->setState( stateQuitAtOnce );

			//PollIO
		}

	}

	void sighandlerNull( int signum)
	{}

	void Thread::Pool::Run( Policy* policy, bool wReturn)
	{
		{
			Mutex
		}
	}

}; //MNET