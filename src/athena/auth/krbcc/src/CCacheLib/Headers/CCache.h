/* $Copyright:
 *
 * Copyright 1998-2000 by the Massachusetts Institute of Technology.
 * 
 * All rights reserved.
 * 
 * Export of this software from the United States of America may require a
 * specific license from the United States Government.  It is the
 * responsibility of any person or organization contemplating export to
 * obtain such a license before exporting.
 * 
 * WITHIN THAT CONSTRAINT, permission to use, copy, modify, and distribute
 * this software and its documentation for any purpose and without fee is
 * hereby granted, provided that the above copyright notice appear in all
 * copies and that both that copyright notice and this permission notice
 * appear in supporting documentation, and that the name of M.I.T. not be
 * used in advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.  Furthermore if you
 * modify this software you must label your software as modified software
 * and not distribute it in such a fashion that it might be confused with
 * the original MIT software. M.I.T. makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 * 
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
 * MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 * Individual source code files are copyright MIT, Cygnus Support,
 * OpenVision, Oracle, Sun Soft, FundsXpress, and others.
 * 
 * Project Athena, Athena, Athena MUSE, Discuss, Hesiod, Kerberos, Moira,
 * and Zephyr are trademarks of the Massachusetts Institute of Technology
 * (MIT).  No commercial use of these trademarks may be made without prior
 * written permission of MIT.
 * 
 * "Commercial use" means use of a name in a product or other for-profit
 * manner.  It does NOT prevent a commercial firm from referring to the MIT
 * trademarks in order to convey information (although in doing so,
 * recognition of their trademark status should be given).
 * $
 */

/* $Header: /cvs/pismere/pismere/athena/auth/krbcc/src/CCacheLib/Headers/CCache.h,v 1.3 2000/06/15 15:15:16 dalmeida Exp $ */

/*
 * Declarations for Credentials Cache API Library
 *
 * API specification: <http://web.mit.edu/pismere/kerberos/ccache-api-v2.html>
 *
 *	Revision 1: Frank Dabek, 6/4/1998
 *	Revision 2: meeroh, 2/24/1999
 *      Revision 3: meeroh, 11/12/1999
 *
 */
 
#ifndef CCache_h
#define CCache_h

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <ConditionalMacros.h>

#include <MacTypes.h>

#if PRAGMA_IMPORT
#	pragma import on
#endif

/* This stuff is to make sure that we always use the same compiler options for
   this header file. Otherwise we get really exciting failure modes -- meeroh */
#if PRAGMA_STRUCT_ALIGN
	#pragma options align=mac68k4byte
#elif PRAGMA_STRUCT_PACKPUSH
	#pragma pack(push, 4)
#elif PRAGMA_STRUCT_PACK
	#pragma pack(4)
#endif

#if PRAGMA_ENUM_ALWAYSINT
	#pragma enumsalwaysint on
#endif

#if TARGET_CPU_68K
	#pragma fourbyteints on
#endif 

/*
 * Constants
 */
 
/* API versions */
enum {
	ccapi_version_2 = 2,
	ccapi_version_3 = 3
};
 
/* Errors */
enum {
	ccNoError							= 0,

	ccIteratorEnd						= 201,
	ccErrBadParam,
	ccErrNoMem,
	ccErrInvalidContext,
	ccErrInvalidCCache,

	ccErrInvalidString,					/* 206 */
	ccErrInvalidCredentials,
	ccErrInvalidCCacheIterator,
	ccErrInvalidCredentialsIterator,
	ccErrInvalidLock,

	ccErrBadName,						/* 211 */
	ccErrBadCredentialsVersion,
	ccErrBadAPIVersion,
	ccErrContextLocked,
	ccErrContextUnlocked,

	ccErrCCacheLocked,					/* 216 */
	ccErrCCacheUnlocked,
	ccErrBadLockType,
	ccErrNeverDefault,
	ccErrCredentialsNotFound,

	ccErrCCacheNotFound,				/* 221 */
	ccErrContextNotFound
};

/* Credentials versions */
enum {
	cc_credentials_v4 = 1,
	cc_credentials_v5 = 2,
	cc_credentials_v4_v5 = 3
};

/*
 * Basic types
 */
 
typedef UInt32 cc_uint32;
typedef SInt32 cc_int32;
typedef cc_uint32 cc_time_t;

/*
 * API types
 */
 
