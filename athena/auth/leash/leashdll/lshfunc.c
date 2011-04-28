#include <windows.h>
#include <stdio.h>
#include <sys/types.h>
#include <winsock.h>
#include "leashdll.h"
#include <krb.h>
#include <prot.h>
#include <time.h>

#include "leasherr.h"
#include "leash-int.h"
#include <leashwin.h>
#include <mitwhich.h>

#include <winkrbid.h>
#include "reminder.h"

#define NO_TICKETS 0    // Don't change this value
#define EXPD_TICKETS 2  // Don't change this value
#define GOOD_TICKETS 1  // Don't change this value
#define KRBERR(code) (code + krb_err_base)
#define LSHERR(code) (code + lsh_err_base) 

static char FAR *err_context;

char KRB_HelpFile[_MAX_PATH] =	HELPFILE;

#define LEN     64                /* Maximum Hostname Length */

#define LIFE    DEFAULT_TKT_LIFE  /* lifetime of ticket in 5-minute units */

char *
short_date(dp)
    long   *dp;
{
    register char *cp;
    cp = ctime(dp) + 4; // skip day of week
    // cp[15] = '\0';
    cp[12] = '\0'; // Don't display seconds
    return (cp);
}


static
char*
clean_string(
    char* s
    )
{
    char* p = s;
    char* b = s;

    if (!s) return s;

    for (p = s; *p; p++) {
        switch (*p) {
        case '\007':
            /* Add more cases here */
            break;
        default:
            *b = *p;
            b++;
        }
    }
    *b = *p;
    return s;
}

static
int
leash_error_message(
    const char *error,
    int rcL,
    int rc4,
    int rc5,
    int rcA,
    char* result_string
    )
{
    char message[2048];
    char *p = message;
    int size = sizeof(message);
    int n;

    // XXX: ignore AFS for now.

    if (!rc5 && !rc4 && !rcL)
        return 0;

    n = _snprintf(p, size, "%s\n\n", error);
    p += n;
    size -= n;

    if (rc5 && !result_string)
    {
        n = _snprintf(p, size,
                      "Kerberos 5: %s (error %ld)\n",
                      perror_message(rc5),
                      rc5 & 255 // XXX: & 255??!!!
            );
        p += n;
        size -= n;
    }
    if (rc4 && !result_string)
    {
        char buffer[1024];
        n = _snprintf(p, size,
                      "Kerberos 4: %s\n",
                      err_describe(buffer, rc4)
            );
        p += n;
        size -= n;
    }
    if (rcL)
    {
        char buffer[1024];
        n = _snprintf(p, size,
                      "\n%s\n",
                      err_describe(buffer, rcL)
            );
        p += n;
        size -= n;
    }
    if (result_string)
    {
        n = _snprintf(p, size,
                      "%s\n",
                      result_string);
        p += n;
        size -= n;
    }
    MessageBox(NULL, message, "Leash", MB_OK | MB_ICONERROR | MB_TASKMODAL | 
               MB_SETFOREGROUND);
    if (rc4) return rc4;
    if (rc5) return rc5;
    if (rcL) return rcL;
    return 0;
}


static
char *
make_postfix(
    const char * base,
    const char * postfix,
    char ** rcopy
    )
{
    int base_size;
    int ret_size;
    char * copy = 0;
    char * ret = 0;

    base_size = strlen(base) + 1;
    ret_size = base_size + strlen(postfix) + 1;
    copy = malloc(base_size);
    ret = malloc(ret_size);

    if (!copy || !ret)
        goto cleanup;

    strncpy(copy, base, base_size);
    copy[base_size - 1] = 0;

    strncpy(ret, base, base_size);
    strncpy(ret + (base_size - 1), postfix, ret_size - (base_size - 1));
    ret[ret_size - 1] = 0;

 cleanup:
    if (!copy || !ret) {
        if (copy)
            free(copy);
        if (ret)
            free(ret);
        copy = ret = 0;
    }
    // INVARIANT: (ret ==> copy) && (copy ==> ret)
    *rcopy = copy;
    return ret;
}

