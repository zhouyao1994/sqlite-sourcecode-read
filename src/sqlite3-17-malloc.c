/************** Begin file mutex_w32.c ***************************************/
/*
** 2007 August 14
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
** This file contains the C functions that implement mutexes for Win32.
*/
/* #include "sqliteInt.h" */

#if SQLITE_OS_WIN
/*
** Include code that is common to all os_*.c files
*/
/************** Include os_common.h in the middle of mutex_w32.c *************/
/************** Begin file os_common.h ***************************************/
/*
** 2004 May 22
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
******************************************************************************
**
** This file contains macros and a little bit of code that is common to
** all of the platform-specific files (os_*.c) and is #included into those
** files.
**
** This file should be #included by the os_*.c files only.  It is not a
** general purpose header file.
*/
#ifndef _OS_COMMON_H_
#define _OS_COMMON_H_

/*
** At least two bugs have slipped in because we changed the MEMORY_DEBUG
** macro to SQLITE_DEBUG and some older makefiles have not yet made the
** switch.  The following code should catch this problem at compile-time.
*/
#ifdef MEMORY_DEBUG
# error "The MEMORY_DEBUG macro is obsolete.  Use SQLITE_DEBUG instead."
#endif

/*
** Macros for performance tracing.  Normally turned off.  Only works
** on i486 hardware.
*/
#ifdef SQLITE_PERFORMANCE_TRACE

/*
** hwtime.h contains inline assembler code for implementing
** high-performance timing routines.
*/
/************** Include hwtime.h in the middle of os_common.h ****************/
/************** Begin file hwtime.h ******************************************/
/*
** 2008 May 27
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
******************************************************************************
**
** This file contains inline asm code for retrieving "high-performance"
** counters for x86 class CPUs.
*/
#ifndef SQLITE_HWTIME_H
#define SQLITE_HWTIME_H

/*
** The following routine only works on pentium-class (or newer) processors.
** It uses the RDTSC opcode to read the cycle count value out of the
** processor and returns that value.  This can be used for high-res
** profiling.
*/
#if (defined(__GNUC__) || defined(_MSC_VER)) && \
      (defined(i386) || defined(__i386__) || defined(_M_IX86))

  #if defined(__GNUC__)

  __inline__ sqlite_uint64 sqlite3Hwtime(void){
     unsigned int lo, hi;
     __asm__ __volatile__ ("rdtsc" : "=a" (lo), "=d" (hi));
     return (sqlite_uint64)hi << 32 | lo;
  }

  #elif defined(_MSC_VER)

  __declspec(naked) __inline sqlite_uint64 __cdecl sqlite3Hwtime(void){
     __asm {
        rdtsc
        ret       ; return value at EDX:EAX
     }
  }

  #endif

#elif (defined(__GNUC__) && defined(__x86_64__))

  __inline__ sqlite_uint64 sqlite3Hwtime(void){
      unsigned long val;
      __asm__ __volatile__ ("rdtsc" : "=A" (val));
      return val;
  }
 
#elif (defined(__GNUC__) && defined(__ppc__))

  __inline__ sqlite_uint64 sqlite3Hwtime(void){
      unsigned long long retval;
      unsigned long junk;
      __asm__ __volatile__ ("\n\
          1:      mftbu   %1\n\
                  mftb    %L0\n\
                  mftbu   %0\n\
                  cmpw    %0,%1\n\
                  bne     1b"
                  : "=r" (retval), "=r" (junk));
      return retval;
  }

#else

  #error Need implementation of sqlite3Hwtime() for your platform.

  /*
  ** To compile without implementing sqlite3Hwtime() for your platform,
  ** you can remove the above #error and use the following
  ** stub function.  You will lose timing support for many
  ** of the debugging and testing utilities, but it should at
  ** least compile and run.
  */
SQLITE_PRIVATE   sqlite_uint64 sqlite3Hwtime(void){ return ((sqlite_uint64)0); }

#endif

#endif /* !defined(SQLITE_HWTIME_H) */

