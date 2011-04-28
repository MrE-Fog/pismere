/*
 * util/support/threads.c
 *
 * Copyright 2004,2005 by the Massachusetts Institute of Technology.
 * All Rights Reserved.
 *
 * Export of this software from the United States of America may
 *   require a specific license from the United States Government.
 *   It is the responsibility of any person or organization contemplating
 *   export to obtain such a license before exporting.
 * 
 * WITHIN THAT CONSTRAINT, permission to use, copy, modify, and
 * distribute this software and its documentation for any purpose and
 * without fee is hereby granted, provided that the above copyright
 * notice appear in all copies and that both that copyright notice and
 * this permission notice appear in supporting documentation, and that
 * the name of M.I.T. not be used in advertising or publicity pertaining
 * to distribution of the software without specific, written prior
 * permission.  Furthermore if you modify this software you must label
 * your software as modified software and not distribute it in such a
 * fashion that it might be confused with the original M.I.T. software.
 * M.I.T. makes no representations about the suitability of
 * this software for any purpose.  It is provided "as is" without express
 * or implied warranty.
 * 
 *
 * Preliminary thread support.
 */

#include <assert.h>
#include <stdlib.h>
#include <errno.h>
#include "k5-thread.h"
#include "k5-platform.h"

MAKE_INIT_FUNCTION(krb5int_thread_support_init);
MAKE_FINI_FUNCTION(krb5int_thread_support_fini);

#ifndef ENABLE_THREADS /* no thread support */

static void (*destructors[K5_KEY_MAX])(void *);
struct tsd_block { void *values[K5_KEY_MAX]; };
static struct tsd_block tsd_no_threads;
static unsigned char destructors_set[K5_KEY_MAX];

#elif defined(_WIN32)

static DWORD tls_idx;
static CRITICAL_SECTION key_lock;
struct tsd_block {
  void *values[K5_KEY_MAX];
};
static void (*destructors[K5_KEY_MAX])(void *);
static unsigned char destructors_set[K5_KEY_MAX];

void krb5int_thread_detach_hook (void)
{
    /* XXX Memory leak here!
       Need to destroy all TLS objects we know about for this thread.  */
    struct tsd_block *t;
    int i, err;

    err = CALL_INIT_FUNCTION(krb5int_thread_support_init);
    if (err)
	return;

    t = TlsGetValue(tls_idx);
    if (t == NULL)
	return;
    for (i = 0; i < K5_KEY_MAX; i++) {
	if (destructors_set[i] && destructors[i] && t->values[i]) {
	    void *v = t->values[i];
	    t->values[i] = 0;
	    (*destructors[i])(v);
	}
    }
}

/* Stub function not used on Windows. */ 
int krb5int_pthread_loaded (void)
{
    return 0;
}
#else /* POSIX threads */

/* Must support register/delete/register sequence, e.g., if krb5 is
   loaded so this support code stays in the process, and gssapi is
   loaded, unloaded, and loaded again.  */

static k5_mutex_t key_lock = K5_MUTEX_PARTIAL_INITIALIZER;
static void (*destructors[K5_KEY_MAX])(void *);
static unsigned char destructors_set[K5_KEY_MAX];

/* This is not safe yet!

   Thread termination concurrent with key deletion can cause two
   threads to interfere.  It's a bit tricky, since one of the threads
   will want to remove this structure from the list being walked by
   the other.

   Other cases, like looking up data while the library owning the key
   is in the process of being unloaded, we don't worry about.  */

struct tsd_block {
    struct tsd_block *next;
    void *values[K5_KEY_MAX];
};