static
long
make_temp_cache_v4(
    const char * postfix
    )
{
    static char * old_cache = 0;

    if (!pkrb_set_tkt_string || !ptkt_string || !pdest_tkt)
        return 0; // XXX - is this appropriate?

    if (old_cache) {
        pdest_tkt();
        pkrb_set_tkt_string(old_cache);
        free(old_cache);
        old_cache = 0;
    }

    if (postfix)
    {
        char * tmp_cache = make_postfix(ptkt_string(), postfix, &old_cache);

        if (!tmp_cache)
            return KFAILURE;

        pkrb_set_tkt_string(tmp_cache);
        free(tmp_cache);
    }
    return 0;
}

static
long
make_temp_cache_v5(
    const char * postfix,
    krb5_context * pctx
    )
{
    static krb5_context ctx = 0;
    static char * old_cache = 0;

    // INVARIANT: old_cache ==> ctx && ctx ==> old_cache

    if (pctx)
        *pctx = 0;

    if (!pkrb5_init_context || !pkrb5_free_context || !pkrb5_cc_resolve ||
        !pkrb5_cc_default_name || !pkrb5_cc_set_default_name)
        return 0;

    if (old_cache) {
        krb5_ccache cc = 0;
        if (!pkrb5_cc_resolve(ctx, pkrb5_cc_default_name(ctx), &cc))
            pkrb5_cc_destroy(ctx, cc);
        pkrb5_cc_set_default_name(ctx, old_cache);
        free(old_cache);
        old_cache = 0;
    }
    if (ctx) {
        pkrb5_free_context(ctx);
        ctx = 0;
    }

    if (postfix)
    {
        char * tmp_cache = 0;
        krb5_error_code rc = 0;

        rc = pkrb5_init_context(&ctx);
        if (rc) goto cleanup;

        tmp_cache = make_postfix(pkrb5_cc_default_name(ctx), postfix, 
                                 &old_cache);

        if (!tmp_cache) {
            rc = ENOMEM;
            goto cleanup;
        }

        rc = pkrb5_cc_set_default_name(ctx, tmp_cache);

    cleanup:
        if (rc && ctx) {
            pkrb5_free_context(ctx);
            ctx = 0;
        }
        if (tmp_cache)
            free(tmp_cache);
        if (pctx)
            *pctx = ctx;
        return rc;
    }
    return 0;
}

long
Leash_checkpwd(
    char *principal, 
    char *password
    )
{
    long rc = 0;
    krb5_context ctx = 0;
    // XXX - we ignore errors in make_temp_cache_v?  This is BAD!!!
    make_temp_cache_v4("_checkpwd");
    make_temp_cache_v5("_checkpwd", &ctx);
    rc = Leash_kinit_ex(principal, password, 0, ctx);
    make_temp_cache_v4(0);
    make_temp_cache_v5(0, &ctx);
    return rc;
}

static
long
Leash_changepwd_v5(
    char * principal,
    char * password,
    char * newpassword,
    char** error_str
    )
{
    krb5_error_code rc = 0;
    int result_code;
    krb5_data result_code_string, result_string;
    krb5_context context = 0;
    krb5_principal princ = 0;
    krb5_get_init_creds_opt opts;
    krb5_creds creds;

    result_string.data = 0;
    result_code_string.data = 0;

   if (rc = pkrb5_init_context(&context)) {
#if 0
       com_err(argv[0], ret, "initializing kerberos library");
#endif
       goto cleanup;
   }

   if (rc = pkrb5_parse_name(context, principal, &princ)) {
#if 0
       com_err(argv[0], ret, "parsing client name");
#endif
       goto cleanup;
   }

   pkrb5_get_init_creds_opt_init(&opts);
   pkrb5_get_init_creds_opt_set_tkt_life(&opts, 5*60);
   pkrb5_get_init_creds_opt_set_renew_life(&opts, 0);
   pkrb5_get_init_creds_opt_set_forwardable(&opts, 0);
   pkrb5_get_init_creds_opt_set_proxiable(&opts, 0);

   if (rc = pkrb5_get_init_creds_password(context, &creds, princ, password,
                                          0, 0, 0, "kadmin/changepw", &opts)) {
       if (rc == KRB5KRB_AP_ERR_BAD_INTEGRITY) {
#if 0
           com_err(argv[0], 0,
                   "Password incorrect while getting initial ticket");
#endif
       }
       else {
#if 0
           com_err(argv[0], ret, "getting initial ticket");
#endif
       }
       goto cleanup;
   }

   if (rc = pkrb5_change_password(context, &creds, newpassword,
                                  &result_code, &result_code_string,
                                  &result_string)) {
#if 0
       com_err(argv[0], ret, "changing password");
#endif
       goto cleanup;
   }

   if (result_code) {
       int len = result_code_string.length + 
           (result_string.length ? (sizeof(": ") - 1) : 0) +
           result_string.length;
       if (len && error_str) {
           *error_str = malloc(len + 1);
           if (*error_str)
               _snprintf(*error_str, len + 1,
                         "%.*s%s%.*s",
                         result_code_string.length, result_code_string.data,
                         result_string.length?": ":"",
                         result_string.length, result_string.data);
       }
      rc = result_code;
      goto cleanup;
   }

 cleanup:
   if (result_string.data)
       pkrb5_free_data_contents(context, &result_string);

   if (result_code_string.data)
       pkrb5_free_data_contents(context, &result_code_string);

   if (princ)
       pkrb5_free_principal(context, princ);

   if (context)
       pkrb5_free_context(context);

   return rc;
}

