#pragma once
#include "autolock.hxx"
#include <rpc.h>

typedef RPC_STATUS (RPC_ENTRY *FP_RpcBindingSetAuthInfoExA)(
    IN RPC_BINDING_HANDLE Binding,
    IN unsigned char __RPC_FAR * ServerPrincName,
    IN unsigned long AuthnLevel,
    IN unsigned long AuthnSvc,
    IN RPC_AUTH_IDENTITY_HANDLE AuthIdentity, OPTIONAL
    IN unsigned long AuthzSvc,
    IN RPC_SECURITY_QOS *SecurityQos OPTIONAL
    );

typedef RPC_STATUS (RPC_ENTRY *FP_RpcBindingSetAuthInfoExW)(
    IN RPC_BINDING_HANDLE Binding,
    IN unsigned short __RPC_FAR * ServerPrincName,
    IN unsigned long AuthnLevel,
    IN unsigned long AuthnSvc,
    IN RPC_AUTH_IDENTITY_HANDLE AuthIdentity, OPTIONAL
    IN unsigned long AuthzSvc, OPTIONAL
    IN RPC_SECURITY_QOS *SecurityQOS
    );

typedef RPC_STATUS (RPC_ENTRY *FP_RpcServerRegisterIfEx)(
    IN RPC_IF_HANDLE IfSpec,
    IN UUID __RPC_FAR * MgrTypeUuid,
    IN RPC_MGR_EPV __RPC_FAR * MgrEpv,
    IN unsigned int Flags,
    IN unsigned int MaxCalls,
    IN RPC_IF_CALLBACK_FN __RPC_FAR *IfCallback
    );

#ifdef UNICODE
#define FP_RpcBindingSetAuthInfoEx FP_RpcBindingSetAuthInfoExW
#define FN_RpcBindingSetAuthInfoEx "RpcBindingSetAuthInfoExW"
#else
#define FP_RpcBindingSetAuthInfoEx FP_RpcBindingSetAuthInfoExA
#define FN_RpcBindingSetAuthInfoEx "RpcBindingSetAuthInfoExA"
#endif

#define FN_RpcServerRegisterIfEx   "RpcServerRegisterIfEx"

class Init
{
public:
    struct InitInfo {
        BOOL isNT;
        FP_RpcBindingSetAuthInfoEx fRpcBindingSetAuthInfoEx;
        FP_RpcServerRegisterIfEx fRpcServerRegisterIfEx;
    };

    static DWORD Initialize();
    static DWORD Cleanup();
    static DWORD Info(InitInfo& info);

    static bool Initialized() { return s_init; }

private:
    static CcOsLock s_lock;
    static DWORD s_refcount;
    static DWORD s_error;
    static bool s_init;
    static InitInfo s_info;
    static HINSTANCE s_hRpcDll;
};

#define INIT_INIT_EX(trap, error) \
do \
{ \
    if (!Init::Initialized()) \
    { \
        DWORD rc = Init::Initialize(); \
        if (rc) return (trap) ? (error) : rc; \
    } \
} while(0)
