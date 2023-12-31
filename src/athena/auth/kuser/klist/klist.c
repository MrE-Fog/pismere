/*
 * clients/klist/klist.c
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
 * permission.  Furthermore if you modify this software you must label
 * your software as modified software and not distribute it in such a
 * fashion that it might be confused with the original M.I.T. software.
 * M.I.T. makes no representations about the suitability of
 * this software for any purpose.  It is provided "as is" without express
 * or implied warranty.
 * 
 *
 * List out the contents of your credential cache or keytab.
 */

#include <krb5.h>
#ifdef KRB5_KRB4_COMPAT
#include <krb.h>
#endif
#include <com_err.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#if defined(HAVE_ARPA_INET_H)
#include <arpa/inet.h>
#endif

#include <getopt.h>
#include <kuser.h>
#include <kuser-redefine.h>

#ifndef _WIN32
#define GET_PROGNAME(x) (strrchr((x), '/') ? strrchr((x), '/')+1 : (x))
#else
#define GET_PROGNAME(x) max(max(strrchr((x), '/'), strrchr((x), '\\')) + 1,(x))
#endif

#if (defined(_MSDOS) || defined(_WIN32))
#include <winsock.h>
#else
#include <sys/socket.h>
#include <netdb.h>
#endif

extern int optind;

int show_flags = 0, show_time = 0, status_only = 0, show_keys = 0;
int show_etype = 0, show_addresses = 0, no_resolve = 0;
int all_ccaches = 0;
char *defname;
char *progname;
krb5_int32 now;
int timestamp_width;

krb5_context kcontext;

char * etype_string (krb5_enctype );
void show_credential (char *, krb5_context, krb5_creds *);
     
#ifdef CCAPIV2_COMPAT
void do_all_ccache (void);
#endif
void do_ccache (char *);
void do_keytab (char *);
void printtime (time_t);
void one_addr (krb5_address *);
void fillit (FILE *, int, int);

#ifdef KRB5_KRB4_COMPAT
void do_v4_ccache (char *);
#endif /* KRB5_KRB4_COMPAT */

#define DEFAULT 0
#define CCACHE 1
#define KEYTAB 2

/*
 * The reason we start out with got_k4 and got_k5 as zero (false) is
 * so that we can easily add dynamic loading support for determining
 * whether Kerberos 4 and Keberos 5 libraries are available
 */

static int got_k5 = 0; 
static int got_k4 = 0;          
static int got_cc = 0;

static int default_k5 = 1;
#if defined(KRB5_KRB4_COMPAT) && defined(KLIST_DEFAULT_BOTH)
static int default_k4 = 1;
#else
static int default_k4 = 0;
#endif

static int exit_status = 0;

static void 
extended_com_err_fn (const char *myprog, errcode_t code,
                     const char *fmt, va_list args)
{
    const char *emsg;
    emsg = krb5_get_error_message (kcontext, code);
    fprintf (stderr, "%s: %s ", myprog, emsg);
    krb5_free_error_message (kcontext, emsg);
    vfprintf (stderr, fmt, args);
    fprintf (stderr, "\n");
}


void usage()
{
#define KRB_AVAIL_STRING(x) ((x)?"available":"not available")

    fprintf(stderr, "Usage: %s [-5] [-4] [-e] [[-c] [-f] [-s] [-a [-n]] [-u]] "
	     "[-k [-t] [-K]] [name]\n", progname); 
    fprintf(stderr, "\t-5 Kerberos 5 (%s)\n", KRB_AVAIL_STRING(got_k5));
    fprintf(stderr, "\t-4 Kerberos 4 (%s)\n", KRB_AVAIL_STRING(got_k4));
    fprintf(stderr, "\t   (Default is %s%s%s%s)\n",
	    default_k5?"Kerberos 5":"",
	    (default_k5 && default_k4)?" and ":"",
	    default_k4?"Kerberos 4":"",
	    (!default_k5 && !default_k4)?"neither":"");
    fprintf(stderr, "\t-c specifies credentials cache\n");
    fprintf(stderr, "\t-C specifies and enumerates all credentials caches\n");
    fprintf(stderr, "\t-k specifies keytab\n");
    fprintf(stderr, "\t   (Default is credentials cache)\n");
    fprintf(stderr, "\t-e shows the encryption type\n");
    fprintf(stderr, "\toptions for credential caches:\n");
    fprintf(stderr, "\t\t-f shows credentials flags\n");
    fprintf(stderr, "\t\t-s sets exit status based on valid tgt existence\n");
    fprintf(stderr, "\t\t-a displays the address list\n");
    fprintf(stderr, "\t\t\t-n do not reverse-resolve\n");
    fprintf(stderr, "\toptions for keytabs:\n");
    fprintf(stderr, "\t\t-t shows keytab entry timestamps\n");
    fprintf(stderr, "\t\t-K shows keytab entry DES keys\n");
    exit(1);
}

