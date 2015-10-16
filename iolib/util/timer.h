//
//  timer.h
//  musculus/iolib/util
//
// 一个观察者模式定时器类
//
//  Created by umiringo on 15/7/8.
//

#ifndef __TIMER_H__
#define __TIMER_H__


#include <vector>
#include <functional>
#include <algorithm>
#include <timer.h>
#include <sys/time.h>

namespace MNET
{

class Timer
{
//一个观察者模式类
public:
	class Observer
	{
	//观察者基类
	public:
		virtual ~Observer() {}
		virtual void update() = 0; //
	};

private:
	typedef std::vector<Observer *> observerList; //观察者列表

	static observerList& getInstance()
	{
		static observerList instance;
		return instance;
	}

private:
	static time_t now;	//当前时间,在update时更新,静态，所以作为计时器的now的计算来对加入时间进行赋值
	static timeval now_tv; //当前时间的另一个模式

	time_t t;	// 观察者加入的时间
	timeval tv; // 观察者加入的时间的另一个模式

public:
	//构造函数在此！
	Timer() : t(now)
	{
		if(!now){
			//第一次启动，初始化时间
			now = t = time(NULL);
		}
		if( !timerisset( &now_tv )) { //检查tv是否被设定过
			gettimeofday( &now_tv, NULL );
			tv.tv_sec = now_tv.tv_sec;
			tv.tv_usec = now_tv.tv_usec;
		}
	}

	~Timer() {}

public:
	static void attach(Observer *ob)
	{
		getInstance().push_back(ob);
	}
	static void detach(Observer *ob)
	{
		getInstance().erase( std::remove(getInstance().begin(), getInstance().end(), ob), getInstance().end() );
	}
	static void update()
	{
		time_t tmp = time(NULL);
		if( tmp > now ){ //更新now和now_tv
			now = tmp;
			gettimeofday( &now_tv, NULL );
			std::for_each( getInstance().begin(), getInstance().end(), std::mem_fun(&Observer::update)); //实际对每个观察者执行了update
		}
	}

	static time_t getTime()
	{
		return now;
	}
	static timeval getTimeTV()
	{
		return now_tv;
	}
public:
	int elapse() const
	{
		return now - t;
	}
	int elapseMS() const
	{
		return 1000 * (now_tv.tv_sec - tv.tv_sec) + (now_tv.tv_usec - tv.tv_usec)/1000;
	}
	timeval elapseTV() const
	{
		timeval rst;
		rst.tv_sec = now_tv.tv_sec - tv.tv_sec - (now_tv.tv_usec >= tv.tv_usec ? 0 : 1);  //根据微秒项修正秒差
		rst.tv_usec = now_tv.tv_usec - tv.tv_usec + (now_tv.tv_usec >= tv.tv_usec ? 0 : 1000000);
		return rst;
	}
	void reset()
	{
		t = now;
		tv.tv_sec = now_tv.tv_sec;
		tv.tv_usec = now_tv.tv_usec;
	}
}; //class Timer


}; //namespace MNET

#endif //__TIMER_H__