/* Forward declarations */
struct cc_context_f;
typedef struct cc_context_f cc_context_f;

struct cc_ccache_f;
typedef struct cc_ccache_f cc_ccache_f;

struct cc_ccache_iterator_f;
typedef struct cc_ccache_iterator_f cc_ccache_iterator_f;

struct cc_ccache_iterator_f;
typedef struct cc_credentials_iterator_f cc_credentials_iterator_f;

struct cc_string_f;
typedef struct cc_string_f cc_string_f;

struct cc_credentials_f;
typedef struct cc_credentials_f cc_credentials_f;

struct cc_lock_f;
typedef struct cc_lock_f cc_lock_f;

/* Credentials types */

enum {	/* Make sure all of these are multiples of four (for alignment sanity) */
	cc_v4_name_size		= 40,
	cc_v4_instance_size	= 40,
	cc_v4_realm_size	= 40,
	cc_v4_ticket_size	= 1254
};

enum cc_string_to_key_type {
	cc_v4_stk_afs = 0,
	cc_v4_stk_des = 1,
	cc_v4_stk_columbia_special = 2,
	cc_v4_stk_unknown = 3
};

struct cc_credentials_v4_t {
	cc_uint32			version;
	char				principal [cc_v4_name_size];
	char				principal_instance [cc_v4_instance_size];
	char				service [cc_v4_name_size];
	char				service_instance [cc_v4_instance_size];
	char				realm [cc_v4_realm_size];
	unsigned char		session_key [8];
	cc_int32			kvno;
	cc_int32			string_to_key_type;
	cc_time_t			issue_date;
	cc_int32			lifetime;
	cc_uint32			address;
	cc_int32			ticket_size;
	unsigned char		ticket [cc_v4_ticket_size];
};
typedef struct cc_credentials_v4_t cc_credentials_v4_t;

struct cc_data {
	cc_uint32			type;
	cc_uint32			length;
	void*				data;
};
typedef struct cc_data cc_data;

struct cc_credentials_v5_t {
	char*				client;
	char*				server;
	cc_data				keyblock;
	cc_time_t			authtime;
	cc_time_t			starttime;
	cc_time_t			endtime;
	cc_time_t			renew_till;
	cc_uint32			is_skey;
	cc_uint32			ticket_flags;
	cc_data**			addresses;
	cc_data				ticket;
	cc_data				second_ticket;
	cc_data**			authdata;
};
typedef struct cc_credentials_v5_t cc_credentials_v5_t;

struct cc_credentials_union {
	cc_int32			version;
	union {
		cc_credentials_v4_t*	credentials_v4;
		cc_credentials_v5_t*	credentials_v5;
	}					credentials;
};
typedef struct cc_credentials_union cc_credentials_union;

/* Exposed parts */

struct cc_context_d {
	const cc_context_f*	functions;
};
typedef struct cc_context_d cc_context_d;
typedef cc_context_d*	cc_context_t;

struct cc_ccache_d {
	const cc_ccache_f*	functions;
};
typedef struct cc_ccache_d cc_ccache_d;
typedef cc_ccache_d*	cc_ccache_t;

struct cc_ccache_iterator_d {
	const cc_ccache_iterator_f*	functions;
};
typedef struct cc_ccache_iterator_d cc_ccache_iterator_d;
typedef cc_ccache_iterator_d*	cc_ccache_iterator_t;

struct cc_credentials_iterator_d {
	const cc_credentials_iterator_f*	functions;
};
typedef struct cc_credentials_iterator_d cc_credentials_iterator_d;
typedef cc_credentials_iterator_d*	cc_credentials_iterator_t;

struct cc_string_d {
	const char*			data;
	const cc_string_f*	functions;
};
typedef struct cc_string_d cc_string_d;
typedef cc_string_d*	cc_string_t;

struct cc_credentials_d {
	const cc_credentials_union* data;
	const cc_credentials_f* functions;
};
typedef struct cc_credentials_d cc_credentials_d;
typedef cc_credentials_d* cc_credentials_t;

struct cc_lock_d {
	const cc_lock_f*	functions;
};
typedef struct cc_lock_d cc_lock_d;
typedef cc_lock_d*	cc_lock_t;

/* Function pointer structs */