int
main(
    int argc,
    char **argv)
{
    int c;
    char *name;
    int mode;
    int use_k5 = 0, use_k4 = 0;

    dynamic_load(&got_k4, &got_k5, &got_cc);

    progname = GET_PROGNAME(argv[0]);

    name = NULL;
    mode = DEFAULT;
    while ((c = getopt(argc, argv, "fetKsnack45C")) != -1) {
	switch (c) {
	case 'f':
	    show_flags = 1;
	    break;
	case 'e':
	    show_etype = 1;
	    break;
	case 't':
	    show_time = 1;
	    break;
	case 'K':
	    show_keys = 1;
	    break;
	case 's':
	    status_only = 1;
	    break;
	case 'n':
	    no_resolve = 1;
	    break;
	case 'a':
	    show_addresses = 1;
	    break;
    case 'C':
        all_ccaches = 1;
        /* fall-through */
	case 'c':
	    if (mode != DEFAULT) usage();
	    mode = CCACHE;
	    break;
	case 'k':
	    if (mode != DEFAULT) usage();
	    mode = KEYTAB;
	    break;
	case '4':
	    if (!got_k4)
	    {
#ifdef KRB5_KRB4_COMPAT
		fprintf(stderr, "Kerberos 4 support could not be loaded\n");
#else
		fprintf(stderr, "This was not built with Kerberos 4 support\n");
#endif
		exit(3);
	    }
	    use_k4 = 1;
	    break;
	case '5':
	    if (!got_k5)
	    {
		fprintf(stderr, "Kerberos 5 support could not be loaded\n");
		exit(3);
	    }
	    use_k5 = 1;
	    break;
	default:
	    usage();
	    break;
	}
    }

    if (no_resolve && !show_addresses) {
        usage();
    }

    if (mode == DEFAULT || mode == CCACHE) {
        if (show_time || show_keys)
            usage();
    } else {
        if (show_flags || status_only || show_addresses)
            usage();
    }

    if (argc - optind > 1) {
        fprintf(stderr, "Extra arguments (starting with \"%s\").\n",
                 argv[optind+1]);
        usage();
    }

    name = (optind == argc-1) ? argv[optind] : 0;

    if (!use_k5 && !use_k4)
    {
        use_k5 = default_k5;
        use_k4 = default_k4;
    }

    if (!use_k5)
        got_k5 = 0;
    if (!use_k4)
        got_k4 = 0;

    now = time(0);
    {
	char tmp[BUFSIZ];

	if (!krb5_timestamp_to_sfstring(now, tmp, 20, (char *) NULL) ||
	    !krb5_timestamp_to_sfstring(now, tmp, sizeof(tmp), 
					(char *) NULL))
	    timestamp_width = (int) strlen(tmp);
	else
	    timestamp_width = 15;
    }

    if (got_k5)
    {
        krb5_error_code retval;
        retval = krb5_init_context(&kcontext);
        if (retval) {
            com_err(progname, retval, "while initializing krb5");
            exit(1);
        }
        set_com_err_hook (extended_com_err_fn);

        if (mode == DEFAULT || mode == CCACHE) { 
#ifdef CCAPIV2_COMPAT
            if ( got_cc && all_ccaches && !name ) {
                do_all_ccache();
            } else 
#endif
            {
                do_ccache(name);
            }
        } else {
            do_keytab(name);
        }
    } else {
#ifdef KRB5_KRB4_COMPAT
        if (mode == DEFAULT || mode == CCACHE)
            do_v4_ccache(name);
        else {
            /* We may want to add v4 srvtab support */
            fprintf(stderr, 
                     "%s: srvtab option not supported for Kerberos 4\n", 
                     progname);
            exit(1);
        }
#endif /* KRB4_KRB5_COMPAT */
    }

    return 0;
}    

