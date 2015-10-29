//
//  itimer.h
//  musculus/iolib/util
//
// 一个观察者模式定时器类
//
//  Created by umiringo on 15/7/8.
//
#ifndef __ITIMER_H__
#define __ITIMER_H__

#include <signal.h>
#include <sys/time.h>
#include <map>

#include "threadpool.h"

namespace MNET
{
#define defaultInterval 100000 //100ms

class IntervalTimer : public Thread::RunnableTask
{
public:
	class Observer
	{
		friend class IntervalTimer;
		size_t interval;
	public:
		virtual ~Observer() {}
		virtual bool update() = 0;
		virtual size_t getInterval() { return interval; }
	};//class Observer

private:
	typedef std::multimap<int64_t, Observer*> ObserverMap; //key是时间间隔
	class ScheduleTask : public Observer
	{
		Thread::RunnableTask *task;
	public:
		ScheduleTask( Thread::RunnableTask *rt) : task(rt) {}
		bool update()
		{
			Thread::Pool::addTask(task, true);
			delete this;
			return false;
		}
	};

	static bool quitAtOnce;//?
	static bool beTriggered;//?
	static size_t interval; //每tick的微秒数
	static Thread::Mutex mutexItimer;
	static ObserverMap observerMap;
	static timeval now;
	static timeval base;
	static int64_t tickNow;
	static sigset_t sigMask;
	static IntervalTimer instance;

	virtual void run();
	static void attachObserver(Observer *o);
	static void attachObserver(Observer *o, int64_t targetTick);

public:
	IntervalTimer(){}
	static void update();
	static int getInterval();
	static void attach(Observer *o, size_t delay);
	static void attach(Observer *o);
	static void addTimer(Observer *o, int sec);
	static void schedule(RunnableTask *rt, size_t delay);
	static bool startTimer(size_t usec = 0);
	static void stopTimer();
	static void getTime(timeval* val);
	static int64_t getTick();
	static void handler(int signum);
	static void suspend();
	static void resume();
	static void updateTimer();

	/*
	int64_t uElapse() const;
	int elapse() const;
	void reset() const;
	*/


};// class IntervalTimer

};// namespace MNET


#endif //__ITIMER_H__
