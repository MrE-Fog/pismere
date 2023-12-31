/*
 * $Source: /cvs/pismere/pismere/athena/auth/krb4/krbv4/krbdll/get_req.c,v $
 * $Author: dalmeida $
 *
 * Copyright 1985, 1986, 1987, 1988 by the Massachusetts Institute
 * of Technology.
 *
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 */

#ifndef lint
static char *rcsid_get_request_c =
"$Header: /cvs/pismere/pismere/athena/auth/krb4/krbv4/krbdll/get_req.c,v 1.1 1999/03/12 23:05:37 dalmeida Exp $";
#endif /* lint */

#ifdef WINDOWS
        #include <windows.h>
#endif

#include <mit_copy.h>
#include <krb.h>
#include <prot.h>

/*
 * This procedure is obsolete.  It is used in the kerberos_slave
 * code for Version 3 tickets.
 *
 * This procedure sets s_name, and instance to point to
 * the corresponding fields from tne nth request in the packet.
 * it returns the lifetime requested.  Garbage will be returned
 * if there are less than n requests in the packet.
 */

get_request(pkt, n, s_name, instance)
    KTEXT pkt;                  /* The packet itself */
    int n;                      /* Which request do we want */
    char **s_name;              /* Service name to be filled in */
    char **instance;            /* Instance name to be filled in */
{
    /* Go to the beginning of the request list */
    char *ptr = (char *) pkt_a_realm(pkt) + 6 +
        strlen((char *)pkt_a_realm(pkt));

    /* Read requests until we hit the right one */
    while (n-- > 1) {
        ptr++;
        ptr += 1 + strlen(ptr);
        ptr += 1 + strlen(ptr);
    }

    /* Set the arguments to point to the right place */
    *s_name = 1 + ptr;
    *instance = 2 + ptr + strlen(*s_name);

    /* Return the requested lifetime */
    return((int) *ptr);
}