#ifdef HAVE_PRAGMA_WEAK_REF
# pragma weak pthread_getspecific
# pragma weak pthread_setspecific
# pragma weak pthread_key_create
# pragma weak pthread_key_delete
# pragma weak pthread_create
# pragma weak pthread_join
static volatile int flag_pthread_loaded = -1;
static void loaded_test_aux(void)
{
    if (flag_pthread_loaded == -1)
	flag_pthread_loaded = 1;
    else
	/* Could we have been called twice?  */
	flag_pthread_loaded = 0;
}
static pthread_once_t loaded_test_once = PTHREAD_ONCE_INIT;
int krb5int_pthread_loaded (void)
{
    int x = flag_pthread_loaded;
    if (x != -1)
	return x;
    if (&pthread_getspecific == 0
	|| &pthread_setspecific == 0
	|| &pthread_key_create == 0
	|| &pthread_key_delete == 0
	|| &pthread_once == 0
	|| &pthread_mutex_lock == 0
	|| &pthread_mutex_unlock == 0
	|| &pthread_mutex_destroy == 0
	|| &pthread_mutex_init == 0
	|| &pthread_self == 0
	|| &pthread_equal == 0
	/* This catches Solaris 9.  May be redundant with the above
	   tests now.  */
# ifdef HAVE_PTHREAD_MUTEXATTR_SETROBUST_NP_IN_THREAD_LIB
	|| &pthread_mutexattr_setrobust_np == 0
# endif
	/* Any program that's really multithreaded will have to be
	   able to create threads.  */
	|| &pthread_create == 0
	|| &pthread_join == 0
	/* Okay, all the interesting functions -- or stubs for them --
	   seem to be present.  If we call pthread_once, does it
	   actually seem to cause the indicated function to get called
	   exactly one time?  */
	|| pthread_once(&loaded_test_once, loaded_test_aux) != 0
	|| pthread_once(&loaded_test_once, loaded_test_aux) != 0
	/* This catches cases where pthread_once does nothing, and
	   never causes the function to get called.  That's a pretty
	   clear violation of the POSIX spec, but hey, it happens.  */
	|| flag_pthread_loaded < 0) {
	flag_pthread_loaded = 0;
	return 0;
    }
    /* If we wanted to be super-paranoid, we could try testing whether
       pthread_get/setspecific work, too.  I don't know -- so far --
       of any system with non-functional stubs for those.  */
    return flag_pthread_loaded;
}
static struct tsd_block tsd_if_single;
# define GET_NO_PTHREAD_TSD()	(&tsd_if_single)
#else
# define GET_NO_PTHREAD_TSD()	(abort(),(struct tsd_block *)0)
#endif

static pthread_key_t key;
static void thread_termination(void *);

static void thread_termination (void *tptr)
{
    int err = k5_mutex_lock(&key_lock);
    if (err == 0) {
        int i, pass, none_found;
        struct tsd_block *t = tptr;
        
        /* Make multiple passes in case, for example, a libkrb5 cleanup
            function wants to print out an error message, which causes
            com_err to allocate a thread-specific buffer, after we just
            freed up the old one.
            
            Shouldn't actually happen, if we're careful, but check just in
            case.  */
        
        pass = 0;
        none_found = 0;
        while (pass < 4 && !none_found) {
            none_found = 1;
            for (i = 0; i < K5_KEY_MAX; i++) {
                if (destructors_set[i] && destructors[i] && t->values[i]) {
                    void *v = t->values[i];
                    t->values[i] = 0;
                    (*destructors[i])(v);
                    none_found = 0;
                }
            }
        }
        free (t);
        err = k5_mutex_unlock(&key_lock);
   }
    
    /* remove thread from global linked list */
}

#endif /* no threads vs Win32 vs POSIX */

void *k5_getspecific (k5_key_t keynum)
{
    struct tsd_block *t;
    int err;

    err = CALL_INIT_FUNCTION(krb5int_thread_support_init);
    if (err)
	return NULL;

    assert(keynum >= 0 && keynum < K5_KEY_MAX);
    assert(destructors_set[keynum] == 1);

#ifndef ENABLE_THREADS

    t = &tsd_no_threads;

#elif defined(_WIN32)

    t = TlsGetValue(tls_idx);

#else /* POSIX */

    if (K5_PTHREADS_LOADED)
	t = pthread_getspecific(key);
    else
	t = GET_NO_PTHREAD_TSD();

#endif

    if (t == NULL)
	return NULL;
    return t->values[keynum];
}

int k5_setspecific (k5_key_t keynum, void *value)
{
    struct tsd_block *t;
    int err;

    err = CALL_INIT_FUNCTION(krb5int_thread_support_init);
    if (err)
	return err;

    assert(keynum >= 0 && keynum < K5_KEY_MAX);
    assert(destructors_set[keynum] == 1);

#ifndef ENABLE_THREADS

    t = &tsd_no_threads;

#elif defined(_WIN32)

    t = TlsGetValue(tls_idx);
    if (t == NULL) {
	int i;
	t = malloc(sizeof(*t));
	if (t == NULL)
	    return errno;
	for (i = 0; i < K5_KEY_MAX; i++)
	    t->values[i] = 0;
	/* add to global linked list */
	/*	t->next = 0; */
	err = TlsSetValue(tls_idx, t);
	if (!err) {
	    free(t);
	    return GetLastError();
	}
    }

#else /* POSIX */

    if (K5_PTHREADS_LOADED) {
	t = pthread_getspecific(key);
	if (t == NULL) {
	    int i;
	    t = malloc(sizeof(*t));
	    if (t == NULL)
		return errno;
	    for (i = 0; i < K5_KEY_MAX; i++)
		t->values[i] = 0;
	    /* add to global linked list */
	    t->next = 0;
	    err = pthread_setspecific(key, t);
	    if (err) {
		free(t);
		return err;
	    }
	}
    } else {
	t = GET_NO_PTHREAD_TSD();
    }

#endif

    t->values[keynum] = value;
    return 0;
}