static
long
Leash_changepwd_v4(
    char * principal,
    char * password,
    char * newpassword,
    char** error_str
    )
{
    long k_errno;

    if (!pkrb_set_tkt_string || !ptkt_string || !pkadm_change_your_password ||
        !pdest_tkt)
        return KFAILURE;

    k_errno = make_temp_cache_v4("_chgpwd");
    if (k_errno) return k_errno;
    k_errno = pkadm_change_your_password(principal, password, newpassword, 
                                         error_str);
    make_temp_cache_v4(0);
    return k_errno;
}

/*
 * Leash_changepwd
 *
 * Try to change the password using one of krb5 or krb4 -- whichever one
 * works.  We return ok on the first one that works.
 */
long
Leash_changepwd(
    char * principal, 
    char * password, 
    char * newpassword,
    char** result_string
    )
{
    char* v5_error_str = 0;
    char* v4_error_str = 0;
    char* error_str = 0;
    int rc4 = 0;
    int rc5 = 0;
    int rc = 0;
    if (hKrb5)
        rc = rc5 = Leash_changepwd_v5(principal, password, newpassword,
                                      &v5_error_str);
    if (hKrb4 && (!hKrb5 || rc5))
        rc = rc4 = Leash_changepwd_v4(principal, password, newpassword, 
                                      &v4_error_str);
    if (!rc)
        return 0;
    if (v5_error_str || v4_error_str) {
        int len = 0;
        char v5_prefix[] = "Kerberos 5: ";
        char sep[] = "\n";
        char v4_prefix[] = "Kerberos 4: ";

        clean_string(v5_error_str);
        clean_string(v4_error_str);

        if (v5_error_str)
            len += sizeof(sep) + sizeof(v5_prefix) + strlen(v5_error_str) + 
                sizeof(sep);
        if (v4_error_str)
            len += sizeof(sep) + sizeof(v4_prefix) + strlen(v4_error_str) + 
                sizeof(sep);
        error_str = malloc(len + 1);
        if (error_str) {
            char* p = error_str;
            int size = len + 1;
            int n;
            if (v5_error_str) {
                n = _snprintf(p, size, "%s%s%s%s",
                              sep, v5_prefix, v5_error_str, sep);
                p += n;
                size -= n;
            }
            if (v4_error_str) {
                n = _snprintf(p, size, "%s%s%s%s",
                              sep, v4_prefix, v4_error_str, sep);
                p += n;
                size -= n;
            }
            if (result_string)
                *result_string = error_str;
        }
    }
    return leash_error_message("Error while changing password.", 
                               rc4, rc4, rc5, 0, error_str);
}

int (*Lcom_err)(LPSTR,long,LPSTR,...);
LPSTR (*Lerror_message)(long);
LPSTR (*Lerror_table_name)(long);


long
Leash_kinit(
    char * principal,
    char * password,
    int lifetime
    )
{
    return Leash_kinit_ex(principal, password, lifetime, 0);
}

