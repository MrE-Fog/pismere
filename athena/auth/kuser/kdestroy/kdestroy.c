/*
 * clients/kdestroy/kdestroy.c
 *
 * Copyright 1990 by the Massachusetts Institute of Technology.
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
 * permission.  M.I.T. makes no representations about the suitability of
 * this software for any purpose.  It is provided "as is" without express
 * or implied warranty.
 * 
 *
 * Destroy the contents of your credential cache.
 */

#include <krb5.h>
#include <krb.h>
#include <com_err.h>
#include <string.h>
#include <stdio.h>

#include <getopt.h>
#include <kuser.h>

#define com_err pcom_err

void
main(argc, argv)
    int argc;
    char **argv;
{
    krb5_context kcontext;
    krb5_error_code retval;
    int c;
    krb5_ccache cache = NULL;
    char *cache_name = NULL;
    int code = 0;
    int v4code = 0;
    int errflg=0;
    int quiet = 0;	
    int v4 = 1;

    int got_k4 = 0;
    int got_k5 = 0; 

    int use_k4_only = 0;
    int use_k5_only = 0;

    fix_progname(argv);

    if (!LoadFuncs(COMERR_DLL, ce_fi, 0, 0, 1, 0, 1))
        pcom_err = fake_com_err;
    got_k5 = LoadFuncs(KRB5_DLL, k5_fi, 0, 0, 1, 0, 1);
    got_k4 = LoadFuncs(KRB4_DLL, k4_fi, 0, 0, 1, 0, 1);

    while ((c = getopt(argc, argv, "45qc:")) != -1) {
	switch (c) {
	case 'q':
	    quiet = 1;
	    break;	
	case 'c':
            if (cache_name)
            {
		fprintf(stderr, "Only one -c option allowed\n");
		errflg++;
            }
            cache_name = optarg;
            break;
        case '4':
            use_k4_only = 1;
            break;
        case '5':
            use_k5_only = 1;
            break;
	case '?':
	default:
	    errflg++;
	    break;
	}
    }

    if (use_k4_only && use_k5_only)
    {
        fprintf(stderr, "Only one of -4 and -5 allowed\n");
        errflg++;
    }

    if (optind != argc)
	errflg++;
    
    if (errflg) {
	fprintf(stderr, "Usage: %s [-4] [-5] [-q] [ -c cache-name ]\n", 
                progname);
	exit(2);
    }

    if (use_k4_only)
        got_k5 = 0;
    if (use_k5_only)
        got_k4 = 0;

    if (got_k5)
    {
        retval = pkrb5_init_context(&kcontext);
        if (retval) {
	    com_err(progname, retval, "while initializing krb5");
	    exit(1);
        }
        if (cache_name)
        {
            v4 = 0;	/* Don't do krb4 kdestroy if cache name given. */
            code = pkrb5_cc_resolve(kcontext, cache_name, &cache);
            if (code != 0) {
                com_err(progname, code, "while resolving %s", cache_name);
                exit(1);
            }
        } else {
            if (code = pkrb5_cc_default(kcontext, &cache)) {
                com_err(progname, code, "while getting default ccache");
                exit(1);
            }
        }
        code = krb5_cc_destroy (kcontext, cache);
        if (code != 0) {
            com_err(progname, code, "while destroying cache");
            if (code != KRB5_FCC_NOFILE) {
                if (quiet)
                    fprintf(stderr, "Ticket cache NOT destroyed!\n");
                else {
                    fprintf(stderr, "Ticket cache \aNOT\a destroyed!\n");
                }
                errflg = 1;
            }
        }
    }

    if (got_k4 && v4)
    {
        v4code = pdest_tkt();
        if (v4code == KSUCCESS && code != 0)
            fprintf(stderr, "Kerberos 4 ticket cache destroyed.\n");
        if (v4code != KSUCCESS && v4code != RET_TKFIL) {
            if (quiet)
                fprintf(stderr, "Kerberos 4 ticket cache NOT destroyed!\n");
            else
                fprintf(stderr,
                        "Kerberos 4 ticket cache \aNOT\a destroyed!\n");
            errflg = 1;
        }
    }

    exit (errflg);
}
