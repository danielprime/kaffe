/*
 * jthread.h
 *
 * Thread support using internal system.
 *
 * Copyright (c) 1996, 1997, 1998
 *      Transvirtual Technologies, Inc.  All rights reserved.
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file.
 *
 * Written by Godmar Back <gback@cs.utah.edu> and
 *            Tim Wilkinson <tim@transvirtual.com>
 */

#ifndef __jthread_h
#define __jthread_h

/*
 * The file is independent of Kaffe, but if compiled as part of the Kaffe
 * build process, it will use Kaffe's constants and config options.
 * This should make it machine-independent or at least aid in porting it.
 *
 * We use the preprocessor constant "KVER" to determine whether that is
 * the case or not.
 */
#if defined(KVER)

/*======== 	begin of definitions that apply to Kaffe 	     ========*/

#include "debug.h"
#include "config.h"
#include "config-setjmp.h"
#include "config-std.h"
#include "config-mem.h"
#include "config-io.h"
#include "config-signal.h"
#include "gtypes.h"		/* for jlong */
#include "lerrno.h"
#include "support.h"		/* XXX: for currentTime */
#include "md.h"

#define  NOTIMEOUT                       0

#if defined(__WIN32__)
#define SIG_T   void(*)()
#else
#define SIG_T   void*
#endif

/*======== 	end of definitions that apply to Kaffe 	     	     ========*/

#else	/* !KVER */

/*
 * all definitions for compilation under plain UNIX are in this file
 */
#include "config-jthreads.h"

/*======== end of definitions that apply to plain UNIX only ========*/

#endif  /* !KVER */

/*
 * a mutex is represented by a two-word structure
 */
typedef struct _jmutex
{
        struct _jthread *holder;
        struct _jthread *waiting;
} jmutex;

/*
 * a condition variable is simply expressed as a list of threads
 * waiting to be notified
 */
typedef struct _jthread *jcondvar;

/*
 * This is our internal structure representing the "native" threads.
 * This used to be called "ctx".
 */
typedef struct _jthread {
	unsigned char			status;
	unsigned char			priority;
	void*				restorePoint;
	void*				stackBase;
	void*				stackEnd;
	jlong				time;
	struct _jthread*		nextQ;
	struct _jthread*		nextlive;
	struct _jthread*		nextalarm;
	struct _jthread**		blockqueue;
	unsigned char			flags;
	void				(*func)(void *);
	int				daemon;

	/* this one is simply thread specific data, or a cookie -
	 * used to hold the current Java thread
	 */
	void*				jlThread;
	JTHREAD_JMPBUF			env;
#if defined(SAVED_FP_SIZE)
	char				fpstate[SAVED_FP_SIZE];
#endif
	/* 
	 * note that this causes gdb under Solaris to crash when trying to
	 * print a struct jthread
	 */
	/* for alignment (Gcc extension) */
	double				align[0];
} jthread, *jthread_t;

#define GET_COOKIE()	(jthread_current()->jlThread)
#define SET_COOKIE(x)	(jthread_current()->jlThread = (void*)(x))
extern jthread_t currentJThread;

/****************************************************************************
 *
 * The following functions form the interface provided by the 
 * revised internal threading system.
 */

/* 
 * initialize the threading system
 */
jthread_t 
jthread_init(
	int preemptive,			/* preemptive scheduling */
	int maxpr, 			/* maximum priority */
	int minpr, 			/* minimum priority */
	int mainthreadpr,		/* priority of current (main) thread */
	size_t mainThreadStackSize,	/* assumed main stack size */
        void *(*_allocator)(size_t),	/* memory allocator */
	void (*_deallocator)(void*),	/* memory deallocator */
	void (*_destructor1)(void*),	/* called when a thread exits */ 
	void (*_onstop)(void), 		/* called when a thread is stopped */
	void (*_ondeadlock)(void)); 	/* called when we detect deadlock */

/*
 * create a thread with a given priority
 */
jthread_t
jthread_create(unsigned char pri, 	/* initial priority */
	void (*func)(void *), 		/* start function */
	int daemon, 			/* is this thread a daemon? */
	void *jlThread, 		/* cookie for this thread */
	size_t threadStackSize);	/* stack size to be allocated */

struct _exceptionFrame;
typedef void (*exchandler_t)(struct _exceptionFrame*);

/*
 * Register handlers for null pointer accesses and floating point exceptions
 */
void
jthread_initexceptions(exchandler_t _nullHandler,
                       exchandler_t _floatingHandler);

/*
 * set a function to be run when last non-daemon dies 
 * this is used to run the finalizer on exit.
 */
void 	jthread_atexit(void (*f)(void));

/*
 * walk all live threads, and invoke `func', passing in their cookie
 * this is used with `func' set to walkMemory
 */