void do_keytab(
   char *name)
{
     krb5_keytab kt;
     krb5_keytab_entry entry;
     krb5_kt_cursor cursor;
     char buf[BUFSIZ]; /* hopefully large enough for any type */
     char *pname;
     int code;
     
     if (name == NULL) {
	  if ((code = krb5_kt_default(kcontext, &kt))) {
	       com_err(progname, code, "while getting default keytab");
	       exit(1);
	  }
     } else {
	  if ((code = krb5_kt_resolve(kcontext, name, &kt))) {
	       com_err(progname, code, "while resolving keytab %s",
		       name);
	       exit(1);
	  }
     }

     if ((code = krb5_kt_get_name(kcontext, kt, buf, BUFSIZ))) {
	  com_err(progname, code, "while getting keytab name");
	  exit(1);
     }

     printf("Keytab name: %s\n", buf);
     
     if ((code = krb5_kt_start_seq_get(kcontext, kt, &cursor))) {
	  com_err(progname, code, "while starting keytab scan");
	  exit(1);
     }

     if (show_time) {
	  printf("KVNO Timestamp");
	  fillit(stdout, timestamp_width - sizeof("Timestamp") + 2, (int) ' ');
	  printf("Principal\n");
	  printf("---- ");
	  fillit(stdout, timestamp_width, (int) '-');
	  printf(" ");
	  fillit(stdout, 78 - timestamp_width - sizeof("KVNO"), (int) '-');
	  printf("\n");
     } else {
	  printf("KVNO Principal\n");
	  printf("---- --------------------------------------------------------------------------\n");
     }
     
     while ((code = krb5_kt_next_entry(kcontext, kt, &entry, &cursor)) == 0) {
	  if ((code = krb5_unparse_name(kcontext, entry.principal, &pname))) {
	       com_err(progname, code, "while unparsing principal name");
	       exit(1);
	  }
	  printf("%4d ", entry.vno);
	  if (show_time) {
	       printtime(entry.timestamp);
	       printf(" ");
	  }
	  printf("%s", pname);
	  if (show_etype)
	      printf(" (%s) " , etype_string(entry.key.enctype));
	  if (show_keys) {
	       printf(" (0x");
	       {
		    unsigned int i;
		    for (i = 0; i < entry.key.length; i++)
			 printf("%02x", entry.key.contents[i]);
	       }
	       printf(")");
	  }
	  printf("\n");
	  krb5_free_unparsed_name(kcontext, pname);
     }
     if (code && code != KRB5_KT_END) {
	  com_err(progname, code, "while scanning keytab");
	  exit(1);
     }
     if ((code = krb5_kt_end_seq_get(kcontext, kt, &cursor))) {
	  com_err(progname, code, "while ending keytab scan");
	  exit(1);
     }
     exit(0);
}

#ifdef CCAPIV2_COMPAT
void do_all_ccache()
{
    krb5_error_code retval;
    apiCB * cc_ctx = 0;
    struct _infoNC ** pNCi = 0;
    int i;

    retval = cc_initialize(&cc_ctx, CC_API_VER_2, NULL, NULL);
    if (retval) {
        com_err(progname, retval, "while initializing ccapiv2");
        exit(1);
    }

    retval = cc_get_NC_info(cc_ctx, &pNCi);
    if (retval) {
        com_err(progname, retval, "while obtaining named cache info");
        exit(1);
    }

    for ( i=0; pNCi[i]; i++ ) {
        switch (pNCi[i]->vers) {
        case CC_CRED_V5:
            if (got_k5)
                do_ccache(pNCi[i]->name);
            break;
#ifdef KRB5_KRB4_COMPAT
        case CC_CRED_V4:
            if (got_k4)
                do_v4_ccache(pNCi[i]->name);
            break;
#endif /* KRB5_KRB4_COMPAT */
        }
        printf("\n\n");
    }

    if (pNCi)
        cc_free_NC_info(cc_ctx, &pNCi);
    if (cc_ctx)
        cc_shutdown(&cc_ctx);
    exit(exit_status);
}
#endif

