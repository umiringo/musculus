//
//  timer.cpp
//  musculus/iolib/util
//
// 一个观察者模式类
//
//  Created by umiringo on 15/7/8.
//

#include "timer.h"

namespace MNET
{
	time_t Timer::now = time(NULL);
	timeval Timer::nowTimeVal;
	Timer::ObserverList Timer::obList;

	//functions
	void Timer::attach(Observer *ob)
	{
		obList.push_back(ob);
	}
	void Timer::detach(Observer *ob)
	{
		obList.erase( std::remove( obList.begin(), obList.end(), ob), obList.end() );
	}
	void Timer::update()
	{
		time_t tmp = time(NULL);
		if( tmp > now ){
			now = tmp;
			gettimeofday( &nowTimeVal, NULL );
			std::for_each( obList.begin(), obList.end(), std::mem_fun(&Observer::update)); //实际对每个观察者执行了update
		}
	}
	time_t Timer::getTime()
	{
		return now;
	}
	timeval Timer::getTimeVal()
	{
		return nowTimeVal;
	}

	int Timer::elapse() const 
	{
		return now - t;
	}
	int Timer::elapseMS() const
	{
		return 1000 * (nowTimeVal.tv_sec - tv.tv_sec) + (nowTimeVal.tv_usec - tv.tv_usec)/1000;
	}
	timeval Timer::elapseTimeVal() const
	{
		timeval rst;
		rst.tv_sec = nowTimeVal.tv_sec - tv.tv_sec - (nowTimeVal.tv_usec >= tv.tv_usec ? 0 : 1);  //根据微秒项修正秒差
		rst.tv_usec = nowTimeVal.tv_usec - tv.tv_usec + (nowTimeVal.tv_usec >= tv.tv_usec ? 0 : 1000000);
		return rst;
	}
	void Timer::reset()
	{
		t = now;
		tv.tv_sec = nowTimeVal.tv_sec;
		tv.tv_usec = nowTimeVal.tv_usec;
	}

}; //namespace MNET