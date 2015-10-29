//
//  itimer.cpp
//  musculus/iolib/util
//
// 一个观察者模式类
//
//  Created by umiringo on 15/7/8.
//

#include "itimer.h"

namespace MNET
{

#define usecPerSec 1000000

bool IntervalTimer::quitAtOnce = true;
bool IntervalTimer::beTriggered = true;
size_t IntervalTimer::interval = defaultInterval;
Thread::Mutex IntervalTimer::mutexItimer("IntervalTimer::mutexItimer", true);
IntervalTimer::ObserverMap IntervalTimer::observerMap;
timeval IntervalTimer::now;
timeval IntervalTimer::base;
int64_t IntervalTimer::tickNow = 0;
sigset_t IntervalTimer::sigMask;
IntervalTimer IntervalTimer::instance;

// functions 

void IntervalTimer::run()
{
	sigset_t sigs;
	sigfillset( &sigs );
	pthread_sigmask( SIG_BLOCK, &sigs, NULL );

	struct itimerval value;
	value.it_interval.tv_sec = interval / usecPerSec;
	value.it_interval.tv_usec = interval % usecPerSec;
	value.it_value.tv_sec = interval / usecPerSec;
	value.it_value.tv_usec = interval % usecPerSec;
	setitimer( ITIMER_REAL, &value, NULL );

	sigfillset( &sigs );
	sigdelset( &sigs, SIGALRM);

	while( !quitAtOnce ){
		sigsuspend( &sigs );
		update();
	}
	setitimer( ITIMER_REAL, NULL, NULL ); //清除定时器
}
void IntervalTimer::attachObserver(Observer *o)
{
	if( !quitAtOnce ){
		observerMap.insert(std::make_pair(tickNow + o->interval, o));
	}
}
void IntervalTimer::attachObserver(Observer *o, int64_t targetTick)
{
	if( !quitAtOnce ){
		observerMap.insert(std::make_pair(targetTick, o));
	}
}

void IntervalTimer::update()
{
	ObserverMap::iterator obIt;
	bool repeated;
	{
		Thread::Mutex::Scoped lock(mutexItimer);
		gettimeofday( &now, NULL );
		tickNow = ((int64_t)(now.tv_sec - base.tv_sec) * 1000000LL + now.tv_usec - base.tv_usec) / interval;
		obIt = observerMap.begin();
	}

	while( obIt != observerMap.end() ){
		if( obIt->first > tickNow ) break;
		repeated = obIt->second->update();
		{
			Thread::Mutex::Scoped lock(mutexItimer);
			if( repeated ){
				int64_t nextTick = obIt->first + obIt->second->interval;
				attachObserver( obIt->second, nextTick);
			}
		}
		observerMap.erase(obIt);
		obIt = observerMap.begin();
	}
}
int IntervalTimer::getInterval()
{
	return interval;
}
void IntervalTimer::attach(Observer *o, size_t delay) 
{
	delay = delay > 0 ? delay : 1;
	o->interval = delay;
	Thread::Mutex::Scoped lock(mutexItimer);
	attachObserver(o);
}
void IntervalTimer::attach(Observer *o)
{
	o->interval = usecPerSec / interval;
	Thread::Mutex::Scoped lock(mutexItimer);
	attachObserver(o);
}
void IntervalTimer::addTimer(Observer *o, int sec) //sec秒后触发的定时器
{
	o->interval = sec * usecPerSec / interval;
	Thread::Mutex::Scoped lock(mutexItimer);
	attachObserver(o);
}
void IntervalTimer::schedule(RunnableTask *rt, size_t delay)
{
	attach( new ScheduleTask(rt), delay );
}
bool IntervalTimer::startTimer(size_t usec)
{
	usec = usec == 0 ? defaultInterval : usec;
	if( !quitAtOnce ) return false;
	quitAtOnce = false;
	interval = usec;
	tickNow = 0;
	gettimeofday( &base, NULL );
	return true;
}
void IntervalTimer::stopTimer()
{
	quitAtOnce = true;
}
void IntervalTimer::getTime( timeval* val )
{
	Thread::Mutex::Scoped lock(mutexItimer);
	if( val ){
		*val = now;
	}
}
int64_t IntervalTimer::getTick()
{
	return tickNow;
}
void IntervalTimer::handler(int signum)
{
	beTriggered = true;
}
void IntervalTimer::suspend()
{
	sigprocmask(SIG_BLOCK, &sigMask, NULL);
	if(beTriggered){
		update();
		beTriggered = false;
	}
}
void IntervalTimer::resume()
{
	sigprocmask(SIG_UNBLOCK, &sigMask, NULL);
}
void IntervalTimer::updateTimer()
{
	if(beTriggered){
		update();
		beTriggered = false;
	}
}

};//namespace MNET