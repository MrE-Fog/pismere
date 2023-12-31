#include <windows.h>
#include "leashdll.h"
#include <krb.h>
#include <leashwin.h>
#include "leash-int.h"

HINSTANCE hLeashInst;

#ifndef NO_KRB4
HINSTANCE hKrb4 = 0;
#endif
HINSTANCE hKrb5 = 0;
HINSTANCE hKrb524 = 0;
HINSTANCE hSecur32 = 0;
HINSTANCE hComErr = 0;
HINSTANCE hService = 0;
HINSTANCE hProfile = 0;
HINSTANCE hPsapi = 0;
HINSTANCE hToolHelp32 = 0;
HINSTANCE hCcapi = 0;

DWORD     AfsAvailable = 0;

#ifndef NO_KRB4
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
DECL_FUNC_PTR(krb_in_tkt);
DECL_FUNC_PTR(krb_save_credentials);
DECL_FUNC_PTR(krb_get_krbconf2);
DECL_FUNC_PTR(krb_get_krbrealm2);
DECL_FUNC_PTR(krb_life_to_time);
#endif

// krb5 functions
DECL_FUNC_PTR(krb5_change_password);
DECL_FUNC_PTR(krb5_get_init_creds_opt_init);
DECL_FUNC_PTR(krb5_get_init_creds_opt_set_tkt_life);
DECL_FUNC_PTR(krb5_get_init_creds_opt_set_renew_life);
DECL_FUNC_PTR(krb5_get_init_creds_opt_set_forwardable);
DECL_FUNC_PTR(krb5_get_init_creds_opt_set_proxiable);
DECL_FUNC_PTR(krb5_get_init_creds_opt_set_address_list);
DECL_FUNC_PTR(krb5_get_init_creds_password);
DECL_FUNC_PTR(krb5_build_principal_ext);
DECL_FUNC_PTR(krb5_cc_resolve);
DECL_FUNC_PTR(krb5_cc_default);
DECL_FUNC_PTR(krb5_cc_default_name);
DECL_FUNC_PTR(krb5_cc_set_default_name);
DECL_FUNC_PTR(krb5_cc_initialize);
DECL_FUNC_PTR(krb5_cc_destroy);
DECL_FUNC_PTR(krb5_cc_close);
DECL_FUNC_PTR(krb5_cc_store_cred);
DECL_FUNC_PTR(krb5_cc_copy_creds);
// DECL_FUNC_PTR(krb5_cc_retrieve_cred);
DECL_FUNC_PTR(krb5_cc_get_principal);
DECL_FUNC_PTR(krb5_cc_start_seq_get);
DECL_FUNC_PTR(krb5_cc_next_cred);
DECL_FUNC_PTR(krb5_cc_end_seq_get);
// DECL_FUNC_PTR(krb5_cc_remove_cred);
DECL_FUNC_PTR(krb5_cc_set_flags);
// DECL_FUNC_PTR(krb5_cc_get_type);
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
// DECL_FUNC_PTR(krb5_get_realm_domain);
DECL_FUNC_PTR(krb5_free_unparsed_name);
DECL_FUNC_PTR(krb5_os_localaddr);
DECL_FUNC_PTR(krb5_copy_keyblock_contents);
DECL_FUNC_PTR(krb5_copy_data);
DECL_FUNC_PTR(krb5_free_creds);
DECL_FUNC_PTR(krb5_build_principal);
DECL_FUNC_PTR(krb5_get_renewed_creds);
DECL_FUNC_PTR(krb5_get_default_config_files);
DECL_FUNC_PTR(krb5_free_config_files);
DECL_FUNC_PTR(krb5_get_default_realm);
DECL_FUNC_PTR(krb5_free_ticket);
DECL_FUNC_PTR(krb5_decode_ticket);
DECL_FUNC_PTR(krb5_get_host_realm);
DECL_FUNC_PTR(krb5_free_host_realm);
DECL_FUNC_PTR(krb5_c_random_make_octets);
DECL_FUNC_PTR(krb5_free_addresses);
DECL_FUNC_PTR(krb5_free_default_realm);
DECL_FUNC_PTR(krb5_principal_compare);
DECL_FUNC_PTR(krb5_string_to_deltat);

