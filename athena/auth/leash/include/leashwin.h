#ifndef __LEASHWIN__
#define __LEASHWIN__

#include <krb.h>

typedef struct {
    int dlgtype;
#define DLGTYPE_PASSWD   0
#define DLGTYPE_CHPASSWD 1
    // Tells whether dialog box is in change pwd more or init ticket mode???
    // (verify this):
    int dlgstatemax; // What is this???
    // The title on the Dialog box - for Renewing or Initializing:
    LPSTR title;
    LPSTR principal;
} LSH_DLGINFO, FAR *LPLSH_DLGINFO;

#define PASSWORDCHAR '#'

typedef struct {                                                
    char    principal[MAX_K_NAME_SZ]; /* Principal name/instance/realm */
    int     btickets;                 /* Do we have tickets? */
    long    lifetime;                 /* Lifetime -- needs to have
                                         room for 255 5-minute
                                         periods * 5 * 60 */
    long    issue_date;               /* The issue time */
} TICKETINFO;

int FAR Leash_kinit_dlg(HWND hParent, LPLSH_DLGINFO lpdlginfo);
int FAR Leash_changepwd_dlg(HWND hParent, LPLSH_DLGINFO lpdlginfo);

long FAR Leash_checkpwd(char *principal, char *password);
long FAR Leash_changepwd(char *principal, char *password, char *newpassword, char** result_string);
long FAR Leash_kinit(char *principal, char *password, int lifetime);
long FAR Leash_klist(HWND hlist, TICKETINFO FAR *ticketinfo);
long FAR Leash_kdestroy(void);
long FAR Leash_get_lsh_errno( LONG FAR *err_val);

BOOL Leash_set_help_file( char FAR *szHelpFile );
LPSTR Leash_get_help_file(void);


#include <com_err.h>

/*
 * This is a hack needed because the real com_err.h does
 * not define err_func.  We need it in the case where
 * we pull in the real com_err instead of the krb4 
 * impostor.
 */
#ifndef _DCNS_MIT_COM_ERR_H
typedef LPSTR (*err_func)(int, long);
#endif

#include <krberr.h>
extern void Leash_initialize_krb_error_func(err_func func,struct et_list **);
#undef init_krb_err_func
#define init_krb_err_func(erf) Leash_initialize_krb_error_func(erf,&_et_list)

#include <kadm_err.h>

extern void Leash_initialize_kadm_error_table(struct et_list **);
#undef init_kadm_err_tbl
#define init_kadm_err_tbl() Leash_initialize_kadm_error_table(&_et_list)
#define kadm_err_base ERROR_TABLE_BASE_kadm

#define krb_err_func Leash_krb_err_func

#include <stdarg.h>
int lsh_com_err_proc (LPSTR whoami, long code,
		      LPSTR fmt, va_list args);
void FAR Leash_load_com_err_callback(FARPROC,FARPROC,FARPROC);

#define NO_TICKETS 0
#define EXPD_TICKETS 2
#define GOOD_TICKETS 1

#ifndef KRBERR
#define KRBERR(code) (code + krb_err_base)
#endif

DWORD
Leash_get_default_lifetime(
    );

DWORD
Leash_set_default_lifetime(
    DWORD minutes
    );


#endif /* LEASHWIN */
