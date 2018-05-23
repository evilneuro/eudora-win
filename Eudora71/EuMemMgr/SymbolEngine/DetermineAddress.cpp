/*----------------------------------------------------------------------
Debugging Applications for Microsoft .NET and Microsoft Windows
Copyright © 1997-2003 John Robbins -- All rights reserved.
----------------------------------------------------------------------*/
#include "stdafx.h"
#include "SymbolEngine.h"
#include "SymEngInternal.h"


BOOL CSymbolEngine :: DetermineAddress ( CVariableInfo & cVar )
{
    BOOL bRet = TRUE ;
    
    // If the flags field is zero, the address field is already filled
    // out so I can get out now.
    if ( 0 == cVar.ulFlags )
    {
        return ( TRUE ) ;
    }
    
#ifdef _M_IX86
    // I'm not sure why, but StackWalk puts the NEXT
    // up the stack EBP, ESP, and EIP in the context,
    // not the current one.  Whatever.
    // I think StackWalk is using it as temporary
    // storage.
    CONTEXT stCtx ;

    if ( NULL != m_pContext )
    {
        stCtx = *m_pContext ;
    }
    else
    {
        ZeroMemory ( &stCtx , sizeof ( CONTEXT ) ) ;
    }

    stCtx.Ebp = (DWORD)m_pFrame->AddrFrame.Offset ;
    stCtx.Esp = (DWORD)m_pFrame->AddrStack.Offset ;
    stCtx.Eip = (DWORD)m_pFrame->AddrPC.Offset ;
#endif
    
    if ( IMAGEHLP_SYMBOL_INFO_REGISTER           ==
              ( cVar.ulFlags & IMAGEHLP_SYMBOL_INFO_REGISTER      ) )
    {
#ifdef _M_IX86
        bRet = ConvertRegisterToValueIA32 ( (CV_HREG_e)cVar.ulRegister ,
                                            cVar.dwAddress             ,
                                            &stCtx                    );
#else
        bRet = ConvertRegisterToValueIA64 ( (CV_HREG_e)cVar.ulRegister ,
                                            cVar.dwAddress             ,
                                            &stCtx                    );
#endif  // _M_IX86
                                        
    }
    else if ( IMAGEHLP_SYMBOL_INFO_REGRELATIVE   ==
              ( cVar.ulFlags & IMAGEHLP_SYMBOL_INFO_REGRELATIVE   ) )
    {
        DWORD64 dwRegVal = 0 ;
#ifdef _M_IX86
        bRet = ConvertRegisterToValueIA32 ( (CV_HREG_e)cVar.ulRegister ,
                                            dwRegVal                   ,
                                            &stCtx                    );
#else
        bRet = ConvertRegisterToValueIA64 ( (CV_HREG_e)cVar.ulRegister ,
                                            dwRegVal                   ,
                                            &stCtx                    );
#endif  // _M_IX86
        cVar.dwAddress = dwRegVal + cVar.dwAddress ;
    }
    else if ( IMAGEHLP_SYMBOL_INFO_FRAMERELATIVE ==
              ( cVar.ulFlags & IMAGEHLP_SYMBOL_INFO_FRAMERELATIVE ) )
    {
        // This looks like an interesting flag, but I have never seen
        // it used.  I gotta keep an eye on it.
        ASSERT ( FALSE ) ;
    }
    return ( bRet ) ;
}