int k5_key_register (k5_key_t keynum, void (*destructor)(void *))
{
    int err;

    err = CALL_INIT_FUNCTION(krb5int_thread_support_init);
    if (err)
	return err;

    assert(keynum >= 0 && keynum < K5_KEY_MAX);

#ifndef ENABLE_THREADS

    assert(destructors_set[keynum] == 0);
    destructors[keynum] = destructor;
    destructors_set[keynum] = 1;
    err = 0;

#elif defined(_WIN32)

    /* XXX: This can raise EXCEPTION_POSSIBLE_DEADLOCK.  */
    EnterCriticalSection(&key_lock);
    assert(destructors_set[keynum] == 0);
    destructors_set[keynum] = 1;
    destructors[keynum] = destructor;
    LeaveCriticalSection(&key_lock);
    err = 0;

#else /* POSIX */

    err = k5_mutex_lock(&key_lock);
    if (err == 0) {
	assert(destructors_set[keynum] == 0);
	destructors_set[keynum] = 1;
	destructors[keynum] = destructor;
	err = k5_mutex_unlock(&key_lock);
    }

#endif
    return 0;
}

int k5_key_delete (k5_key_t keynum)
{
    assert(keynum >= 0 && keynum < K5_KEY_MAX);

#ifndef ENABLE_THREADS

    assert(destructors_set[keynum] == 1);
    if (destructors[keynum] && tsd_no_threads.values[keynum])
	(*destructors[keynum])(tsd_no_threads.values[keynum]);
    destructors[keynum] = 0;
    tsd_no_threads.values[keynum] = 0;
    destructors_set[keynum] = 0;

#elif defined(_WIN32)

    /* XXX: This can raise EXCEPTION_POSSIBLE_DEADLOCK.  */
    EnterCriticalSection(&key_lock);
    /* XXX Memory leak here!
       Need to destroy the associated data for all threads.
       But watch for race conditions in case threads are going away too.  */
    assert(destructors_set[keynum] == 1);
    destructors_set[keynum] = 0;
    destructors[keynum] = 0;
    LeaveCriticalSection(&key_lock);

#else /* POSIX */

    {
	int err;

	/* XXX RESOURCE LEAK:

	   Need to destroy the allocated objects first!  */

	err = k5_mutex_lock(&key_lock);
	if (err == 0) {
	    assert(destructors_set[keynum] == 1);
	    destructors_set[keynum] = 0;
	    destructors[keynum] = NULL;
	    k5_mutex_unlock(&key_lock);
	}
    }

#endif

    return 0;
}

int krb5int_call_thread_support_init (void)
{
    return CALL_INIT_FUNCTION(krb5int_thread_support_init);
}

extern int krb5int_init_fac(void);
extern void krb5int_fini_fac(void);

int krb5int_thread_support_init (void)
{
    int err;

#ifdef SHOW_INITFINI_FUNCS
    printf("krb5int_thread_support_init\n");
#endif

#ifndef ENABLE_THREADS

    /* Nothing to do for TLS initialization.  */

#elif defined(_WIN32)

    tls_idx = TlsAlloc();
    /* XXX This can raise an exception if memory is low!  */
    InitializeCriticalSection(&key_lock);

#else /* POSIX */

    err = k5_mutex_finish_init(&key_lock);
    if (err)
	return err;
    if (K5_PTHREADS_LOADED) {
	err = pthread_key_create(&key, thread_termination);
	if (err)
	    return err;
    }

#endif

    err = krb5int_init_fac();
    if (err)
	return err;

    return 0;
}

void krb5int_thread_support_fini (void)
{
    if (! INITIALIZER_RAN (krb5int_thread_support_init))
	return;

#ifdef SHOW_INITFINI_FUNCS
    printf("krb5int_thread_support_fini\n");
#endif

#ifndef ENABLE_THREADS

    /* Do nothing.  */

#elif defined(_WIN32)

    /* ... free stuff ... */
    TlsFree(tls_idx);
    DeleteCriticalSection(&key_lock);

#else /* POSIX */

    if (! INITIALIZER_RAN(krb5int_thread_support_init))
	return;
    if (K5_PTHREADS_LOADED)
	pthread_key_delete(key);
    /* ... delete stuff ... */
    k5_mutex_destroy(&key_lock);

#endif

    krb5int_fini_fac();
}

