//
//  mutext.h
//  musculus/iolib/util
//
//  互斥锁类
//
//  Created by umiringo on 15/7/8.
//

#ifndef __MUTEX_H__
#define __MUTEX_H__

#include <sys/time.h>
#include <pthread.h>
#include <errno.h>

namespace MNET
{

namespace Thread
{
	class Condition;

	class Mutex //互斥量类
	{
		friend class Condition;
	private:
		pthread_mutex_t mutex;
		Mutex(const Mutex& rhs) {}

	public:
		~Mutex()
		{
			while( pthread_mutex_destroy(&mutex) == EBUSY ){
				lock();
				unLock();
			}
		}
		explicit Mutex(bool recursive = false)
		{
			pthread_mutexattr_t attr;
			pthread_mutexattr_init(&attr);
			pthread_mutexattr_settype(&attr, recursive ? PTHREAD_MUTEX_RECURSIVE : PTHREAD_MUTEX_NORMAL);
			pthread_mutex_init(&mutex, &attr);
			pthread_mutexattr_destroy(&attr);
		}

		explicit Mutex(const char* id, bool recursive = false)
		{
			pthread_mutexattr_t attr;
			pthread_mutexattr_init(&attr);
			pthread_mutexattr_settype(&attr, recursive ? PTHREAD_MUTEX_RECURSIVE : PTHREAD_MUTEX_NORMAL);
			pthread_mutex_init(&mutex, &attr);
			pthread_mutexattr_destroy(&attr);
		}

		void lock()
		{
			pthread_mutex_lock( &mutex );
		}

		void unLock()
		{
			pthread_mutex_unlock( &mutex );
		}

		bool tryLock()
		{
			return pthread_mutex_trylock( &mutex ) == 0;
		}

		class Scoped
		{
			Mutex *mx;
		public:
			~Scoped() 
			{ 
				mx->unLock();
			}

			explicit Scoped( Mutex& m) : mx(&m)
			{
				mx->lock();
			}
		};

	}; //class Mutex

	class SpinLock //自旋锁
	{
	private:
		pthread_spinlock_t spin;
		SpinLock(const SpinLock& rhs) {}
	public:
		~SpinLock()
		{
			pthread_spin_destroy( &spin );
		}

		explicit SpinLock(const char* id)
		{
			pthread_spin_init( &spin, PTHREAD_PROCESS_PRIVATE );
		}
		void lock()
		{
			pthread_spin_lock( &spin );
		}
		void unLock()
		{
			pthread_spin_unlock( &spin );
		}

		class Scoped
		{
			SpinLock *sl;
		public:
			~Scoped()
			{
				sl->unLock();
			}
			explicit Scoped( SpinLock& m ) : sl(&m)
			{
				sl->lock();
			}
		};
	};

	class RWLock //读写锁
	{
	private:
		pthread_rwlock_t locker;
		RWLock(const RWLock& rhs) {}
	public:
		~RWLock()
		{
			while( pthread_rwlock_destroy( &locker ) == EBUSY )
			{
				wrLock();
				unLock();
			}
		}

		explicit RWLock()
		{
			pthread_rwlock_init( &locker, NULL);
		}

		explicit RWLock(const char* id)
		{
			pthread_rwlock_init( &locker, NULL);
		}

		void wrLock()
		{
			pthread_rwlock_wrlock( &locker );
		}

		void rdLock()
		{
			pthread_rwlock_rdlock( &locker );
		}

		void unLock()
		{
			pthread_rwlock_unlock( &locker );
		}
	};

	class WRScoped
	{
	private:
		RWLock *rw;
	public:
		~WRScoped()
		{
			rw->unLock();
		}
		explicit WRScoped( RWLock &l ) : rw(&l)
		{
			rw->wrLock();
		}
	};

	class RDScoped
	{
	private:
		RWLock *rw;
	public:
		~RDScoped()
		{
			rw->unLock();
		}
		explicit RDScoped( RWLock &l ) : rw(&l)
		{
			rw->rdLock();
		}
	};

	class Condition
	{
	private:
		pthread_cond_t cond;
		Condition(const Condition& rhs) {}
	public:
		~Condition()
		{
			while( pthread_cond_destroy( &cond ) == EBUSY )
			{
				notifyAll();
			}
		}

		explicit Condition()
		{
			pthread_cond_init( &cond, NULL );
		}

		int wait(  Mutex& mutex )
		{
			return pthread_cond_wait(  &cond, &mutex.mutex );
		}

		int timedWait( Mutex& mutex, int nseconds)
		{
			if( nseconds >= 0 )
			{
				timeval now;
				timespec timeout;
				struct timezone tz;
				gettimeofday( &now, &tz );
				timeout.tv_sec = now.tv_sec + nseconds;
				timeout.tv_nsec = now.tv_usec * 1000;
				return pthread_cond_timedwait( &cond, &mutex.mutex, &timeout );	
			}
			return pthread_cond_wait( &cond, &mutex.mutex );	
		}
		
		int notifyOne()
		{
			return pthread_cond_signal( &cond );
		}

		int notifyAll()
		{
			return pthread_cond_broadcast( &cond );
		}
	}; //class Condition
}; //namespace Thread
}; //namespace MNET
#endif //__MUTEX_H__