#ifndef NO_KRB4
// Krb524 functions
DECL_FUNC_PTR(krb524_init_ets);
DECL_FUNC_PTR(krb524_convert_creds_kdc);
#endif

// ComErr functions
DECL_FUNC_PTR(com_err);
DECL_FUNC_PTR(error_message);

// Profile functions
DECL_FUNC_PTR(profile_init);
DECL_FUNC_PTR(profile_release);
DECL_FUNC_PTR(profile_get_subsection_names);
DECL_FUNC_PTR(profile_free_list);
DECL_FUNC_PTR(profile_get_string);
DECL_FUNC_PTR(profile_release_string);
DECL_FUNC_PTR(profile_get_integer);

// Service functions
DECL_FUNC_PTR(OpenSCManagerA);
DECL_FUNC_PTR(OpenServiceA);
DECL_FUNC_PTR(QueryServiceStatus);
DECL_FUNC_PTR(CloseServiceHandle);
DECL_FUNC_PTR(LsaNtStatusToWinError);

// LSA Functions
DECL_FUNC_PTR(LsaConnectUntrusted);
DECL_FUNC_PTR(LsaLookupAuthenticationPackage);
DECL_FUNC_PTR(LsaCallAuthenticationPackage);
DECL_FUNC_PTR(LsaFreeReturnBuffer);
DECL_FUNC_PTR(LsaGetLogonSessionData);

// CCAPI Functions
DECL_FUNC_PTR(cc_initialize);
DECL_FUNC_PTR(cc_shutdown);
DECL_FUNC_PTR(cc_get_NC_info);
DECL_FUNC_PTR(cc_free_NC_info);

#ifndef NO_KRB4
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
    MAKE_FUNC_INFO(krb_in_tkt),
    MAKE_FUNC_INFO(krb_save_credentials),
    MAKE_FUNC_INFO(krb_get_krbconf2),
    MAKE_FUNC_INFO(krb_get_krbrealm2),
    MAKE_FUNC_INFO(krb_life_to_time),
    END_FUNC_INFO
};
#endif

FUNC_INFO k5_fi[] = {
    MAKE_FUNC_INFO(krb5_change_password),
    MAKE_FUNC_INFO(krb5_get_init_creds_opt_init),
    MAKE_FUNC_INFO(krb5_get_init_creds_opt_set_tkt_life),
    MAKE_FUNC_INFO(krb5_get_init_creds_opt_set_renew_life),
    MAKE_FUNC_INFO(krb5_get_init_creds_opt_set_forwardable),
    MAKE_FUNC_INFO(krb5_get_init_creds_opt_set_proxiable),
    MAKE_FUNC_INFO(krb5_get_init_creds_opt_set_address_list),
    MAKE_FUNC_INFO(krb5_get_init_creds_password),
    MAKE_FUNC_INFO(krb5_build_principal_ext),
    MAKE_FUNC_INFO(krb5_cc_resolve),
    MAKE_FUNC_INFO(krb5_cc_default),
    MAKE_FUNC_INFO(krb5_cc_default_name),
    MAKE_FUNC_INFO(krb5_cc_set_default_name),
    MAKE_FUNC_INFO(krb5_cc_initialize),
    MAKE_FUNC_INFO(krb5_cc_destroy),
    MAKE_FUNC_INFO(krb5_cc_close),
    MAKE_FUNC_INFO(krb5_cc_copy_creds),
    MAKE_FUNC_INFO(krb5_cc_store_cred),
// MAKE_FUNC_INFO(krb5_cc_retrieve_cred),
    MAKE_FUNC_INFO(krb5_cc_get_principal),
    MAKE_FUNC_INFO(krb5_cc_start_seq_get),
    MAKE_FUNC_INFO(krb5_cc_next_cred),
    MAKE_FUNC_INFO(krb5_cc_end_seq_get),
// MAKE_FUNC_INFO(krb5_cc_remove_cred),
    MAKE_FUNC_INFO(krb5_cc_set_flags),
// MAKE_FUNC_INFO(krb5_cc_get_type),
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
//  MAKE_FUNC_INFO(krb5_get_realm_domain),
    MAKE_FUNC_INFO(krb5_free_unparsed_name),
    MAKE_FUNC_INFO(krb5_os_localaddr),
    MAKE_FUNC_INFO(krb5_copy_keyblock_contents),
    MAKE_FUNC_INFO(krb5_copy_data),
    MAKE_FUNC_INFO(krb5_free_creds),
    MAKE_FUNC_INFO(krb5_build_principal),
    MAKE_FUNC_INFO(krb5_get_renewed_creds),
    MAKE_FUNC_INFO(krb5_free_addresses),
    MAKE_FUNC_INFO(krb5_get_default_config_files),
    MAKE_FUNC_INFO(krb5_free_config_files),
    MAKE_FUNC_INFO(krb5_get_default_realm),
    MAKE_FUNC_INFO(krb5_free_ticket),
    MAKE_FUNC_INFO(krb5_decode_ticket),
    MAKE_FUNC_INFO(krb5_get_host_realm),
    MAKE_FUNC_INFO(krb5_free_host_realm),
    MAKE_FUNC_INFO(krb5_c_random_make_octets),
    MAKE_FUNC_INFO(krb5_free_default_realm),
    MAKE_FUNC_INFO(krb5_principal_compare),
    MAKE_FUNC_INFO(krb5_string_to_deltat),
    END_FUNC_INFO
};

