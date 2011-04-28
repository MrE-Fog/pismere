#include <windows.h>
#include "leashdll.h"
#include <krb.h>
#include <leashwin.h>
#include "leash-int.h"

HINSTANCE hLeashInst;

HINSTANCE hKrb4 = 0;
HINSTANCE hKrb5 = 0;

// krb4 functions
DECL_FUNC_PTR(get_krb_err_txt_entry);
DECL_FUNC_PTR(k_isinst);
DECL_FUNC_PTR(k_isname);
DECL_FUNC_PTR(k_isrealm);
DECL_FUNC_PTR(kadm_change_your_password);
DECL_FUNC_PTR(kname_parse);
DECL_FUNC_PTR(krb_get_cred);
DECL_FUNC_PTR(krb_get_krbhst);
DECL_FUNC_PTR(krb_get_lrealm);
DECL_FUNC_PTR(krb_get_pw_in_tkt);
DECL_FUNC_PTR(krb_get_tf_realm);
DECL_FUNC_PTR(krb_mk_req);
DECL_FUNC_PTR(krb_realmofhost);
DECL_FUNC_PTR(tf_init);
DECL_FUNC_PTR(tf_close);
DECL_FUNC_PTR(tf_get_cred);
DECL_FUNC_PTR(tf_get_pname);
DECL_FUNC_PTR(tf_get_pinst);
DECL_FUNC_PTR(LocalHostAddr);
DECL_FUNC_PTR(tkt_string);
DECL_FUNC_PTR(krb_set_tkt_string);
DECL_FUNC_PTR(initialize_krb_error_func);
DECL_FUNC_PTR(initialize_kadm_error_table);
DECL_FUNC_PTR(dest_tkt);
DECL_FUNC_PTR(lsh_LoadKrb4LeashErrorTables); // XXX

// krb5 functions
DECL_FUNC_PTR(krb5_change_password);
DECL_FUNC_PTR(krb5_get_init_creds_opt_init);
DECL_FUNC_PTR(krb5_get_init_creds_opt_set_tkt_life);
DECL_FUNC_PTR(krb5_get_init_creds_opt_set_renew_life);
DECL_FUNC_PTR(krb5_get_init_creds_opt_set_forwardable);
DECL_FUNC_PTR(krb5_get_init_creds_opt_set_proxiable);
DECL_FUNC_PTR(krb5_get_init_creds_password);
DECL_FUNC_PTR(krb5_build_principal_ext);
DECL_FUNC_PTR(krb5_cc_resolve);
DECL_FUNC_PTR(krb5_cc_default);
DECL_FUNC_PTR(krb5_cc_default_name);
DECL_FUNC_PTR(krb5_cc_set_default_name);
DECL_FUNC_PTR(krb5_free_context);
DECL_FUNC_PTR(krb5_free_cred_contents);
DECL_FUNC_PTR(krb5_free_principal);
DECL_FUNC_PTR(krb5_get_in_tkt_with_password);
DECL_FUNC_PTR(krb5_init_context);
DECL_FUNC_PTR(krb5_parse_name);
DECL_FUNC_PTR(krb5_timeofday);
DECL_FUNC_PTR(krb5_timestamp_to_sfstring);
DECL_FUNC_PTR(krb5_unparse_name);
DECL_FUNC_PTR(krb5_get_credentials);
DECL_FUNC_PTR(krb5_mk_req);
DECL_FUNC_PTR(krb5_sname_to_principal);
DECL_FUNC_PTR(krb5_get_credentials_renew);
DECL_FUNC_PTR(krb5_free_data);
DECL_FUNC_PTR(krb5_free_data_contents);
DECL_FUNC_PTR(krb5_get_realm_domain);
DECL_FUNC_PTR(krb5_free_unparsed_name);

// ComErr functions
DECL_FUNC_PTR(com_err);
DECL_FUNC_PTR(error_message);

// AFS functions
DECL_FUNC_PTR(ktc_ListTokens);
DECL_FUNC_PTR(ktc_GetToken);
DECL_FUNC_PTR(ktc_SetToken);
DECL_FUNC_PTR(ktc_ForgetAllTokens);
DECL_FUNC_PTR(cm_GetRootCellName);
DECL_FUNC_PTR(cm_SearchCellFile);