struct  cc_context_f {
    cc_int32    (*release) (
                                cc_context_t context);
    cc_int32    (*get_version) (
                                cc_context_t context,
                                cc_int32* version);
    cc_int32    (*clone) (
                                cc_context_t context,
                                cc_context_t* new_context,
                                cc_int32 api_version,
                                cc_int32* supported_version,
                                char const** vendor);
    cc_int32    (*get_change_time) (
                                cc_context_t context,
                                cc_time_t* time);
    cc_int32    (*get_default_ccache_name) (
                                cc_context_t context,
                                cc_string_t* name);
    cc_int32    (*open_ccache) (
                                cc_context_t context,
                                const char* name,
                                cc_ccache_t* ccache);
    cc_int32    (*open_default_ccache) (
                                cc_context_t context,
                                cc_ccache_t* ccache);
    cc_int32    (*create_ccache) (
                                cc_context_t context,
                                const char* name,
                                cc_int32 cred_vers,
                                const char* principal, 
                                cc_ccache_t* ccache);
    cc_int32    (*create_default_ccache) (
                                cc_context_t context,
                                cc_int32 cred_vers,
                                const char* principal, 
                                cc_ccache_t* ccache);
    cc_int32    (*create_new_ccache) (
                                cc_context_t context,
                                cc_int32 cred_vers,
                                const char* principal, 
                                cc_ccache_t* ccache);
    cc_int32    (*new_ccache_iterator) (
                                cc_context_t context,
                                cc_ccache_iterator_t* iterator);
    cc_int32    (*lock) (
                                cc_context_t context,
                                cc_uint32 lock_type,
                                cc_lock_t* lock);
};

struct cc_ccache_f {
    cc_int32    (*release) (
                                 cc_ccache_t ccache);
    cc_int32    (*destroy) (
                                 cc_ccache_t ccache);
    cc_int32    (*set_default) (
                                 cc_ccache_t ccache);
    cc_int32    (*get_credentials_version) (
                                 cc_ccache_t ccache,
                                 cc_int32* credentials_version);
    cc_int32    (*get_name) (
                                 cc_ccache_t ccache,
                                 cc_string_t* name);
    cc_int32    (*get_principal) (
                                 cc_ccache_t ccache,
                                 cc_uint32 credentials_version,
                                 cc_string_t* principal);
    cc_int32    (*set_principal) (
                                 cc_ccache_t ccache,
                                 cc_uint32 credentials_version,
                                 const char* principal);
    cc_int32    (*store_credentials) (
                                 cc_ccache_t ccache,
                                 const cc_credentials_union* credentials);
    cc_int32    (*remove_credentials) (
                                 cc_ccache_t ccache,
                                 cc_credentials_t credentials);
    cc_int32    (*new_credentials_iterator) (
                                 cc_ccache_t ccache,
                                 cc_credentials_iterator_t* iterator);
    cc_int32    (*move) (
                                 cc_ccache_t source,
                                 cc_ccache_t destination);
    cc_int32    (*lock) (
                                 cc_ccache_t ccache,
                                 cc_uint32 lock_type,
                                 cc_lock_t* lock);
    cc_int32    (*get_last_default_time) (
                                 cc_ccache_t ccache,
                                 cc_time_t* time);
    cc_int32    (*get_change_time) (
                                 cc_ccache_t ccache,
                                 cc_time_t* time);
};

struct cc_string_f {
	cc_int32	(*release) (
								cc_string_t string);
};

struct cc_credentials_f {
	cc_int32	(*release) (
								cc_credentials_t credentials);
};

			   
struct cc_ccache_iterator_f {
    cc_int32    (*release) (
                                 cc_ccache_iterator_t iter);
    cc_int32    (*next) (
                                 cc_ccache_iterator_t iter,
                                 cc_ccache_t* ccache);
};

struct cc_credentials_iterator_f {
    cc_int32    (*release) (
                                 cc_credentials_iterator_t iter);
    cc_int32    (*next) (
                                 cc_credentials_iterator_t iter,
                                 cc_credentials_t* ccache);
};

struct cc_lock_f {
    cc_int32    (*release) (
                                 cc_lock_t lock);
};

/*
 * API functions
 */
 
cc_int32 cc_initialize (
	cc_context_t*		outContext,
	cc_int32			inVersion,
	cc_int32*			outSupportedVersion,
	char const**		outVendor);
	
/*
 * Convenience macros
 */
 
#define		cc_context_release(context)														\
			((context) -> functions -> release (context))
#define		cc_context_get_version(context, version)										\
			((context) -> functions -> get_version (context, version))
