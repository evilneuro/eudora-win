#ifndef __QC_SECFAIL_H__
#define __QC_SECFAIL_H__

#ifdef __cplusplus
extern "C"{
#endif 

typedef void (__cdecl * _qc_secerr_handler_func)();

__declspec(dllexport) void __cdecl __qc_security_error_handler();
__declspec(dllexport) _qc_secerr_handler_func __cdecl _qc_set_security_error_handler(_qc_secerr_handler_func handler);

#ifdef __cplusplus
}
#endif

#endif		//	__QC_SECFAIL_H__