long
Leash_kinit_ex(
    char * principal, 
    char * password, 
    int lifetime,
    krb5_context ctx
    )
{
    LPCSTR  functionName; 
    char    aname[ANAME_SZ];
    char    inst[INST_SZ];
    char    realm[REALM_SZ];
    char    first_part[256];
    char    second_part[256];
    char    temp[1024];
    int     count;
    int     i;
    int rc4 = 0;
    int rc5 = 0;
    int rcA = 0;
    int rcL = 0;

    if (lifetime < 5)
        lifetime = 1;
    else
        lifetime /= 5;

    /* This should be changed if the maximum ticket lifetime */
    /* changes */

    if (lifetime > 255)
        lifetime = 255;

    err_context = "parsing principal";

    memset(temp, '\0', sizeof(temp));
    memset(inst, '\0', sizeof(inst));
    memset(realm, '\0', sizeof(realm));
    memset(first_part, '\0', sizeof(first_part));
    memset(second_part, '\0', sizeof(second_part));

    sscanf(principal, "%[/0-9a-zA-Z._-]@%[/0-9a-zA-Z._-]", first_part, second_part);
    strcpy(temp, first_part);
    strcpy(realm, second_part);
    memset(first_part, '\0', sizeof(first_part));
    memset(second_part, '\0', sizeof(second_part));
    if (sscanf(temp, "%[@0-9a-zA-Z._-]/%[@0-9a-zA-Z._-]", first_part, second_part) == 2)
    {
        strcpy(aname, first_part);
        strcpy(inst, second_part);
    }
    else
    {
        count = 0;
        i = 0;
        for (i = 0; temp[i]; i++)
        {
            if (temp[i] == '.')
                ++count;
        }
        if (count > 1)
        {
            strcpy(aname, temp);
        }
        else
        {
            if (pkname_parse != NULL)
            {
                memset(first_part, '\0', sizeof(first_part));
                memset(second_part, '\0', sizeof(second_part));
                sscanf(temp, "%[@/0-9a-zA-Z_-].%[@/0-9a-zA-Z_-]", first_part, second_part);
                strcpy(aname, first_part);
                strcpy(inst, second_part);
            }
            else
            {
                strcpy(aname, temp);
            }
        }
    }

    memset(temp, '\0', sizeof(temp));
    strcpy(temp, aname);
    if (strlen(inst) != 0)
    {
        strcat(temp, "/");
        strcat(temp, inst);
    }
    if (strlen(realm) != 0)
    {
        strcat(temp, "@");
        strcat(temp, realm);
    }

    rc5 = Leash_krb5_kinit(temp, password, lifetime, ctx);

    if (pkname_parse == NULL)
    {
        goto cleanup;
    }

    err_context = "getting realm";
    if (!*realm && (rc4  = (int)(*pkrb_get_lrealm)(realm, 1))) 
    {
        functionName = "krb_get_lrealm()";
        rcL  = LSH_FAILEDREALM;
        goto cleanup;
    }

    err_context = "checking principal";
    if ((!*aname) || (!(rc4  = (int)(*pk_isname)(aname))))
    {
        functionName = "krb_get_lrealm()";
        rcL = LSH_INVPRINCIPAL;
        goto cleanup;
    }

    /* optional instance */
    if (!(rc4 = (int)(*pk_isinst)(inst)))
    {
        functionName = "k_isinst()";
        rcL = LSH_INVINSTANCE;
        goto cleanup;
    }
	
    if (!(rc4 = (int)(*pk_isrealm)(realm)))
    {
        functionName = "k_isrealm()";
        rcL = LSH_INVREALM;
        goto cleanup;
    }

    err_context = "fetching ticket";	
    if (!(rc4 = (*pkrb_get_pw_in_tkt)(aname, inst, realm, "krbtgt", realm, 
                                      lifetime, password)))
    {
        rcA = Leash_afs_klog("", "", "", lifetime);
    }
    else if (rc4) /* XXX: do we want: && (rc != NO_TKT_FIL) as well? */
    { 
        functionName = "krb_get_pw_in_tkt()";
        rcL = KRBERR(rc4);
        goto cleanup;
    }

    return 0;

 cleanup:
    return leash_error_message("Ticket initialization failed.", 
                               rcL, rc4?KRBERR(rc4):0, rc5, rcA, 0);

#pragma message(Reminder "Are we returning the right thing?")
}

long Leash_kdestroy(void)
{
    int k_errno;

    Leash_afs_unlog();
    Leash_krb5_kdestroy();

    if (pdest_tkt != NULL)
    {
        k_errno = (*pdest_tkt)();
        if (k_errno && (k_errno != RET_TKFIL))
            return KRBERR(k_errno);
    }

    return 0;
}

