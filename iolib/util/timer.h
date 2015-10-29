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
	typedef std::vector<Observer *> ObserverList; 

private:
	static ObserverList obList; //观察者列表
	static time_t now;	//当前时间,在update时更新,静态，所以作为计时器的now的计算来对加入时间进行赋值
	static timeval nowTimeVal; //当前时间的另一个模式

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
		if( !timerisset( &nowTimeVal )) { //检查tv是否被设定过
			gettimeofday( &nowTimeVal, NULL );
			tv.tv_sec = nowTimeVal.tv_sec;
			tv.tv_usec = nowTimeVal.tv_usec;
		}
	}

	~Timer() {}

public:
	static void attach(Observer *ob);
	static void detach(Observer *ob);
	static void update();
	static time_t getTime();
	static timeval getTimeVal();

	int elapse() const;
	int elapseMS() const;
	timeval elapseTimeVal() const;
	void reset();

}; //class Timer


}; //namespace MNET

#endif //__TIMER_H__