void 	jthread_walkLiveThreads(void (*func)(void *jlThread));

/* 
 * destroy this jthread structure 
 */
void	jthread_destroy(jthread_t jtid);

/*
 * set the priority of a thread 
 */
void	jthread_setpriority(jthread_t jtid, int prio);

/*
 * yield to another thread
 */
void 	jthread_yield(void);

/*
 * sleep for time milliseconds
 */
void 	jthread_sleep(jlong time);

/* 
 * return the current thread 
 */
static inline jthread_t
jthread_current(void) 
{ 
	return currentJThread; 
}

/* 
 * count the number of stack frames - unimplemented 
 */
int 	jthread_frames(jthread_t thrd);

/* 
 * return whether this thread is alive or not
 */
int 	jthread_alive(jthread_t jtid);

/*
 * stop this thread
 */
void 	jthread_stop(jthread_t jtid);

/*
 * interrupt this thread
 */
void 	jthread_interrupt(jthread_t jtid);

/*
 * have the current thread exit
 */
void 	jthread_exit(void) __NORETURN__;

/*
 * determine whether a location is on the stack of the current thread
 */
static inline int
jthread_on_current_stack(void *bp)      
{
        return bp >= currentJThread->stackBase && bp < currentJThread->stackEnd;
}

/* 
 * Check for room on stack.
 */
static inline int     
jthread_stackcheck(int left)
{
	int rc;
#if defined(STACK_GROWS_UP)
        rc = jthread_on_current_stack((void*)&rc + left);
#else
        rc = jthread_on_current_stack((void*)&rc - left);
#endif
	return (rc);
}

/*
 * determine the "interesting" stack range a conservative gc must walk
 */
int jthread_extract_stack(jthread_t jtid, void **from, unsigned *len);

/*
 * Disallow cancellation for current thread
 */
void jthread_disable_stop(void);

/*
 * Reallow cancellation for current thread
 * If a cancellation is pending, the stop method will be called
 */
void jthread_enable_stop(void);

/*
 * functions to disable and restore interrupts
 * These are *not* part of the public interface.
 */
#define	INTS_DISABLED()		intsDisabled()
int 	intsDisabled(void);

/*
 * Prevent all other threads from running.
 * In this uniprocessor implementation, this is simple.
 */
void jthread_suspendall(void);

/*
 * Reallow other threads.
 * In this uniprocessor implementation, this is simple.
 */
void jthread_unsuspendall(void);

/*
 * Print info about a given jthread to stderr
 */
void jthread_dumpthreadinfo(jthread_t tid);

/*
 * return thread-specific data for a given jthread
 */
static inline void*
jthread_getcookie(jthread_t tid) 
{
	return (tid->jlThread);
}

/*
 * API related to I/O
 */
int jthreadedOpen(const char* path, int flags, int mode, int*);
int jthreadedSocket(int af, int type, int proto, int*);
int jthreadedConnect(int fd, struct sockaddr* addr, size_t len);
int jthreadedAccept(int fd, struct sockaddr* addr, size_t* len, 
	int timeout, ssize_t *);
int jthreadedRead(int fd, void* buf, size_t len, ssize_t *);
int jthreadedTimedRead(int fd, void* buf, size_t len, int timeout, ssize_t *);
int jthreadedWrite(int fd, const void* buf, size_t len, ssize_t *);
int jthreadedRecvfrom(int fd, void* buf, size_t len, int flags,
        struct sockaddr* from, int* fromlen, int timeout, ssize_t *);
int jthreadedWaitpid(int wpid, int* status, int options, int *);
int jthreadedForkExec(char **argv, char **arge, int ioes[4], int *);

/* 
 * Locking API
 */
void jmutex_initialise(jmutex *lock);
void jmutex_lock(jmutex *lock);
void jmutex_unlock(jmutex *lock);

#define JMUTEX_BLOCKED
/* Return a list of threads blocked on a mutex.  Caller frees */
int  jmutex_blocked(jmutex *lock, jthread_t **list);

void jcondvar_initialise(jcondvar *cv);
void jcondvar_wait(jcondvar *cv, jmutex *lock, jlong timeout);
void jcondvar_signal(jcondvar *cv, jmutex *lock);
void jcondvar_broadcast(jcondvar *cv, jmutex *lock);

#define JCONDVAR_WAITING
/* Return a list of threads waiting on a condvar.  Caller frees */
int  jcondvar_waiting(jcondvar *cv, jthread_t **list);

/* Spinlocks: simple since we're uniprocessor */
/* ARGSUSED */
static inline
void jthread_spinon(void *arg)
{
	jthread_suspendall();
}

/* ARGSUSED */
static inline
void jthread_spinoff(void *arg)
{
	jthread_unsuspendall();
}

#endif
