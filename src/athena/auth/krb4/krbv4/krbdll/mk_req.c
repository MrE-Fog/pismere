/*
 * Copyright 1985, 1986, 1987, 1988 by the Massachusetts Institute
 * of Technology.
 *
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 */
    
#include <mit_copy.h>
#ifdef WINDOWS
#include <windows.h>
#include <winsock.h>
#endif
#include <krb.h>
#include <prot.h>
#include <des.h>
#include <string.h>
#include <stdio.h>
#ifdef PC
#include <timeval.h>
#endif
    
extern          int     krb_ap_req_debug;
static struct   timeval tv_local = { 0, 0 };
static int lifetime = DEFAULT_TKT_LIFE;
    
#ifdef WINDOWS
#define strcpy lstrcpy
#endif /*WINDOWS*/
    
/*
 * krb_mk_req takes a text structure in which an authenticator is to
 * be built, the name of a service, an instance, a realm,
 * and a checksum.  It then retrieves a ticket for
 * the desired service and creates an authenticator in the text
 * structure passed as the first argument.  krb_mk_req returns
 * KSUCCESS on success and a Kerberos error code on failure.
 *
 * The peer procedure on the other end is krb_rd_req.  When making
 * any changes to this routine it is important to make corresponding
 * changes to krb_rd_req.
 *
 * The authenticator consists of the following:
 *
 * authent->dat
 *
 * unsigned char        KRB_PROT_VERSION        protocol version no.
 * unsigned char        AUTH_MSG_APPL_REQUEST   message type
 * (least significant
 * bit of above)        HOST_BYTE_ORDER         local byte ordering
 * unsigned char        kvno from ticket        server's key version
 * string               realm                   server's realm
 * unsigned char        tl                      ticket length
 * unsigned char        idl                     request id length
 * text                 ticket->dat             ticket for server
 * text                 req_id->dat             request id
 *
 * The ticket information is retrieved from the ticket cache or
 * fetched from Kerberos.  The request id (called the "authenticator"
 #ifdef NOENCRYPTION
 * in the papers on Kerberos) contains the following:
 #else
 * in the papers on Kerberos) contains information encrypted in the session
 * key for the client and ticket-granting service:  {req_id}Kc,tgs
 * Before encryption, it contains the following:
 #endif
 *
 * req_id->dat
 *
 * string               cr.pname                {name, instance, and
 * string               cr.pinst                realm of principal
 * string               myrealm                 making this request}
 * 4 bytes              checksum                checksum argument given
 * unsigned char        tv_local.tf_usec        time (milliseconds)
 * 4 bytes              tv_local.tv_sec         time (seconds)
 *
 * req_id->length = 3 strings + 3 terminating nulls + 5 bytes for time,
 *                  all rounded up to multiple of 8.
 */

