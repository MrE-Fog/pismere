/*
 * lib/gssapi/krb5/gssapi_err_krb5.h:
 * This file is automatically generated; please do not edit it.
 */

#include <com_err.h>

#define KG_CCACHE_NOMATCH                        (39756032L)
#define KG_KEYTAB_NOMATCH                        (39756033L)
#define KG_TGT_MISSING                           (39756034L)
#define KG_NO_SUBKEY                             (39756035L)
#define KG_CONTEXT_ESTABLISHED                   (39756036L)
#define KG_BAD_SIGN_TYPE                         (39756037L)
#define KG_BAD_LENGTH                            (39756038L)
#define KG_CTX_INCOMPLETE                        (39756039L)
#define KG_CONTEXT                               (39756040L)
#define KG_CRED                                  (39756041L)
#define KG_ENC_DESC                              (39756042L)
#define KG_BAD_SEQ                               (39756043L)
#define KG_EMPTY_CCACHE                          (39756044L)
#define KG_NO_CTYPES                             (39756045L)
#define ERROR_TABLE_BASE_k5g (39756032L)

extern const struct error_table et_k5g_error_table;

#if !defined(_WIN32)
/* for compatibility with older versions... */
extern void initialize_k5g_error_table () /*@modifies internalState@*/;
#else
#define initialize_k5g_error_table()
#endif

#if !defined(_WIN32)
#define init_k5g_err_tbl initialize_k5g_error_table
#define k5g_err_base ERROR_TABLE_BASE_k5g
#endif
