//
//  threadpool.h
//  musculus/iolib/util
//
// 线程池类
//
//  Created by umiringo on 15/7/8.
//

#ifndef __THREADPOOL_H
#define __THREADPOOL_H

#include <signal.h>
#include <map>
#include <vector>
#include <string>
#include <queue>
#include <list>
#include <pthread.h>
#include <errno.h>
#include <unistd.h>

#include "mutex.h"
#include "timer.h"

namespace MNET
{

namespace Thread
{

	class RunnableTask //执行模块基类, 称作任务类
	{
	protected:
		int priority; //优先级

	public:
        RunnableTask( int prior = 1 ) : priority(prior) {}
		virtual ~RunnableTask() {}

	public:
		virtual void run() = 0;

		inline void setPriority( int prior )
		{
			priority = prior;
		}
		inline int getPriority() const
		{
			return priority;
		}
	};

	//线程池类参上！
	class Pool
	{
	public:
		enum PoolState
		{
			stateNormal = 1,
			stateQuitAtOnce = 2
		};
		typedef std::map<int, size_t> CountMap; //key是priority，value是数量
		typedef std::multimap< int, RunnableTask*, std::less<int> > TaskQueue;  //key是priority，value是执行模块指针

		class Policy //策略类,用于维护配置数据以及task的策略
		{
			friend class Pool;
		protected:
			size_t maxTaskQueueSize;	//任务队列长度上限
			size_t  policyThreadCount;	//总线程数量
			CountMap policyThreadCountMap; //计数map
			int policyState;             //状态

			void loadConfig();      //从配置文件加载以上数据
		public:
			static Policy sPolicy;

			Policy() : maxTaskQueueSize(1048576), policyState(stateNormal) {}
			virtual ~Policy() {}

			inline void setState( int state );
			inline void setMaxQueueSize( size_t taskQueueSize);
			inline CountMap& getPolicyThreadCountMap(); //得到计数map引用

			virtual std::string indentification();
			virtual size_t getThreadCount( int prior ); //返回某prior的线程数
			virtual size_t getAllThreadCount() const; //返回线程数量
			virtual bool onAddTask( Thread::RunnableTask* pTask, size_t qSize, bool bForced); //增加task之后的后续处理
			virtual void onQuit() {}
			virtual void onSIGUSR2() {}
			virtual void onSIGHUP() {}
		}; //class Policy

	private:
		virtual ~Pool() {}

		static Mutex mutexTask; //添加task的时候的锁
		static Mutex mutexThread; //对线程池操作的锁
		static Condition condThread; //线程调度的条件变量
		static size_t allThreadCount; //线程数量
		static CountMap threadCountMap;
		static bool priorStrict;
		static Policy* pPolicy;     //策略类
		
        static TaskQueue taskQueue;    //任务列表

		static void sigNullHandler( int signum );
        static void sigusr1Handler( int signum );
        static RunnableTask* fetchTask( int nThreadPrior );
        static void* routine( void* pParam );

	public:
		static void setupDeamon();
		static void setPolicy( Policy* policy);
		static size_t taskQueueSize();
		static size_t threadSize();
		static void start( Policy* policy = &(Policy::sPolicy), bool wReturn = false);
		static void addTask( RunnableTask *task, bool bForced = false );
			
	}; //class Pool

	class HouseKeeper : public Timer::Observer 
	{ 
		//一个单例管理类，本身是timer观察者的一个，但是本身又是一个tasks列表
		//可以执行一些延迟执行的task, 一次退出
	private:
		typedef std::multimap< int, RunnableTask* > TimerTaskQueue;
		TimerTaskQueue timerTaskQueue;
		Thread::Mutex locker;
		Timer timer; //观察者类在此

		HouseKeeper() : locker("Thread::HouseKeeper::locker")
		{
			Timer::attach( this );
		}
		static HouseKeeper& getInstance()
		{
			static HouseKeeper instance;
			return instance;
		}

		void addTask( RunnableTask* pTask, int waitSecs );
		void update();

	public:
		static void addTimerTask( RunnableTask* pTask, int waitSecs );

	}; //class HouseKeeper
}; //namespace Thread
}; //namespace MNET

#endif //__THREADPOOL_H