#ifdef _M_IX86
BOOL CSymbolEngine :: ConvertRegisterToValueIA32 ( CV_HREG_e eReg     ,
                                                   DWORD64 & dwVal    ,
                                                   CONTEXT * pContext  )
{
    BOOL bRet = TRUE ;
    switch ( eReg )
    {
        case CV_REG_NONE    :
            bRet = FALSE ;
            break ;
        case CV_REG_AL      :
            dwVal = pContext->Eax & 0x000000FF ;
            break ;
        case CV_REG_CL      :
            dwVal = pContext->Ecx & 0x000000FF ;
            break ;
        case CV_REG_DL      :
            dwVal = pContext->Edx & 0x000000FF ;
            break ;
        case CV_REG_BL      :
            dwVal = pContext->Ebx & 0x000000FF ;
            break ;
        case CV_REG_AH      :
            dwVal = (pContext->Eax & 0x0000FF00) >> 8 ;
            break ;
        case CV_REG_CH      :
            dwVal = (pContext->Ecx & 0x0000FF00) >> 8 ;
            break ;
        case CV_REG_DH      :
            dwVal = (pContext->Edx & 0x0000FF00) >> 8 ;
            break ;
        case CV_REG_BH      :
            dwVal = (pContext->Ebx & 0x0000FF00) >> 8 ;
            break ;
        case CV_REG_AX      :
            dwVal = pContext->Eax & 0x0000FFFF ;
            break ;
        case CV_REG_CX      :
            dwVal = pContext->Ecx & 0x0000FFFF ;
            break ;
        case CV_REG_DX      :
            dwVal = pContext->Edx & 0x0000FFFF ;
            break ;
        case CV_REG_BX      :
            dwVal = pContext->Ebx & 0x0000FFFF ;
            break ;
        case CV_REG_SP      :
            dwVal = pContext->Esp & 0x0000FFFF ;
            break ;
        case CV_REG_BP      :
            dwVal = pContext->Ebp & 0x0000FFFF ;
            break ;
        case CV_REG_SI      :
            dwVal = pContext->Esi & 0x0000FFFF ;
            break ;
        case CV_REG_DI      :
            dwVal = pContext->Edi & 0x0000FFFF ;
            break ;
        case CV_REG_EAX     :
            dwVal = pContext->Eax ;
            break ;
        case CV_REG_ECX     :
            dwVal = pContext->Ecx ;
           break ;
        case CV_REG_EDX     :
            dwVal = pContext->Edx ;
            break ;
        case CV_REG_EBX     :
            dwVal = pContext->Ebx ;
            break ;
        case CV_REG_ESP     :
            dwVal = pContext->Esp ;
            break ;
        case CV_REG_EBP     :
            dwVal = pContext->Ebp ;
            break ;
        case CV_REG_ESI     :
            dwVal = pContext->Esi ;
            break ;
        case CV_REG_EDI     :
            dwVal = pContext->Edi ;
            break ;
        case CV_REG_ES      :
            dwVal = pContext->SegEs ;
            break ;
        case CV_REG_CS      :
            dwVal = pContext->SegCs ;
            break ;
        case CV_REG_SS      :
            dwVal = pContext->SegSs ;
            break ;
        case CV_REG_DS      :
            dwVal = pContext->SegDs ;
            break ;
        case CV_REG_FS      :
            dwVal = pContext->SegFs ;
            break ;
        case CV_REG_GS      :
            dwVal = pContext->SegGs ;
            break ;
        case CV_REG_IP      :
            dwVal = pContext->Eip & 0x000000FF ;
            break ;
        case CV_REG_FLAGS   :
            dwVal = pContext->EFlags & 0x000000FF ;
            break ;
        case CV_REG_EIP     :
            dwVal = pContext->Eip ;
            break ;
        case CV_REG_EFLAGS  :
            dwVal = pContext->EFlags ;
            break ;
        default :
            ASSERT ( !"Invalid register value!" ) ;
            bRet = FALSE ;
            break ;
    }
    return ( bRet ) ;
}
#else
BOOL CSymbolEngine :: ConvertRegisterToValueIA64 ( CV_HREG_e eReg     ,
                                                   DWORD64 & dwVal    ,
                                                   CONTEXT * pContext  )
{
    ASSERT ( FALSE ) ;
    eReg = eReg ;
    dwVal = dwVal ;
    pContext = pContext ;
    return ( FALSE ) ;
}
#endif  // _M_IX86