void do_ccache(char* name)
{
    krb5_ccache cache = NULL;
    krb5_cc_cursor cur;
    krb5_creds creds;
    krb5_principal princ;
    krb5_flags flags = 0;
    krb5_error_code code;
	    
    if (status_only)
	/* exit_status is set back to 0 if a valid tgt is found */
	exit_status = 1;

    if (name == NULL) {
	if ((code = krb5_cc_default(kcontext, &cache))) {
	    if (!status_only)
		com_err(progname, code, "while getting default ccache");
	    exit(1);
	    }
    } else {
	if ((code = krb5_cc_resolve(kcontext, name, &cache))) {
	    if (!status_only)
		com_err(progname, code, "while resolving ccache %s",
			name);
	    exit(1);
	}
    }
 
#ifdef KRB5_TC_NOTICKET
    /* turn off OPENCLOSE mode and turn on NOTICKET mode */
    flags = show_etype ? 0 : KRB5_TC_NOTICKET;
#endif
    if ((code = krb5_cc_set_flags(kcontext, cache, flags))) {
	if (code == KRB5_FCC_NOFILE) {
	    if (!status_only) {
		com_err(progname, code, "(ticket cache %s:%s)",
			krb5_cc_get_type(kcontext, cache),
			krb5_cc_get_name(kcontext, cache));
#ifdef KRB5_KRB4_COMPAT
		if (name == NULL)
		    do_v4_ccache(0);
#endif
	    }
	} else {
	    if (!status_only)
		com_err(progname, code,
			"while setting cache flags (ticket cache %s:%s)",
			krb5_cc_get_type(kcontext, cache),
			krb5_cc_get_name(kcontext, cache));
	}
	exit(1);
    }
    if ((code = krb5_cc_get_principal(kcontext, cache, &princ))) {
	if (!status_only)
	    com_err(progname, code, "while retrieving principal name");
	exit(1);
    }
    if ((code = krb5_unparse_name(kcontext, princ, &defname))) {
	if (!status_only)
	    com_err(progname, code, "while unparsing principal name");
	exit(1);
    }
    if (!status_only) {
	printf("Ticket cache: %s:%s\nDefault principal: %s\n\n",
	       krb5_cc_get_type(kcontext, cache),
	       krb5_cc_get_name(kcontext, cache), defname);
	fputs("Valid starting", stdout);
	fillit(stdout, timestamp_width - sizeof("Valid starting") + 3,
	       (int) ' ');
	fputs("Expires", stdout);
	fillit(stdout, timestamp_width - sizeof("Expires") + 3,
	       (int) ' ');
	fputs("Service principal\n", stdout);
    }
    if ((code = krb5_cc_start_seq_get(kcontext, cache, &cur))) {
	if (!status_only)
	    com_err(progname, code, "while starting to retrieve tickets");
	exit(1);
    }
    while (!(code = krb5_cc_next_cred(kcontext, cache, &cur, &creds))) {
	if (status_only) {
	    if (exit_status && creds.server->length == 2 &&
		strcmp(creds.server->realm.data, princ->realm.data) == 0 &&
		strcmp((char *)creds.server->data[0].data, "krbtgt") == 0 &&
		strcmp((char *)creds.server->data[1].data,
		       princ->realm.data) == 0 && 
		creds.times.endtime > now)
		exit_status = 0;
	} else {
	    show_credential(progname, kcontext, &creds);
	}
	krb5_free_cred_contents(kcontext, &creds);
    }
    if (code == KRB5_CC_END) {
	if ((code = krb5_cc_end_seq_get(kcontext, cache, &cur))) {
	    if (!status_only)
		com_err(progname, code, "while finishing ticket retrieval");
	    exit(1);
	}
	flags = KRB5_TC_OPENCLOSE;	/* turns on OPENCLOSE mode */
	if ((code = krb5_cc_set_flags(kcontext, cache, flags))) {
	    if (!status_only)
		com_err(progname, code, "while closing ccache");
	    exit(1);
	}
    if (!(got_cc && all_ccaches)) {
#ifdef KRB5_KRB4_COMPAT
        if (name == NULL && !status_only)
            do_v4_ccache(0);
#endif
    	exit(exit_status);
    }
    } else {
	if (!status_only)
	    com_err(progname, code, "while retrieving a ticket");
	exit(1);
    }	
}

char *
etype_string(
    krb5_enctype enctype)
{
    static char buf[100];
    krb5_error_code retval;
    
    if ((retval = krb5_enctype_to_string(enctype, buf, sizeof(buf)))) {
	/* XXX if there's an error != EINVAL, I should probably report it */
	sprintf(buf, "etype %d", enctype);
    }

    return buf;
}