// Service functions
DECL_FUNC_PTR(OpenSCManagerA);
DECL_FUNC_PTR(OpenServiceA);
DECL_FUNC_PTR(QueryServiceStatus);
DECL_FUNC_PTR(CloseServiceHandle);

FUNC_INFO k4_fi[] = {
    MAKE_FUNC_INFO(get_krb_err_txt_entry),
    MAKE_FUNC_INFO(k_isinst),
    MAKE_FUNC_INFO(k_isname),
    MAKE_FUNC_INFO(k_isrealm),
    MAKE_FUNC_INFO(kadm_change_your_password),
    MAKE_FUNC_INFO(kname_parse),
    MAKE_FUNC_INFO(krb_get_cred),
    MAKE_FUNC_INFO(krb_get_krbhst),
    MAKE_FUNC_INFO(krb_get_lrealm),
    MAKE_FUNC_INFO(krb_get_pw_in_tkt),
    MAKE_FUNC_INFO(krb_get_tf_realm),
    MAKE_FUNC_INFO(krb_mk_req),
    MAKE_FUNC_INFO(krb_realmofhost),
    MAKE_FUNC_INFO(tf_init),
    MAKE_FUNC_INFO(tf_close),
    MAKE_FUNC_INFO(tf_get_cred),
    MAKE_FUNC_INFO(tf_get_pname),
    MAKE_FUNC_INFO(tf_get_pinst),
    MAKE_FUNC_INFO(LocalHostAddr),
    MAKE_FUNC_INFO(tkt_string),
    MAKE_FUNC_INFO(krb_set_tkt_string),
    MAKE_FUNC_INFO(initialize_krb_error_func),
    MAKE_FUNC_INFO(initialize_kadm_error_table),
    MAKE_FUNC_INFO(dest_tkt),
    MAKE_FUNC_INFO(lsh_LoadKrb4LeashErrorTables), // XXX
    END_FUNC_INFO
};

FUNC_INFO k5_fi[] = {
    MAKE_FUNC_INFO(krb5_change_password),
    MAKE_FUNC_INFO(krb5_get_init_creds_opt_init),
    MAKE_FUNC_INFO(krb5_get_init_creds_opt_set_tkt_life),
    MAKE_FUNC_INFO(krb5_get_init_creds_opt_set_renew_life),
    MAKE_FUNC_INFO(krb5_get_init_creds_opt_set_forwardable),
    MAKE_FUNC_INFO(krb5_get_init_creds_opt_set_proxiable),
    MAKE_FUNC_INFO(krb5_get_init_creds_password),
    MAKE_FUNC_INFO(krb5_build_principal_ext),
    MAKE_FUNC_INFO(krb5_cc_resolve),
    MAKE_FUNC_INFO(krb5_cc_default),
    MAKE_FUNC_INFO(krb5_cc_default_name),
    MAKE_FUNC_INFO(krb5_cc_set_default_name),
    MAKE_FUNC_INFO(krb5_free_context),
    MAKE_FUNC_INFO(krb5_free_cred_contents),
    MAKE_FUNC_INFO(krb5_free_principal),
    MAKE_FUNC_INFO(krb5_get_in_tkt_with_password),
    MAKE_FUNC_INFO(krb5_init_context),
    MAKE_FUNC_INFO(krb5_parse_name),
    MAKE_FUNC_INFO(krb5_timeofday),
    MAKE_FUNC_INFO(krb5_timestamp_to_sfstring),
    MAKE_FUNC_INFO(krb5_unparse_name),
    MAKE_FUNC_INFO(krb5_get_credentials),
    MAKE_FUNC_INFO(krb5_mk_req),
    MAKE_FUNC_INFO(krb5_sname_to_principal),
    MAKE_FUNC_INFO(krb5_get_credentials_renew),
    MAKE_FUNC_INFO(krb5_free_data),
    MAKE_FUNC_INFO(krb5_free_data_contents),
    MAKE_FUNC_INFO(krb5_get_realm_domain),
    MAKE_FUNC_INFO(krb5_free_unparsed_name),
    END_FUNC_INFO
};

