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

	class Routine //执行模块基类, 称作协程
	{
	protected:
		int mPriority;

	public:
		Routine( int prior = 1 ) : mPriority(prior) {}
		virtual ~Routine() {}

	public:
		virtual void Run() = 0;

		void setPriority( int prior )
		{
			mPriority = prior;
		}
		int getPriority() const
		{
			return mPriority;
		}
	};

	class Pool
	{
	public:
		enum PoolState
		{
			stateNormal = 1,
			stateQuitAtOnce = 2
		};
		typedef std::map<int, size_t> CountMap; //key是priority，value是数量
		typedef std::multimap< int, Routine*, std::less<int> > TaskQueue;  //key是priority，value是执行模块指针

		class Policy //策略类
		{
			friend class Pool;
		protected:
			size_t mMaxQueueSize;	//任务队列长度
			size_t mThreadCount;	//线程数量	
			CountMap mThreadCounts; //计数map (考虑hash)
			int mState;

			void loadConfig();
		public:
			static Policy sPolicy;
			Policy() : mMaxQueueSize(1048576), mState(stateNormal) {}
			virtual ~Policy() {}

			void setState( int state )
			{
				mState = state;
				if( stateQuitAtOnce == state){
					Pool::sCondThreads.notifyAll();
				}
			}

			void setMaxQueueSize( size_t maxQueueSize)
			{
				mMaxQueueSize = maxQueueSize;
			}

			CountMap& getThreadCountMap() //得到计数map引用
			{
				return mThreadCounts;
			}
		public:
			virtual std::string indentification()
			{
				return "MusculusThreadPool";
			}
			virtual size_t onGetThreadCount( int prior ) //返回某prior的线程数
			{
				if( stateQuitAtOnce == mState ) return 0;
				return std::max( 1, (int)mThreadCounts[prior] );
			}

			virtual size_t onGetAllThreadCount() const //返回线程数量
			{
				if( stateQuitAtOnce == mState ) return 0;
				return mThreadCount;
			}

			virtual bool onAddTask( Thread::Routine* pTask, size_t qSize, bool bForced) //增加task之后的后续处理
			{
				//log?
				if( stateNormal == mState )
				{
					if( pTask && (qSize < mMaxQueueSize || bForced ) )
						return true;
					//增加失败时应该加个log
				}
				//失败才会至此
				delete pTask;
				return false;
			}
			virtual void onQuit() {}
			virtual void onSIGUSR2() {}
			virtual void onSIGHUP() {}
		}; //class Policy

	private:
		static TaskQueue sTasks;
		static Mutex sMutexTasks;
		static Mutex sMutexThreads;
		static Condition sCondThreads;
		static size_t sThreadCount;
		static CountMap sThreadCounts;
		static bool sPriorStrict;
		static Policy* sPolicy;
		static void sigusr1_handler( int signum );

	public:
		static void setupDeamon()
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

		static void setPolicy( Policy* policy)
		{
			sPolicy = policy;
		}

		static size_t queueSize()
		{
			Mutex::Scoped lock( sMutexTasks );
			return sTasks.size();
		}

		static size_t threadSize()
		{
			return sThreadCount;
		}

		static void Run( Policy* policy = &(Policy::sPolicy), bool wReturn = false);

		static void addTask( Routine *task, bool bForced = false )
		{
			if( !sPolicy || sPolicy->onAddTask( task, queueSize(), bForced) ){
				{
					Mutex::Scoped lock(sMutexTasks);
					sTasks.insert( std::make_pair( task->getPriority(), task) );
				}
				sCondThreads.notifyAll(); //通知全部？为什么不只通知一个？
			}
		}
	private:
		virtual ~Pool() {}

		static Routine* fetchTask( int nThreadPrior )
		{
			Mutex::Scoped lock( sMutexTasks );
			if( sPriorStrict ){ 		//取到对应的prior的task(严格模式)
				Routine* pTask = NULL;
				if( !sTasks.empty() ) {
					TaskQueue::iterator it = sTasks.find( nThreadPrior );
					if( it != sTasks.end() ){
						pTask = it->second;
						sTasks.erase( it );
					}
				}
				return pTask;
			} else {
				Routine * pTask = NULL;
				if( !sTasks.empty() ){
					TaskQueue::iterator it = sTasks.lower_bound( nThreadPrior ); //取个差不多的就行了
					if( it != sTasks.end() ){ //没找到
						it = sTasks.begin(); //直接取第一个
					}
					pTask = it->second;
					sTasks.erase(it);
				}
				return pTask;
			}
		}

		static void* runThread( void* pParam )
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
					Routine* pTask = fetchTask( nThreadPrior );
					if( pTask ){
						try{
							pTask->Run(); // go! runtine!
						} catch(...){
							delete pTask;
						}
					}

					{
						Mutex::Scoped lock(sMutexThreads);
						size_t tCountWant = sPolicy->onGetThreadCount( nThreadPrior ); //获得这个prior的线程数
						size_t tCountNow = sThreadCounts[ nThreadPrior ]; //获取了当前的线程数量
						if( tCountNow < tCountWant ){
							sThreadCounts[ nThreadPrior ]++;
							sThreadCount++;
						} else if( tCountNow > tCountWant ){
							sThreadCounts[ nThreadPrior ]--;
							sThreadCount--;
							break;
						}
						if( NULL == pTask )
							sCondThreads.wait( sMutexThreads );
					}
				} catch(...) { continue; } //出现异常则跳过继续
				break;
			}
			return NULL;
		}
	}; //class Pool

	class HouseKeeper : public Timer::Observer 
	{ //一个管理类，本身是timer观察者的一个，但是本身又是一个tasks列表
		//但是用意在何处呢？TODO
	private:
		typedef std::multimap< int, Routine* > TimerTaskQueue;
		TimerTaskQueue tasks;
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

		void addTask( Routine* pTask, int waitSecs )
		{
			Thread::Mutex::Scoped lock(locker);
			tasks.insert( std::make_pair( timer.elapse() + waitSecs, pTask ) );
		}

		void update()
		{
			int nElapse = timer.elapse();
			Thread::Mutex::Scoped lock( locker );
			TimerTaskQueue::iterator it = tasks.begin();
			while( it != tasks.end() ){
				if( it->first > nElapse)
					break;
				Thread::Pool::addTask( it->second, true );
				tasks.erase( it );
				it = tasks.begin();
			}

		}

	public:
		static void addTimerTask( Routine* pTask, int waitSecs )
		{
			getInstance().addTask( pTask, waitSecs );
		}

	}; //class HouseKeeper
}; //namespace Thread
}; //namespace MNET

#endif //__THREADPOOL_H