char *
flags_string(
    register krb5_creds *cred)
{
    static char buf[32];
    int i = 0;
	
    if (cred->ticket_flags & TKT_FLG_FORWARDABLE)
	buf[i++] = 'F';
    if (cred->ticket_flags & TKT_FLG_FORWARDED)
	buf[i++] = 'f';
    if (cred->ticket_flags & TKT_FLG_PROXIABLE)
	buf[i++] = 'P';
    if (cred->ticket_flags & TKT_FLG_PROXY)
	buf[i++] = 'p';
    if (cred->ticket_flags & TKT_FLG_MAY_POSTDATE)
	buf[i++] = 'D';
    if (cred->ticket_flags & TKT_FLG_POSTDATED)
	buf[i++] = 'd';
    if (cred->ticket_flags & TKT_FLG_INVALID)
	buf[i++] = 'i';
    if (cred->ticket_flags & TKT_FLG_RENEWABLE)
	buf[i++] = 'R';
    if (cred->ticket_flags & TKT_FLG_INITIAL)
	buf[i++] = 'I';
    if (cred->ticket_flags & TKT_FLG_HW_AUTH)
	buf[i++] = 'H';
    if (cred->ticket_flags & TKT_FLG_PRE_AUTH)
	buf[i++] = 'A';
    buf[i] = '\0';
    return(buf);
}

void 
printtime(
    time_t tv)
{
    char timestring[BUFSIZ];
    char fill;

    fill = ' ';
    if (!krb5_timestamp_to_sfstring((krb5_timestamp) tv,
				    timestring,
				    timestamp_width+1,
				    &fill)) {
	printf(timestring);
    }
}

void
show_credential(
    char 		* progname,
    krb5_context  	  kcontext,
    register krb5_creds * cred)
{
    krb5_error_code retval;
    krb5_ticket *tkt;
    char *name, *sname, *flags;
    int	extra_field = 0;

    retval = krb5_unparse_name(kcontext, cred->client, &name);
    if (retval) {
	com_err(progname, retval, "while unparsing client name");
	return;
    }
    retval = krb5_unparse_name(kcontext, cred->server, &sname);
    if (retval) {
	com_err(progname, retval, "while unparsing server name");
	krb5_free_unparsed_name(kcontext, name);
	return;
    }
    if (!cred->times.starttime)
	cred->times.starttime = cred->times.authtime;

    printtime(cred->times.starttime);
    putchar(' '); putchar(' ');
    printtime(cred->times.endtime);
    putchar(' '); putchar(' ');

    printf("%s\n", sname);

    if (strcmp(name, defname)) {
	    printf("\tfor client %s", name);
	    extra_field++;
    }
    
    if (cred->times.renew_till) {
	if (!extra_field)
		fputs("\t",stdout);
	else
		fputs(", ",stdout);
	fputs("renew until ", stdout);
	printtime(cred->times.renew_till);
	extra_field += 2;
    }

    if (extra_field > 3) {
	fputs("\n", stdout);
	extra_field = 0;
    }

    if (show_flags) {
	flags = flags_string(cred);
	if (flags && *flags) {
	    if (!extra_field)
		fputs("\t",stdout);
	    else
		fputs(", ",stdout);
	    printf("Flags: %s", flags);
	    extra_field++;
	}
    }

    if (extra_field > 2) {
	fputs("\n", stdout);
	extra_field = 0;
    }

    if (show_etype) {
	retval = krb5_decode_ticket(&cred->ticket, &tkt);
	if (!extra_field)
	    fputs("\t",stdout);
	else
	    fputs(", ",stdout);
	printf("Etype (skey, tkt): %s, ",
	       etype_string(cred->keyblock.enctype));
	printf("%s ",
	       etype_string(tkt->enc_part.enctype));
	krb5_free_ticket(kcontext, tkt);
	extra_field++;
    }

    /* if any additional info was printed, extra_field is non-zero */
    if (extra_field)
	putchar('\n');


    if (show_addresses) {
	if (!cred->addresses || !cred->addresses[0]) {
	    printf("\tAddresses: (none)\n");
	} else {
	    int i;

	    printf("\tAddresses: ");
	    one_addr(cred->addresses[0]);

	    for (i=1; cred->addresses[i]; i++) {
		printf(", ");
		one_addr(cred->addresses[i]);
	    }

	    printf("\n");
	}
    }

    krb5_free_unparsed_name(kcontext, name);
    krb5_free_unparsed_name(kcontext, sname);
}

