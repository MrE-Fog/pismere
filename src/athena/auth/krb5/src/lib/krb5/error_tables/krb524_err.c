/*
 * lib/krb5/error_tables/krb524_err.c:
 * This file is automatically generated; please do not edit it.
 */
#if defined(_WIN32)
# include "win-mac.h"
#endif

#if !defined(_WIN32)
extern void initialize_k524_error_table (void);
#endif

/* Lclint doesn't handle null annotations on arrays
   properly, so we need this typedef in each
   generated .c file.  */
/*@-redef@*/
typedef /*@null@*/ const char *ncptr;
/*@=redef@*/

static ncptr const text[] = {
	"Cannot convert V5 keyblock",
	"Cannot convert V5 address information",
	"Cannot convert V5 principal",
	"V5 realm name longer than V4 maximum",
	"Kerberos V4 error",
	"Encoding too large",
	"Decoding out of data",
	"Service not responding",
	"Kerberos version 4 support is disabled",
    0
};

#include <com_err.h>

const struct error_table et_k524_error_table = { text, -1750206208L, 9 };

#if !defined(_WIN32)
void initialize_k524_error_table (void)
    /*@modifies internalState@*/
{
    (void) add_error_table (&et_k524_error_table);
}
#endif