/************** End of hwtime.h **********************************************/
/************** Continuing where we left off in os_common.h ******************/

static sqlite_uint64 g_start;
static sqlite_uint64 g_elapsed;
#define TIMER_START       g_start=sqlite3Hwtime()
#define TIMER_END         g_elapsed=sqlite3Hwtime()-g_start
#define TIMER_ELAPSED     g_elapsed
#else
#define TIMER_START
#define TIMER_END
#define TIMER_ELAPSED     ((sqlite_uint64)0)
#endif

/*
** If we compile with the SQLITE_TEST macro set, then the following block
** of code will give us the ability to simulate a disk I/O error.  This
** is used for testing the I/O recovery logic.
*/
#if defined(SQLITE_TEST)
SQLITE_API extern int sqlite3_io_error_hit;
SQLITE_API extern int sqlite3_io_error_hardhit;
SQLITE_API extern int sqlite3_io_error_pending;
SQLITE_API extern int sqlite3_io_error_persist;
SQLITE_API extern int sqlite3_io_error_benign;
SQLITE_API extern int sqlite3_diskfull_pending;
SQLITE_API extern int sqlite3_diskfull;
#define SimulateIOErrorBenign(X) sqlite3_io_error_benign=(X)
#define SimulateIOError(CODE)  \
  if( (sqlite3_io_error_persist && sqlite3_io_error_hit) \
       || sqlite3_io_error_pending-- == 1 )  \
              { local_ioerr(); CODE; }
static void local_ioerr(){
  IOTRACE(("IOERR\n"));
  sqlite3_io_error_hit++;
  if( !sqlite3_io_error_benign ) sqlite3_io_error_hardhit++;
}
#define SimulateDiskfullError(CODE) \
   if( sqlite3_diskfull_pending ){ \
     if( sqlite3_diskfull_pending == 1 ){ \
       local_ioerr(); \
       sqlite3_diskfull = 1; \
       sqlite3_io_error_hit = 1; \
       CODE; \
     }else{ \
       sqlite3_diskfull_pending--; \
     } \
   }
#else
#define SimulateIOErrorBenign(X)
#define SimulateIOError(A)
#define SimulateDiskfullError(A)
#endif /* defined(SQLITE_TEST) */

/*
** When testing, keep a count of the number of open files.
*/
#if defined(SQLITE_TEST)
SQLITE_API extern int sqlite3_open_file_count;
#define OpenCounter(X)  sqlite3_open_file_count+=(X)
#else
#define OpenCounter(X)
#endif /* defined(SQLITE_TEST) */

#endif /* !defined(_OS_COMMON_H_) */

/************** End of os_common.h *******************************************/
/************** Continuing where we left off in mutex_w32.c ******************/

/*
** Include the header file for the Windows VFS.
*/
/************** Include os_win.h in the middle of mutex_w32.c ****************/
/************** Begin file os_win.h ******************************************/
/*
** 2013 November 25
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
******************************************************************************
**
** This file contains code that is specific to Windows.
*/
#ifndef SQLITE_OS_WIN_H
#define SQLITE_OS_WIN_H

/*
** Include the primary Windows SDK header file.
*/
#include "windows.h"

#ifdef __CYGWIN__
# include <sys/cygwin.h>
# include <errno.h> /* amalgamator: dontcache */
#endif

/*
** Determine if we are dealing with Windows NT.
**
** We ought to be able to determine if we are compiling for Windows 9x or
** Windows NT using the _WIN32_WINNT macro as follows:
**
** #if defined(_WIN32_WINNT)
** # define SQLITE_OS_WINNT 1
** #else
** # define SQLITE_OS_WINNT 0
** #endif
**
** However, Visual Studio 2005 does not set _WIN32_WINNT by default, as
** it ought to, so the above test does not work.  We'll just assume that
** everything is Windows NT unless the programmer explicitly says otherwise
** by setting SQLITE_OS_WINNT to 0.
*/
#if SQLITE_OS_WIN && !defined(SQLITE_OS_WINNT)
# define SQLITE_OS_WINNT 1
#endif

