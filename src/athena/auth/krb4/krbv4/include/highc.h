/*
 * $Source: /cvs/pismere/pismere/athena/auth/krb4/krbv4/include/highc.h,v $
 * $Author: dalmeida $
 * $Header: /cvs/pismere/pismere/athena/auth/krb4/krbv4/include/highc.h,v 1.1 1999/03/12 23:05:20 dalmeida Exp $
 *
 * Copyright 1988 by the Massachusetts Institute of Technology.
 *
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 *
 * Known breakage in the version of Metaware's High C compiler that
 * we've got available....
 */

#include <mit-copyright.h>

#define const
/*#define volatile*/

/*
 * Some builtin functions we can take advantage of for inlining....
 */

#define abs			_abs
/* the _max and _min builtins accept any number of arguments */
#undef MAX
#define MAX(x,y)		_max(x,y)
#undef MIN
#define MIN(x,y)		_min(x,y)
/*
 * I'm not sure if 65535 is a limit for this builtin, but it's
 * reasonable for a string length.  Or is it?
 */
/*#define strlen(s)		_find_char(s,65535,0)*/
#define bzero(ptr,len)		_fill_char(ptr,len,'\0')
#define bcmp(b1,b2,len)		_compare(b1,b2,len)