void one_addr(
    krb5_address *a)
{
    struct hostent *h;

    if ((a->addrtype == ADDRTYPE_INET && a->length == 4)
#ifdef AF_INET6
	|| (a->addrtype == ADDRTYPE_INET6 && a->length == 16)
#endif
	) {
	int af = AF_INET;
#ifdef AF_INET6
	if (a->addrtype == ADDRTYPE_INET6)
	    af = AF_INET6;
#endif
	if (!no_resolve) {
#ifdef HAVE_GETIPNODEBYADDR
	    int err;
	    h = getipnodebyaddr(a->contents, a->length, af, &err);
	    if (h) {
		printf("%s", h->h_name);
		freehostent(h);
	    }
#else
	    h = gethostbyaddr(a->contents, a->length, af);
	    if (h) {
		printf("%s", h->h_name);
	    }
#endif
	    if (h)
		return;
	}
	if (no_resolve || !h) {
#ifdef HAVE_INET_NTOP
	    char buf[46];
	    const char *name = inet_ntop(a->addrtype, a->contents, buf, sizeof(buf));
	    if (name) {
		printf ("%s", name);
		return;
	    }
#else
	    if (a->addrtype == ADDRTYPE_INET) {
		printf("%d.%d.%d.%d", a->contents[0], a->contents[1],
		       a->contents[2], a->contents[3]);
		return;
	    }
#endif
	}
    }
    printf("unknown addr type %d", a->addrtype);
}

void
fillit(
    FILE	*f,
    int		num,
    int		c)
{
    int i;

    for (i=0; i<num; i++)
	fputc(c, f);
}

#ifdef KRB5_KRB4_COMPAT
void
do_v4_ccache(
    char * name)
{
    char    pname[ANAME_SZ];
    char    pinst[INST_SZ];
    char    prealm[REALM_SZ];
    char    *file;
    int     k_errno;
    CREDENTIALS c;
    int     header = 1;

    if (!got_k4)
	return;

    file = name?name:tkt_string();

    if (status_only) {
	fprintf(stderr, 
		"%s: exit status option not supported for Kerberos 4\n",
		progname);
	exit(1);
    }

    if (got_k5)
	printf("\n\n");

    printf("Kerberos 4 ticket cache: %s\n", file);

    /* 
     * Since krb_get_tf_realm will return a ticket_file error, 
     * we will call tf_init and tf_close first to filter out
     * things like no ticket file.  Otherwise, the error that 
     * the user would see would be 
     * klist: can't find realm of ticket file: No ticket file (tf_util)
     * instead of
     * klist: No ticket file (tf_util)
     */

    /* Open ticket file */
    if (k_errno = tf_init(file, R_TKT_FIL)) {
	fprintf(stderr, "%s: %s\n", progname, krb_get_err_text (k_errno));
	exit(1);
    }
    /* Close ticket file */
    (void) tf_close();

    /* 
     * We must find the realm of the ticket file here before calling
     * tf_init because since the realm of the ticket file is not
     * really stored in the principal section of the file, the
     * routine we use must itself call tf_init and tf_close.
     */
    if ((k_errno = krb_get_tf_realm(file, prealm)) != KSUCCESS) {
	fprintf(stderr, "%s: can't find realm of ticket file: %s\n",
		progname, krb_get_err_text (k_errno));
	exit(1);
    }

    /* Open ticket file */
    if (k_errno = tf_init(file, R_TKT_FIL)) {
	fprintf(stderr, "%s: %s\n", progname, krb_get_err_text (k_errno));
	exit(1);
    }
    /* Get principal name and instance */
    if ((k_errno = tf_get_pname(pname)) ||
	(k_errno = tf_get_pinst(pinst))) {
	fprintf(stderr, "%s: %s\n", progname, krb_get_err_text (k_errno));
	exit(1);
    }

    /* 
     * You may think that this is the obvious place to get the
     * realm of the ticket file, but it can't be done here as the
     * routine to do this must open the ticket file.  This is why 
     * it was done before tf_init.
     */
       
    printf("Principal: %s%s%s%s%s\n\n", pname,
	   (pinst[0] ? "." : ""), pinst,
	   (prealm[0] ? "@" : ""), prealm);
    while ((k_errno = tf_get_cred(&c)) == KSUCCESS) {
	if (header) {
	    printf("%-18s  %-18s  %s\n",
		   "  Issued", "  Expires", "  Principal");
	    header = 0;
	}
	printtime(c.issue_date);
	fputs("  ", stdout);
	printtime(c.issue_date + ((unsigned char) c.lifetime) * 5 * 60);
	printf("  %s%s%s%s%s\n",
	       c.service, (c.instance[0] ? "." : ""), c.instance,
	       (c.realm[0] ? "@" : ""), c.realm);
    }
    if (header && k_errno == EOF) {
	printf("No tickets in file.\n");
    }
}
#endif /* KRB4_KRB5_COMPAT */