/*
** Determine if we are dealing with Windows CE - which has a much reduced
** API.
*/
#if defined(_WIN32_WCE)
# define SQLITE_OS_WINCE 1
#else
# define SQLITE_OS_WINCE 0
#endif

/*
** Determine if we are dealing with WinRT, which provides only a subset of
** the full Win32 API.
*/
#if !defined(SQLITE_OS_WINRT)
# define SQLITE_OS_WINRT 0
#endif

/*
** For WinCE, some API function parameters do not appear to be declared as
** volatile.
*/
#if SQLITE_OS_WINCE
# define SQLITE_WIN32_VOLATILE
#else
# define SQLITE_WIN32_VOLATILE volatile
#endif

/*
** For some Windows sub-platforms, the _beginthreadex() / _endthreadex()
** functions are not available (e.g. those not using MSVC, Cygwin, etc).
*/
#if SQLITE_OS_WIN && !SQLITE_OS_WINCE && !SQLITE_OS_WINRT && \
    SQLITE_THREADSAFE>0 && !defined(__CYGWIN__)
# define SQLITE_OS_WIN_THREADS 1
#else
# define SQLITE_OS_WIN_THREADS 0
#endif

#endif /* SQLITE_OS_WIN_H */

/************** End of os_win.h **********************************************/
/************** Continuing where we left off in mutex_w32.c ******************/
#endif

/*
** The code in this file is only used if we are compiling multithreaded
** on a Win32 system.
*/
#ifdef SQLITE_MUTEX_W32

/*
** Each recursive mutex is an instance of the following structure.
*/
struct sqlite3_mutex {
  CRITICAL_SECTION mutex;    /* Mutex controlling the lock */
  int id;                    /* Mutex type */
#ifdef SQLITE_DEBUG
  volatile int nRef;         /* Number of enterances */
  volatile DWORD owner;      /* Thread holding this mutex */
  volatile int trace;        /* True to trace changes */
#endif
};

/*
** These are the initializer values used when declaring a "static" mutex
** on Win32.  It should be noted that all mutexes require initialization
** on the Win32 platform.
*/
#define SQLITE_W32_MUTEX_INITIALIZER { 0 }

#ifdef SQLITE_DEBUG
#define SQLITE3_MUTEX_INITIALIZER { SQLITE_W32_MUTEX_INITIALIZER, 0, \
                                    0L, (DWORD)0, 0 }
#else
#define SQLITE3_MUTEX_INITIALIZER { SQLITE_W32_MUTEX_INITIALIZER, 0 }
#endif

#ifdef SQLITE_DEBUG
/*
** The sqlite3_mutex_held() and sqlite3_mutex_notheld() routine are
** intended for use only inside assert() statements.
*/
static int winMutexHeld(sqlite3_mutex *p){
  return p->nRef!=0 && p->owner==GetCurrentThreadId();
}

static int winMutexNotheld2(sqlite3_mutex *p, DWORD tid){
  return p->nRef==0 || p->owner!=tid;
}

static int winMutexNotheld(sqlite3_mutex *p){
  DWORD tid = GetCurrentThreadId();
  return winMutexNotheld2(p, tid);
}
#endif

/*
** Try to provide a memory barrier operation, needed for initialization
** and also for the xShmBarrier method of the VFS in cases when SQLite is
** compiled without mutexes (SQLITE_THREADSAFE=0).
*/
SQLITE_PRIVATE void sqlite3MemoryBarrier(void){
#if defined(SQLITE_MEMORY_BARRIER)
  SQLITE_MEMORY_BARRIER;
#elif defined(__GNUC__)
  __sync_synchronize();
#elif !defined(SQLITE_DISABLE_INTRINSIC) && \
      defined(_MSC_VER) && _MSC_VER>=1300
  _ReadWriteBarrier();
#elif defined(MemoryBarrier)
  MemoryBarrier();
#endif
}