int com_addr(void)
{
    long ipAddr;
    char loc_addr[ADDR_SZ];
    CREDENTIALS cred;    
    char service[40];
    char instance[40];
//    char addr[40];
    char realm[40];
    struct in_addr LocAddr;
    int k_errno;

    if (pkrb_get_cred == NULL)
        return(KSUCCESS);

    k_errno = (*pkrb_get_cred)(service,instance,realm,&cred);
    if (k_errno)
        return KRBERR(k_errno);


    while(1) {
	ipAddr = (*pLocalHostAddr)();
	LocAddr.s_addr = ipAddr;
        strcpy(loc_addr,inet_ntoa(LocAddr));
	if ( strcmp(cred.address,loc_addr) != 0) {
            Leash_kdestroy ();
            break;
	}
        break;
    } // while()
    return 0;
} 

long 
not_an_API_LeashFreeTicketList(TicketList** ticketList) 
{
    TicketList* tempList = *ticketList, *killList; 

    //if (tempList == NULL)
    //return -1;

    while (tempList)
    {
        killList = tempList;
           
        tempList = (TicketList*)tempList->next;
        free(killList->theTicket);
        free(killList);
    }

    *ticketList = NULL;

    return 0;
}

long 
not_an_API_LeashKRB4GetTickets(TICKETINFO FAR* ticketinfo, 
                               TicketList** ticketList) 
{
    // Puts tickets in a returned linklist - Can be used with many
    // diff. controls
    char    pname[ANAME_SZ];
    char    pinst[INST_SZ];
    char    prealm[REALM_SZ];
    char    buf[MAX_K_NAME_SZ+40];
    LPSTR   cp;
    LPSTR   functionName;
    long    expdate;
    int     k_errno;
    CREDENTIALS c;
    int newtickets = 0;
    int open = 0;

    TicketList* list = NULL;

    //ticketList = NULL;

    // Since krb_get_tf_realm will return a ticket_file error,
    // we will call tf_init and tf_close first to filter out
    // things like no ticket file.  Otherwise, the error that
    // the user would see would be
    // klist: can't find realm of ticket file: No ticket file (tf_util)
    // instead of klist: No ticket file (tf_util)
    if (ptf_init == NULL)
        return(KSUCCESS);

    com_addr();                    
    err_context = (LPSTR)"tktf1";

    // Open ticket file
    if ((k_errno = (*ptf_init)((*ptkt_string)(), R_TKT_FIL)))
    {
        functionName = "ptf_init()";
        goto cleanup;
    }
    // Close ticket file 
    (void) (*ptf_close)();
    
    // We must find the realm of the ticket file here before calling
    // tf_init because since the realm of the ticket file is not
    // really stored in the principal section of the file, the
    // routine we use must itself call tf_init and tf_close.

    err_context = "tf realm";
    if ((k_errno = (*pkrb_get_tf_realm)((*ptkt_string)(), prealm)) != KSUCCESS)
    {
        functionName = "pkrb_get_tf_realm()";
        goto cleanup;
    }
	
    // Open ticket file 
    err_context = "tf init";
    if (k_errno = (*ptf_init)((*ptkt_string)(), R_TKT_FIL)) 
    {
        functionName = "sptf_init()";
        goto cleanup;
    }

    open = 1;
    err_context = "tf pname";

    // Get principal name and instance 
    if ((k_errno = (*ptf_get_pname)(pname)) || (k_errno = (*ptf_get_pinst)(pinst))) 
    {
        functionName = "ptf_get_pname()";
        goto cleanup;
    }
	
    // You may think that this is the obvious place to get the
    // realm of the ticket file, but it can't be done here as the
    // routine to do this must open the ticket file.  This is why
    // it was done before tf_init.
    wsprintf((LPSTR)ticketinfo->principal,"%s%s%s%s%s", (LPSTR)pname,
             (LPSTR)(pinst[0] ? "." : ""), (LPSTR)pinst,
             (LPSTR)(prealm[0] ? "@" : ""), (LPSTR)prealm);

    newtickets = NO_TICKETS;
    err_context = "tf cred";

    // Get KRB4 tickets
    while ((k_errno = (*ptf_get_cred)(&c)) == KSUCCESS) 
    {
        if (!list)
        {
            list = (TicketList*) calloc(1, sizeof(TicketList));
            (*ticketList) = list; 
        }    
        else 
        {
            list->next = (struct TicketList*) calloc(1, sizeof(TicketList));
            list = (TicketList*) list->next;
        }
        
        expdate = c.issue_date + c.lifetime * 5L * 60L;

        if (!lstrcmp((LPSTR)c.service, (LPSTR)TICKET_GRANTING_TICKET) && !lstrcmp((LPSTR)c.instance, (LPSTR)prealm)) 
        {
            ticketinfo->issue_date = c.issue_date;
            ticketinfo->lifetime = c.lifetime * 5L * 60L;
        }

        newtickets = GOOD_TICKETS;

        cp = (LPSTR)buf;
        cp += wsprintf(cp, "%s     ",
                       short_date(&c.issue_date));
        wsprintf(cp, "%s     %s%s%s%s%s",
                 short_date(&expdate), 
                 c.service,
                 (c.instance[0] ? "." : ""),
                 c.instance,
                 (c.realm[0] ? "@" : ""),
                 c.realm);

        list->theTicket = (char*) calloc(1, sizeof(buf));
        if (!list->theTicket)
        {
            MessageBox(NULL, "Memory Error", "Error", MB_OK);
            return ENOMEM;            
        }       

        strcpy(list->theTicket, buf);

    } // while
    functionName = "not_an_API_LeashKRB4GetTickets()";

cleanup:
    if (ptf_close == NULL)
        return(KSUCCESS);

    if (open)
        (*ptf_close)(); //close ticket file 
    
    if (k_errno == EOF)
        k_errno = 0;

    // XXX the if statement directly below was inserted to eliminate
    // an error NO_TKT_FIL on Leash startup. The error occurs from an
    // error number thrown from krb_get_tf_realm.  We believe this
    // change does not eliminate other errors, but it may.

    if (k_errno == NO_TKT_FIL)
        k_errno = 0;

    ticketinfo->btickets = newtickets;
	
    if (k_errno) /* XXX: do we want: && (rc != NO_TKT_FIL) as well? */
    {
        CHAR message[256];
        CHAR errBuf[256];
        LPCSTR errText; 

        if (!Lerror_message)
            return -1;

        errText = err_describe(errBuf, KRBERR(k_errno));

        sprintf(message, "%s\n\n%s failed", errText, functionName);
        MessageBox(NULL, message, "Kerberos Four", 
                   MB_OK | MB_ICONERROR | MB_TASKMODAL | MB_SETFOREGROUND);
    }
    return k_errno;
}