FUNC_INFO ce_fi[] = {
    MAKE_FUNC_INFO(com_err),
    MAKE_FUNC_INFO(error_message),
    END_FUNC_INFO
};

FUNC_INFO afst_fi[] = {
    MAKE_FUNC_INFO(ktc_ListTokens),
    MAKE_FUNC_INFO(ktc_GetToken),
    MAKE_FUNC_INFO(ktc_SetToken),
    MAKE_FUNC_INFO(ktc_ForgetAllTokens),
    END_FUNC_INFO
};

FUNC_INFO afsc_fi[] = {
    MAKE_FUNC_INFO(cm_GetRootCellName),
    MAKE_FUNC_INFO(cm_SearchCellFile),
    END_FUNC_INFO
};

FUNC_INFO service_fi[] = {
    MAKE_FUNC_INFO(OpenSCManagerA),
    MAKE_FUNC_INFO(OpenServiceA),
    MAKE_FUNC_INFO(QueryServiceStatus),
    MAKE_FUNC_INFO(CloseServiceHandle),
    END_FUNC_INFO
};

HINSTANCE hAfsTokens = 0;
HINSTANCE hAfsConf = 0;
HINSTANCE hComErr = 0;
HINSTANCE hService = 0;

BOOL WINAPI
DllMain(
    HANDLE hinstDLL,
    DWORD fdwReason,
    LPVOID lpReserved
    )
{
    WNDCLASS	wndclass;

    hLeashInst = hinstDLL;

    switch (fdwReason) 
    {
    case DLL_PROCESS_ATTACH:
    {
        LoadFuncs(KRB4_DLL, k4_fi, &hKrb4, 0, 1, 0, 0);
        LoadFuncs(KRB5_DLL, k5_fi, &hKrb5, 0, 1, 0, 0);
        if (LoadFuncs(AFSTOKENS_DLL, afst_fi, &hAfsTokens, 0, 1, 0, 0))
            if (!LoadFuncs(AFSCONF_DLL, afsc_fi, &hAfsConf, 0, 1, 0, 0))
                UnloadFuncs(afst_fi, hAfsTokens);
        LoadFuncs(COMERR_DLL, ce_fi, &hComErr, 0, 0, 1, 0);
        LoadFuncs(SERVICE_DLL, service_fi, &hService, 0, 1, 0, 0);

        /*
         * Register window class for the MITPasswordControl that
	 * replaces normal edit controls for password input.
	 * zero any fields we don't explicitly set
         */
        hLeashInst = hinstDLL;
        if (plsh_LoadKrb4LeashErrorTables)
            plsh_LoadKrb4LeashErrorTables(hLeashInst, 0);


        memset(&wndclass, 0, sizeof(WNDCLASS));

        wndclass.style = CS_HREDRAW | CS_VREDRAW;
        wndclass.lpfnWndProc = (WNDPROC)MITPwdWinProcDLL;
        wndclass.cbClsExtra = sizeof(HWND);
        wndclass.cbWndExtra = 0;
        wndclass.hInstance = hinstDLL;
        wndclass.hbrBackground = (void *)(COLOR_WINDOW + 1);
        wndclass.lpszClassName = MIT_PWD_DLL_CLASS;
        wndclass.hCursor = LoadCursor((HINSTANCE)NULL, IDC_IBEAM);
	
        RegisterClass(&wndclass);

        return 1;  
    }
    case DLL_PROCESS_DETACH:
        if (hKrb4)
            FreeLibrary(hKrb4);
        if (hKrb5)
            FreeLibrary(hKrb5);
        if (hAfsTokens)
            FreeLibrary(hAfsTokens);
        if (hAfsConf)
            FreeLibrary(hAfsConf);
        if (hComErr)
            FreeLibrary(hComErr);
        if (hService)
            FreeLibrary(hService);
        return 1;
    default:
        return 1;
    }
}