/*
** Initialize and deinitialize the mutex subsystem.
*/
static sqlite3_mutex winMutex_staticMutexes[] = {
  SQLITE3_MUTEX_INITIALIZER,
  SQLITE3_MUTEX_INITIALIZER,
  SQLITE3_MUTEX_INITIALIZER,
  SQLITE3_MUTEX_INITIALIZER,
  SQLITE3_MUTEX_INITIALIZER,
  SQLITE3_MUTEX_INITIALIZER,
  SQLITE3_MUTEX_INITIALIZER,
  SQLITE3_MUTEX_INITIALIZER,
  SQLITE3_MUTEX_INITIALIZER,
  SQLITE3_MUTEX_INITIALIZER,
  SQLITE3_MUTEX_INITIALIZER,
  SQLITE3_MUTEX_INITIALIZER
};

static int winMutex_isInit = 0;
static int winMutex_isNt = -1; /* <0 means "need to query" */

/* As the winMutexInit() and winMutexEnd() functions are called as part
** of the sqlite3_initialize() and sqlite3_shutdown() processing, the
** "interlocked" magic used here is probably not strictly necessary.
*/
static LONG SQLITE_WIN32_VOLATILE winMutex_lock = 0;

SQLITE_API int sqlite3_win32_is_nt(void); /* os_win.c */
SQLITE_API void sqlite3_win32_sleep(DWORD milliseconds); /* os_win.c */

static int winMutexInit(void){
  /* The first to increment to 1 does actual initialization */
  if( InterlockedCompareExchange(&winMutex_lock, 1, 0)==0 ){
    int i;
    for(i=0; i<ArraySize(winMutex_staticMutexes); i++){
#if SQLITE_OS_WINRT
      InitializeCriticalSectionEx(&winMutex_staticMutexes[i].mutex, 0, 0);
#else
      InitializeCriticalSection(&winMutex_staticMutexes[i].mutex);
#endif
    }
    winMutex_isInit = 1;
  }else{
    /* Another thread is (in the process of) initializing the static
    ** mutexes */
    while( !winMutex_isInit ){
      sqlite3_win32_sleep(1);
    }
  }
  return SQLITE_OK;
}

static int winMutexEnd(void){
  /* The first to decrement to 0 does actual shutdown
  ** (which should be the last to shutdown.) */
  if( InterlockedCompareExchange(&winMutex_lock, 0, 1)==1 ){
    if( winMutex_isInit==1 ){
      int i;
      for(i=0; i<ArraySize(winMutex_staticMutexes); i++){
        DeleteCriticalSection(&winMutex_staticMutexes[i].mutex);
      }
      winMutex_isInit = 0;
    }
  }
  return SQLITE_OK;
}