#ifndef NO_KRB4
FUNC_INFO k524_fi[] = {
    MAKE_FUNC_INFO(krb524_init_ets),
    MAKE_FUNC_INFO(krb524_convert_creds_kdc),
    END_FUNC_INFO
};
#endif

FUNC_INFO profile_fi[] = {
    MAKE_FUNC_INFO(profile_init),
    MAKE_FUNC_INFO(profile_release),
    MAKE_FUNC_INFO(profile_get_subsection_names),
    MAKE_FUNC_INFO(profile_free_list),
    MAKE_FUNC_INFO(profile_get_string),
    MAKE_FUNC_INFO(profile_release_string),
    MAKE_FUNC_INFO(profile_get_integer),
    END_FUNC_INFO
};

FUNC_INFO ce_fi[] = {
    MAKE_FUNC_INFO(com_err),
    MAKE_FUNC_INFO(error_message),
    END_FUNC_INFO
};

FUNC_INFO service_fi[] = {
    MAKE_FUNC_INFO(OpenSCManagerA),
    MAKE_FUNC_INFO(OpenServiceA),
    MAKE_FUNC_INFO(QueryServiceStatus),
    MAKE_FUNC_INFO(CloseServiceHandle),
    MAKE_FUNC_INFO(LsaNtStatusToWinError),
    END_FUNC_INFO
};

FUNC_INFO lsa_fi[] = {
    MAKE_FUNC_INFO(LsaConnectUntrusted),
    MAKE_FUNC_INFO(LsaLookupAuthenticationPackage),
    MAKE_FUNC_INFO(LsaCallAuthenticationPackage),
    MAKE_FUNC_INFO(LsaFreeReturnBuffer),
    MAKE_FUNC_INFO(LsaGetLogonSessionData),
    END_FUNC_INFO
};

// CCAPI v2
FUNC_INFO ccapi_fi[] = {
    MAKE_FUNC_INFO(cc_initialize),
    MAKE_FUNC_INFO(cc_shutdown),
    MAKE_FUNC_INFO(cc_get_NC_info),
    MAKE_FUNC_INFO(cc_free_NC_info),
    END_FUNC_INFO
};

// psapi functions
DECL_FUNC_PTR(GetModuleFileNameExA);
DECL_FUNC_PTR(EnumProcessModules);

FUNC_INFO psapi_fi[] = {
    MAKE_FUNC_INFO(GetModuleFileNameExA),
    MAKE_FUNC_INFO(EnumProcessModules),
    END_FUNC_INFO
};

// toolhelp functions
DECL_FUNC_PTR(CreateToolhelp32Snapshot);
DECL_FUNC_PTR(Module32First);
DECL_FUNC_PTR(Module32Next);