#define		cc_context_clone(context, new_context, api_version, supported_version, vendor)	\
			((context) -> functions -> close (context, new_context, api_version, supported_version, vendor))
#define		cc_context_get_change_time(context, time)										\
			((context) -> functions -> get_change_time (context, time))
#define		cc_context_get_default_ccache_name(context, name)								\
			((context) -> functions -> get_default_ccache_name (context, name))
#define		cc_context_open_ccache(context, name, ccache)									\
			((context) -> functions -> open_ccache (context, name, ccache))
#define		cc_context_open_default_ccache(context, ccache)									\
			((context) -> functions -> open_default_ccache (context, ccache))
#define		cc_context_create_ccache(context, name, version, principal, ccache)				\
			((context) -> functions -> create_ccache (context, name, version, principal, ccache))
#define		cc_context_create_default_ccache(context, version, principal, ccache)				\
			((context) -> functions -> create_default_ccache (context, version, principal, ccache))
#define		cc_context_create_new_ccache(context, version, principal, ccache)				\
			((context) -> functions -> create_new_ccache (context, version, principal, ccache))
#define		cc_context_new_ccache_iterator(context, iterator)								\
			((context) -> functions -> new_ccache_iterator (context, iterator))
#define		cc_context_lock(context, type, lock)											\
			((context) -> functions -> lock (context, type, lock))

#define		cc_ccache_release(ccache)														\
			((ccache) -> functions -> release (ccache))
#define		cc_ccache_destroy(ccache)														\
			((ccache) -> functions -> destroy (ccache))
#define		cc_ccache_set_default(ccache)													\
			((ccache) -> functions -> set_default (ccache))
#define		cc_ccache_get_credentials_version(ccache, version)								\
			((ccache) -> functions -> get_credentials_version (ccache, version))
#define		cc_ccache_get_name(ccache, name)												\
			((ccache) -> functions -> get_name (ccache, name))
#define		cc_ccache_get_principal(ccache, version, principal)								\
			((ccache) -> functions -> get_principal (ccache, version, principal))
#define		cc_ccache_set_principal(ccache, version, principal)								\
			((ccache) -> functions -> set_principal (ccache, version, principal))
#define		cc_ccache_store_credentials(ccache, credentials)								\
			((ccache) -> functions -> store_credentials (ccache, credentials))
#define		cc_ccache_remove_credentials(ccache, credentials)								\
			((ccache) -> functions -> remove_credentials (ccache, credentials))
#define		cc_ccache_new_credentials_iterator(ccache, iterator)							\
			((ccache) -> functions -> new_credentials_iterator (ccache, iterator))
#define		cc_ccache_lock(ccache, lock)													\
			((ccache) -> functions -> lock (ccache, lock))
#define		cc_ccache_get_last_default_time(ccache, time)									\
			((ccache) -> functions -> get_last_default_time (ccache, time))
#define		cc_ccache_get_change_time(ccache, time)											\
			((ccache) -> functions -> get_change_time (ccache, time))
#define		cc_ccache_move(source, destination)												\
			((source) -> functions -> move (source, destination))

#define		cc_string_release(string)														\
			((string) -> functions -> release (string))

#define		cc_credentials_release(credentials)												\
			((credentials) -> functions -> release (credentials))

#define		cc_ccache_iterator_release(iterator)											\
			((iterator) -> functions -> release (iterator))
#define		cc_ccache_iterator_next(iterator, ccache)										\
			((iterator) -> functions -> next (iterator, ccache))
	
#define		cc_credentials_iterator_release(iterator)										\
			((iterator) -> functions -> release (iterator))
#define		cc_credentials_iterator_next(iterator, credentials)								\
			((iterator) -> functions -> next (iterator, credentials))
			
#define		cc_lock_release(lock)															\
			((lock) -> functions -> release (lock))

#if PRAGMA_STRUCT_ALIGN
	#pragma options align=reset
#elif PRAGMA_STRUCT_PACKPUSH
	#pragma pack(pop)
#elif PRAGMA_STRUCT_PACK
	#pragma pack()
#endif

#if PRAGMA_ENUM_ALWAYSINT
	#pragma enumsalwaysint reset
#endif

#if TARGET_CPU_68K
	#pragma fourbyteints reset
#endif 

#if PRAGMA_IMPORT
#	pragma import reset
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CCache_h */