/*
** The sqlite3_mutex_alloc() routine allocates a new
** mutex and returns a pointer to it.  If it returns NULL
** that means that a mutex could not be allocated.  SQLite
** will unwind its stack and return an error.  The argument
** to sqlite3_mutex_alloc() is one of these integer constants:
**
** <ul>
** <li>  SQLITE_MUTEX_FAST
** <li>  SQLITE_MUTEX_RECURSIVE
** <li>  SQLITE_MUTEX_STATIC_MASTER
** <li>  SQLITE_MUTEX_STATIC_MEM
** <li>  SQLITE_MUTEX_STATIC_OPEN
** <li>  SQLITE_MUTEX_STATIC_PRNG
** <li>  SQLITE_MUTEX_STATIC_LRU
** <li>  SQLITE_MUTEX_STATIC_PMEM
** <li>  SQLITE_MUTEX_STATIC_APP1
** <li>  SQLITE_MUTEX_STATIC_APP2
** <li>  SQLITE_MUTEX_STATIC_APP3
** <li>  SQLITE_MUTEX_STATIC_VFS1
** <li>  SQLITE_MUTEX_STATIC_VFS2
** <li>  SQLITE_MUTEX_STATIC_VFS3
** </ul>
**
** The first two constants cause sqlite3_mutex_alloc() to create
** a new mutex.  The new mutex is recursive when SQLITE_MUTEX_RECURSIVE
** is used but not necessarily so when SQLITE_MUTEX_FAST is used.
** The mutex implementation does not need to make a distinction
** between SQLITE_MUTEX_RECURSIVE and SQLITE_MUTEX_FAST if it does
** not want to.  But SQLite will only request a recursive mutex in
** cases where it really needs one.  If a faster non-recursive mutex
** implementation is available on the host platform, the mutex subsystem
** might return such a mutex in response to SQLITE_MUTEX_FAST.
**
** The other allowed parameters to sqlite3_mutex_alloc() each return
** a pointer to a static preexisting mutex.  Six static mutexes are
** used by the current version of SQLite.  Future versions of SQLite
** may add additional static mutexes.  Static mutexes are for internal
** use by SQLite only.  Applications that use SQLite mutexes should
** use only the dynamic mutexes returned by SQLITE_MUTEX_FAST or
** SQLITE_MUTEX_RECURSIVE.
**
** Note that if one of the dynamic mutex parameters (SQLITE_MUTEX_FAST
** or SQLITE_MUTEX_RECURSIVE) is used then sqlite3_mutex_alloc()
** returns a different mutex on every call.  But for the static
** mutex types, the same mutex is returned on every call that has
** the same type number.
*/
static sqlite3_mutex *winMutexAlloc(int iType){
  sqlite3_mutex *p;

  switch( iType ){
    case SQLITE_MUTEX_FAST:
    case SQLITE_MUTEX_RECURSIVE: {
      p = sqlite3MallocZero( sizeof(*p) );
      if( p ){
        p->id = iType;
#ifdef SQLITE_DEBUG
#ifdef SQLITE_WIN32_MUTEX_TRACE_DYNAMIC
        p->trace = 1;
#endif
#endif
#if SQLITE_OS_WINRT
        InitializeCriticalSectionEx(&p->mutex, 0, 0);
#else
        InitializeCriticalSection(&p->mutex);
#endif
      }
      break;
    }
    default: {
#ifdef SQLITE_ENABLE_API_ARMOR
      if( iType-2<0 || iType-2>=ArraySize(winMutex_staticMutexes) ){
        (void)SQLITE_MISUSE_BKPT;
        return 0;
      }
#endif
      p = &winMutex_staticMutexes[iType-2];
      p->id = iType;
#ifdef SQLITE_DEBUG
#ifdef SQLITE_WIN32_MUTEX_TRACE_STATIC
      p->trace = 1;
#endif
#endif
      break;
    }
  }
  return p;
}


/*
** This routine deallocates a previously
** allocated mutex.  SQLite is careful to deallocate every
** mutex that it allocates.
*/
static void winMutexFree(sqlite3_mutex *p){
  assert( p );
  assert( p->nRef==0 && p->owner==0 );
  if( p->id==SQLITE_MUTEX_FAST || p->id==SQLITE_MUTEX_RECURSIVE ){
    DeleteCriticalSection(&p->mutex);
    sqlite3_free(p);
  }else{
#ifdef SQLITE_ENABLE_API_ARMOR
    (void)SQLITE_MISUSE_BKPT;
#endif
  }
}

/*
** The sqlite3_mutex_enter() and sqlite3_mutex_try() routines attempt
** to enter a mutex.  If another thread is already within the mutex,
** sqlite3_mutex_enter() will block and sqlite3_mutex_try() will return
** SQLITE_BUSY.  The sqlite3_mutex_try() interface returns SQLITE_OK
** upon successful entry.  Mutexes created using SQLITE_MUTEX_RECURSIVE can
** be entered multiple times by the same thread.  In such cases the,
** mutex must be exited an equal number of times before another thread
** can enter.  If the same thread tries to enter any other kind of mutex
** more than once, the behavior is undefined.
*/
static void winMutexEnter(sqlite3_mutex *p){
#if defined(SQLITE_DEBUG) || defined(SQLITE_TEST)
  DWORD tid = GetCurrentThreadId();
#endif
#ifdef SQLITE_DEBUG
  assert( p );
  assert( p->id==SQLITE_MUTEX_RECURSIVE || winMutexNotheld2(p, tid) );
#else
  assert( p );
#endif
  assert( winMutex_isInit==1 );
  EnterCriticalSection(&p->mutex);
#ifdef SQLITE_DEBUG
  assert( p->nRef>0 || p->owner==0 );
  p->owner = tid;
  p->nRef++;
  if( p->trace ){
    OSTRACE(("ENTER-MUTEX tid=%lu, mutex=%p (%d), nRef=%d\n",
             tid, p, p->trace, p->nRef));
  }
#endif
}