int PASCAL
krb_mk_req(authent,service,instance,realm,checksum)
    KTEXT   authent;   /* Place to build the authenticator */
    LPSTR service;           /* Name of the service */
    LPSTR instance;          /* Service instance */
    LPSTR realm;             /* Authentication domain of service */
    long    checksum;           /* Checksum of data (optional) */
{
    static KTEXT_ST req_st; /* Temp storage for req id */
    register KTEXT req_id = &req_st;
    unsigned char *v = authent->dat; /* Prot version number */
    unsigned char *t = (authent->dat+1); /* Message type */
    unsigned char *kv = (authent->dat+2); /* Key version no */
    unsigned char *tl = (authent->dat+4+strlen(realm)); /* Tkt len */
    unsigned char *idl = (authent->dat+5+strlen(realm)); /* Reqid len */
    CREDENTIALS cr;             /* Credentials used by retr */
    
    register KTEXT ticket = &(cr.ticket_st); /* Pointer to tkt_st */
    
    int retval;                 /* Returned by krb_get_cred */
    static Key_schedule  key_s;
    char myrealm[REALM_SZ];
    
    
    /* The fixed parts of the authenticator */
    *v = (unsigned char) KRB_PROT_VERSION;
    *t = (unsigned char) AUTH_MSG_APPL_REQUEST;
    *t |= HOST_BYTE_ORDER;
    
    if (krb_ap_req_debug){
        kdebug_ap(" krb_mk_req: auth, %s, %s, %s, %ld \n",
                  service,instance,realm,checksum);
    }
  
  
    /* Get the ticket and move it into the authenticator */
    if (krb_ap_req_debug)
        kdebug_ap("Realm: %s\n",realm);
    /*
     * Determine realm of these tickets.  We will send this to the
     * KDC from which we are requesting tickets so it knows what to
     * with our session key.
     */
    if ((retval = krb_get_tf_realm(TKT_FILE, myrealm)) != KSUCCESS){
 
  	if (krb_ap_req_debug){
  	    kdebug_ap("problem with krb_get_tf_realm, %ld \n", retval);
  	}
		if (retval == NO_TKT_FIL)
			retval = RET_NOTKT;
		else
			return(retval);
    }
    
	if (retval == KSUCCESS)
		retval = krb_get_cred(service,instance,realm,&cr);
    
    if (retval == RET_NOTKT ) {
  	if (retval = get_ad_tkt(service,instance,realm,lifetime)){
 
  
  	    if (krb_ap_req_debug){
  		kdebug_ap("problem with get_ad_tkt, %ld \n", retval);
  	    }
  
            return(retval);
  	}
  	if (retval = krb_get_cred(service,instance,realm,&cr)){
  	    if (krb_ap_req_debug){
  		kdebug_ap("problem with krb_get_cred, %ld \n", retval);
  	    }
            return(retval);
  	}
    }
    
    if (retval != KSUCCESS) return (retval);
    
    if (krb_ap_req_debug)
        kdebug_ap("%s %s %s %s %s\n", service, instance, realm,
                  cr.pname, cr.pinst);
    *kv = (unsigned char) cr.kvno;
    (void) strcpy((char *)(authent->dat+3),realm);
    *tl = (unsigned char) ticket->length;
    bcopy((char *)(ticket->dat),(char *)(authent->dat+6+strlen(realm)),
          ticket->length);
    authent->length = 6 + strlen(realm) + ticket->length;
    if (krb_ap_req_debug)
        kdebug_ap("Ticket->length = %d\n",ticket->length);
    if (krb_ap_req_debug)
        kdebug_ap("Issue date: %d\n",cr.issue_date);
    
    /* Build request id */
    (void) strcpy((char *)(req_id->dat),cr.pname); /* Auth name */
    req_id->length = strlen(cr.pname)+1;
    /* Principal's instance */
    (void) strcpy((char *)(req_id->dat+req_id->length),cr.pinst);
    req_id->length += strlen(cr.pinst)+1;
    /* Authentication domain */
    (void) strcpy((char *)(req_id->dat+req_id->length),myrealm);
    req_id->length += strlen(myrealm)+1;
    /* Checksum */
    bcopy((char *)&checksum,(char *)(req_id->dat+req_id->length),4);
    req_id->length += 4;
    
    /* Fill in the times on the request id */
    (void) gettimeofday(&tv_local,(struct timezone *) 0);
    *(req_id->dat+(req_id->length)++) =
        (unsigned char) tv_local.tv_usec;
    /* Time (coarse) */
    bcopy((char *)&(tv_local.tv_sec),
          (char *)(req_id->dat+req_id->length), 4);
    req_id->length += 4;
    
    /* Fill to a multiple of 8 bytes for DES */
    req_id->length = ((req_id->length+7)/8)*8;
    
#ifndef NOENCRYPTION
    /* Encrypt the request ID using the session key */
    key_sched(cr.session,key_s);
    pcbc_encrypt((C_Block *)req_id->dat,(C_Block *)req_id->dat,
                 (long) req_id->length,key_s,cr.session,1);
    /* clean up */
    bzero((char *) key_s, sizeof(key_s));
#endif /* NOENCRYPTION */
    
    /* Copy it into the authenticator */
    bcopy((char *)(req_id->dat),(char *)(authent->dat+authent->length),
          req_id->length);
    authent->length += req_id->length;
    /* And set the id length */
    *idl = (unsigned char) req_id->length;
    /* clean up */
    bzero((char *)req_id, sizeof(*req_id));
    
    if (krb_ap_req_debug)
        kdebug_ap("Authent->length = %d\n",authent->length);
    if (krb_ap_req_debug)
        kdebug_ap("idl = %d, tl = %d\n",(int) *idl, (int) *tl);
    
    return(KSUCCESS);
}
    
/*
 * krb_set_lifetime sets the default lifetime for additional tickets
 * obtained via krb_mk_req().
 *
 * It returns the previous value of the default lifetime.
 */
    
int
krb_set_lifetime(newval)
    int newval;
{
    int olife = lifetime;
    
    lifetime = newval;
    return(olife);
}