long FAR Leash_klist(HWND hlist, TICKETINFO FAR *ticketinfo)
{
    // Don't think this function will be used anymore - ADL 5-15-99    
    // Old fucntion to put tickets in a listbox control  
    // Use function  "not_an_API_LeashKRB4GetTickets()" instead! 
    char    pname[ANAME_SZ];
    char    pinst[INST_SZ];
    char    prealm[REALM_SZ];
    char    buf[MAX_K_NAME_SZ+40];
    LPSTR   cp;
    long    expdate;
    int     k_errno;
    CREDENTIALS c;
    int newtickets = 0;
    int open = 0;

    /*
     * Since krb_get_tf_realm will return a ticket_file error,
     * we will call tf_init and tf_close first to filter out
     * things like no ticket file.  Otherwise, the error that
     * the user would see would be 
     * klist: can't find realm of ticket file: No ticket file (tf_util)
     * instead of
     * klist: No ticket file (tf_util)
     */
    if (ptf_init == NULL)
        return(KSUCCESS);

    if (hlist) 
    { 
        SendMessage(hlist, WM_SETREDRAW, FALSE, 0L);
        SendMessage(hlist, LB_RESETCONTENT, 0, 0L);
    }                              
    com_addr();                    
    newtickets = NO_TICKETS;

    err_context = (LPSTR)"tktf1";

    /* Open ticket file */
    if (k_errno = (*ptf_init)((*ptkt_string)(), R_TKT_FIL))
    {
        goto cleanup;
    }
    /* Close ticket file */
    (void) (*ptf_close)();
    /*
     * We must find the realm of the ticket file here before calling
     * tf_init because since the realm of the ticket file is not
     * really stored in the principal section of the file, the
     * routine we use must itself call tf_init and tf_close.
     */
    err_context = "tf realm";
    if ((k_errno = (*pkrb_get_tf_realm)((*ptkt_string)(), prealm)) != KSUCCESS)
    {
        goto cleanup;
    }
    /* Open ticket file */
    err_context = "tf init";
    if (k_errno = (*ptf_init)((*ptkt_string)(), R_TKT_FIL)) 
    {
        goto cleanup;                            
    }

    open = 1;
    err_context = "tf pname";
    /* Get principal name and instance */
    if ((k_errno = (*ptf_get_pname)(pname)) || (k_errno = (*ptf_get_pinst)(pinst))) 
    {
        goto cleanup;             
    }

    /*
     * You may think that this is the obvious place to get the
     * realm of the ticket file, but it can't be done here as the
     * routine to do this must open the ticket file.  This is why
     * it was done before tf_init.
     */

    wsprintf((LPSTR)ticketinfo->principal,"%s%s%s%s%s", (LPSTR)pname,
             (LPSTR)(pinst[0] ? "." : ""), (LPSTR)pinst,
             (LPSTR)(prealm[0] ? "@" : ""), (LPSTR)prealm);
    newtickets = GOOD_TICKETS;

    err_context = "tf cred";
    while ((k_errno = (*ptf_get_cred)(&c)) == KSUCCESS) 
    {
        expdate = c.issue_date + c.lifetime * 5L * 60L;

        if (!lstrcmp((LPSTR)c.service, (LPSTR)TICKET_GRANTING_TICKET) && !lstrcmp((LPSTR)c.instance, (LPSTR)prealm)) 
        {
            ticketinfo->issue_date = c.issue_date;
            ticketinfo->lifetime = c.lifetime * 5L * 60L;
        }

        cp = (LPSTR)buf;
        lstrcpy(cp, (LPSTR)short_date(&c.issue_date));
        cp += lstrlen(cp);
        wsprintf(cp,"\t%s\t%s%s%s%s%s",
                 (LPSTR)short_date(&expdate), (LPSTR)c.service,
                 (LPSTR)(c.instance[0] ? "." : ""),
                 (LPSTR)c.instance, (LPSTR)(c.realm[0] ? "@" : ""),
                 (LPSTR) c.realm);
        if (hlist)
            SendMessage(hlist, LB_ADDSTRING, 0, (LONG)(LPSTR)buf);
    } /* WHILE */

cleanup:

    if (open)
        (*ptf_close)(); /* close ticket file */

    if (hlist) 
    {
        SendMessage(hlist, WM_SETREDRAW, TRUE, 0L);
        InvalidateRect(hlist, NULL, TRUE);
        UpdateWindow(hlist);
    }
    if (k_errno == EOF)
        k_errno = 0;

    /* XXX the if statement directly below was inserted to eliminate
       an error 20 on Leash startup. The error occurs from an error
       number thrown from krb_get_tf_realm.  We believe this change
       does not eliminate other errors, but it may. */

    if (k_errno == RET_NOTKT)
        k_errno = 0;

    ticketinfo->btickets = newtickets;
    if (k_errno != 0)
        return KRBERR(k_errno);
    return 0;
}