static int winMutexTry(sqlite3_mutex *p){
#if defined(SQLITE_DEBUG) || defined(SQLITE_TEST)
  DWORD tid = GetCurrentThreadId();
#endif
  int rc = SQLITE_BUSY;
  assert( p );
  assert( p->id==SQLITE_MUTEX_RECURSIVE || winMutexNotheld2(p, tid) );
  /*
  ** The sqlite3_mutex_try() routine is very rarely used, and when it
  ** is used it is merely an optimization.  So it is OK for it to always
  ** fail.
  **
  ** The TryEnterCriticalSection() interface is only available on WinNT.
  ** And some windows compilers complain if you try to use it without
  ** first doing some #defines that prevent SQLite from building on Win98.
  ** For that reason, we will omit this optimization for now.  See
  ** ticket #2685.
  */
#if defined(_WIN32_WINNT) && _WIN32_WINNT >= 0x0400
  assert( winMutex_isInit==1 );
  assert( winMutex_isNt>=-1 && winMutex_isNt<=1 );
  if( winMutex_isNt<0 ){
    winMutex_isNt = sqlite3_win32_is_nt();
  }
  assert( winMutex_isNt==0 || winMutex_isNt==1 );
  if( winMutex_isNt && TryEnterCriticalSection(&p->mutex) ){
#ifdef SQLITE_DEBUG
    p->owner = tid;
    p->nRef++;
#endif
    rc = SQLITE_OK;
  }
#else
  UNUSED_PARAMETER(p);
#endif
#ifdef SQLITE_DEBUG
  if( p->trace ){
    OSTRACE(("TRY-MUTEX tid=%lu, mutex=%p (%d), owner=%lu, nRef=%d, rc=%s\n",
             tid, p, p->trace, p->owner, p->nRef, sqlite3ErrName(rc)));
  }
#endif
  return rc;
}

/*
** The sqlite3_mutex_leave() routine exits a mutex that was
** previously entered by the same thread.  The behavior
** is undefined if the mutex is not currently entered or
** is not currently allocated.  SQLite will never do either.
*/
static void winMutexLeave(sqlite3_mutex *p){
#if defined(SQLITE_DEBUG) || defined(SQLITE_TEST)
  DWORD tid = GetCurrentThreadId();
#endif
  assert( p );
#ifdef SQLITE_DEBUG
  assert( p->nRef>0 );
  assert( p->owner==tid );
  p->nRef--;
  if( p->nRef==0 ) p->owner = 0;
  assert( p->nRef==0 || p->id==SQLITE_MUTEX_RECURSIVE );
#endif
  assert( winMutex_isInit==1 );
  LeaveCriticalSection(&p->mutex);
#ifdef SQLITE_DEBUG
  if( p->trace ){
    OSTRACE(("LEAVE-MUTEX tid=%lu, mutex=%p (%d), nRef=%d\n",
             tid, p, p->trace, p->nRef));
  }
#endif
}

SQLITE_PRIVATE sqlite3_mutex_methods const *sqlite3DefaultMutex(void){
  static const sqlite3_mutex_methods sMutex = {
    winMutexInit,
    winMutexEnd,
    winMutexAlloc,
    winMutexFree,
    winMutexEnter,
    winMutexTry,
    winMutexLeave,
#ifdef SQLITE_DEBUG
    winMutexHeld,
    winMutexNotheld
#else
    0,
    0
#endif
  };
  return &sMutex;
}

#endif /* SQLITE_MUTEX_W32 */

/************** End of mutex_w32.c *******************************************/