FUNC_INFO toolhelp_fi[] = {
    MAKE_FUNC_INFO(CreateToolhelp32Snapshot),
    MAKE_FUNC_INFO(Module32First),
    MAKE_FUNC_INFO(Module32Next),
    END_FUNC_INFO
};

BOOL WINAPI
DllMain(
    HANDLE hinstDLL,
    DWORD fdwReason,
    LPVOID lpReserved
    )
{
    hLeashInst = hinstDLL;

    switch (fdwReason)
    {
    case DLL_PROCESS_ATTACH:
    {
        OSVERSIONINFO osvi;
#ifndef NO_KRB4
        LoadFuncs(KRB4_DLL, k4_fi, &hKrb4, 0, 1, 0, 0);
#endif
        LoadFuncs(KRB5_DLL, k5_fi, &hKrb5, 0, 1, 0, 0);
        LoadFuncs(COMERR_DLL, ce_fi, &hComErr, 0, 0, 1, 0);
        LoadFuncs(SERVICE_DLL, service_fi, &hService, 0, 1, 0, 0);
        LoadFuncs(SECUR32_DLL, lsa_fi, &hSecur32, 0, 1, 1, 1);
#ifndef NO_KRB4
        LoadFuncs(KRB524_DLL, k524_fi, &hKrb524, 0, 1, 1, 1);
#endif
	LoadFuncs(PROFILE_DLL, profile_fi, &hProfile, 0, 1, 0, 0);
	LoadFuncs(CCAPI_DLL, ccapi_fi, &hCcapi, 0, 1, 0, 0);

        memset(&osvi, 0, sizeof(OSVERSIONINFO));
        osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
        GetVersionEx(&osvi);

        // XXX: We should really use feature testing, first
        // checking for CreateToolhelp32Snapshot.  If that's
        // not around, we try the psapi stuff.
        //
        // Only load LSA functions if on NT/2000/XP
        if(osvi.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)
        {
            // Windows 9x
            LoadFuncs(TOOLHELPDLL, toolhelp_fi, &hToolHelp32, 0, 1, 0, 0);
            hPsapi = 0;
        }
        else if(osvi.dwPlatformId == VER_PLATFORM_WIN32_NT)
        {
            // Windows NT
            LoadFuncs(PSAPIDLL, psapi_fi, &hPsapi, 0, 1, 0, 0);
            hToolHelp32 = 0;
        }


        /*
         * Register window class for the MITPasswordControl that
         * replaces normal edit controls for password input.
         * zero any fields we don't explicitly set
         */
        hLeashInst = hinstDLL;
#ifndef NO_KRB4
        if (plsh_LoadKrb4LeashErrorTables)
            plsh_LoadKrb4LeashErrorTables(hLeashInst, 0);
#endif

        Register_MITPasswordEditControl(hLeashInst);

#ifndef NO_AFS
        {
            DWORD AfsStatus = 0;
            GetAfsStatus(&AfsStatus);

            AfsAvailable = afscompat_init();

            if ( AfsStatus && !AfsAvailable )
                SetAfsStatus(0);
        }
#endif
        return TRUE;
    }
    case DLL_PROCESS_DETACH:
#ifndef NO_AFS
        afscompat_close();
#endif
#ifndef NO_KRB4
        if (hKrb4)
            FreeLibrary(hKrb4);
#endif
        if (hKrb5)
            FreeLibrary(hKrb5);
	if (hCcapi)
	    FreeLibrary(hCcapi);
	if (hProfile)
	    FreeLibrary(hProfile);
        if (hComErr)
            FreeLibrary(hComErr);
        if (hService)
            FreeLibrary(hService);
        if (hSecur32)
            FreeLibrary(hSecur32);
#ifndef NO_KRB4
        if (hKrb524)
            FreeLibrary(hKrb524);
#endif
        if (hPsapi)
            FreeLibrary(hPsapi);
        if (hToolHelp32)
            FreeLibrary(hToolHelp32);

        return TRUE;
    default:
        return TRUE;
    }
}
