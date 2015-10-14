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
	timeval Timer::now_tv;
};