// This function can be used to set the help file that will be
// referenced the DLL's PasswordProcDLL function and err_describe
// function.  Returns true if the help file has been set to the
// argument or the environment variable KERB_HELP. Returns FALSE if
// the default helpfile as defined in by HELPFILE in lsh_pwd.h is
// used.
BOOL Leash_set_help_file( char *szHelpFile )
{
    char *tmpHelpFile;
    BOOL ret = 0;

    if( szHelpFile == NULL ){
	tmpHelpFile = getenv("KERB_HELP");
    } else {
	strcpy( KRB_HelpFile, szHelpFile );
	ret++;
    }

    if( !ret && tmpHelpFile ){
	strcpy( KRB_HelpFile, tmpHelpFile );
	ret++;
    }

    if( !ret){
	strcpy( KRB_HelpFile, HELPFILE );
    }

    return(ret);
}



LPSTR Leash_get_help_file(void)
{
    return( KRB_HelpFile);
}

#if 0
/**************************************/
/* LeashKrb4ErrorMessage():           */
/**************************************/
long
LeashKrb4ErrorMessage(LONG rc, LPCSTR FailedFunctionName)
{
    // At this time the Leashw32.dll. takes care of all error messages. We 
    // may want to add a flag latter on so the .exe can handle it's own 
    // errors.

    CHAR message[256];
    CHAR errBuf[256];
    LPCSTR errText; 

    if (!Lerror_message)
      return -1;

    errText = err_describe(errBuf, rc);

    sprintf(message, "%s\n\n%s failed", errText, FailedFunctionName);
    MessageBox(NULL, message, "Kerberos Four", MB_OK | 
                                               MB_ICONERROR | 
                                               MB_TASKMODAL | 
                                               MB_SETFOREGROUND);
    return rc;
}
#endif

