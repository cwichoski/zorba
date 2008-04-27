/*
 * Copyright 2006-2008 The FLWOR Foundation.
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 * http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef ZORBA_RWLOCK_H
#define ZORBA_RWLOCK_H

#include "common/common.h"

#ifndef ZORBA_FOR_ONE_THREAD_ONLY

/*_____________________________________________________________________
| 
|  Read/write locks with read preferrence.
|
|  Reference: "Programming with POSIX Threads",
|  David Butenhof, Addison-Wesley [1997], Section 7.1.2.
|______________________________________________________________________*/

namespace zorba {

/*
**                                                    
** Read/write lock static initializer
*/
#if defined (WIN32)
#elif defined (HAVE_PTHREAD_H)
#define RWL_INITIALIZER \
	{	PTHREAD_MUTEX_INITIALIZER,	\
		PTHREAD_COND_INITIALIZER, 	\
		PTHREAD_COND_INITIALIZER,		\
		RWLOCK_VALID,								\
		0, 0, 0, 0 }
#endif

#define RWLOCK_VALID		0xfab
#define RWLOCK_INVALID	0xbad


class rwlock
{
protected:
#if defined (WIN32) && !defined (CYGWIN)
  HANDLE    mutex;
  HANDLE    cond_read;
  HANDLE    cond_write;
#elif defined (HAVE_PTHREAD_H)
	pthread_mutex_t		mutex;
	pthread_cond_t		read;				// wait for read
	pthread_cond_t		write;			// wait for write
#endif
	int								valid;			// set when valid
	int								r_active;		// count readers active
	int								w_active;		// boolean writers active
	int 							r_wait;			// count readers waiting
	int								w_wait;			// count writers waiting

public:
	rwlock();
	~rwlock();
	int destroy();

public:
	/**
	**                                                             
	** Handle cleanup when the read/write lock condition
	** variable wait is cancelled.
	*/
	static void read_cleanup(void* arg);
	static void write_cleanup(void* arg);

public:
	/**
	**                                                             
	** Lock a read/write lock for read access.
	*/
	int readlock();

	/**
	**                                                             
	** Try to lock a read/write lock for read access, but
	** don't block if unavailable.
	*/
	int readtrylock();

	/**
	**                                                             
	** Unlock a read/write lock from read access.
	*/
	int readunlock();

	/**
	**                                                             
	** Lock a read/write lock for write access.
	*/
	int writelock();

	/**
	**                                                             
	** Try to lock a read/write lock for write access, but
	** don't block if unavailable.
	*/
	int writetrylock();

	/**
	**                                                             
	** Unlock a read/write lock from write access.
	*/
	int writeunlock();


private:
  
  int lock_mutex();
  int unlock_mutex();
  int signal_cond_read();
  int broadcast_cond_read();
  int wait_cond_read();
  int signal_cond_write();
  int broadcast_cond_write();
  int wait_cond_write();
//  void cleanup_push_read();
//  void cleanup_pop();
//  void cleanup_push_write();

};
		

}	/* namespace zorba */

#endif//end #ifndef ZORBA_FOR_ONE_THREAD_ONLY

#endif	/* ZORBA_RWLOCK_H */