int 
Leash_debug(
    int class, 
    int priority, 
    char* fmt, ...
    )
{

    return 0;
}

/*
 * Leash_get_default_lifetime:
 *
 * This function is used to get the default ticket lifetime for this
 * process in minutes.  A return value of 0 indicates no setting or
 * "default" setting obtained.
 *
 * Here is where we look in order:
 *
 * - LIFETIME environment variable
 * - HKCU\Software\MIT\Leash,lifetime
 * - HKLM\Software\MIT\Leash,lifetime
 * - string resource in the leash DLL
 */

static
BOOL
get_DWORD_from_registry(
    HKEY hBaseKey,
    char * key,
    char * value,
    DWORD * result
    )
{
    HKEY hKey;
    DWORD dwCount;
    LONG rc;

    rc = RegOpenKeyEx(hBaseKey, key, 0, KEY_QUERY_VALUE, &hKey);
    if (rc)
        return FALSE;

    dwCount = sizeof(DWORD);
    rc = RegQueryValueEx(hKey, value, 0, 0, (LPBYTE) result, &dwCount);
    RegCloseKey(hKey);

    return rc?FALSE:TRUE;
}

#define LEASH_REGISTRY_KEY_NAME "Software\\MIT\\Leash"
#define LEASH_REGISTRY_VALUE_LIFETIME "lifetime"

static
BOOL
get_default_lifetime_from_registry(
    HKEY hBaseKey,
    DWORD * result
    )
{
    return get_DWORD_from_registry(hBaseKey, 
                                   LEASH_REGISTRY_KEY_NAME,
                                   LEASH_REGISTRY_VALUE_LIFETIME,
                                   result);
}

#if 0
//
// This is not exported.  We may eventually want it, but not now.
//
static
DWORD
Leash_reset_default_lifetime(
    )
{
    HKEY hKey;
    LONG rc;

    rc = RegOpenKeyEx(HKEY_CURRENT_USER, LEASH_REGISTRY_KEY_NAME, 0, KEY_WRITE, &hKey);
    if (rc)
        return rc;

    rc = RegDeleteValue(hKey, LEASH_REGISTRY_VALUE_LIFETIME);
    RegCloseKey(hKey);

    return rc;
}
#endif

DWORD
Leash_set_default_lifetime(
    DWORD minutes
    )
{
    HKEY hKey;
    LONG rc;

    rc = RegCreateKeyEx(HKEY_CURRENT_USER, LEASH_REGISTRY_KEY_NAME, 0, 
                        0, 0, KEY_WRITE, 0, &hKey, 0);
    if (rc)
        return rc;

    rc = RegSetValueEx(hKey, LEASH_REGISTRY_VALUE_LIFETIME, 0, REG_DWORD, 
                       (LPBYTE) &minutes, sizeof(DWORD));
    RegCloseKey(hKey);

    return rc;
}

#include "leashids.h"

DWORD
Leash_get_default_lifetime(
    )
{
    HMODULE hmLeash;
    char *env;
    DWORD result;

    if(env = getenv("LIFETIME"))
    {
        return atoi(env);
    }

    if (get_default_lifetime_from_registry(HKEY_CURRENT_USER, &result) ||
        get_default_lifetime_from_registry(HKEY_LOCAL_MACHINE, &result))
    {
        return result;
    }

    hmLeash = GetModuleHandle(LEASH_DLL);
    if (hmLeash)
    {
        char lifetime[80];
        if (LoadString(hmLeash, LSH_DEFAULT_TICKET_LIFE, 
                       lifetime, sizeof(lifetime)))
        {
            lifetime[sizeof(lifetime) - 1] = 0;
            return atoi(lifetime);
        }
    }
    return 0;
}
