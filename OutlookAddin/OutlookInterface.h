/*
 * Copyright (c) 2003-2007, Bicom Systems Ltd.
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   - Redistributions of source code must retain the above copyright notice,
 *	   this list of conditions and the following disclaimer.
 *   - Redistributions in binary form must reproduce the above copyright
 *	   notice, this list of conditions and the following disclaimer in the
 *	   documentation and/or other materials provided with the distribution.
 *   - Neither the name of the Bicom Systems Ltd nor the names of its
 *	   contributors may be used to endorse or promote products derived from this
 *	   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 * 
 * Denis Komadaric,
 * Bicom Systems Ltd.
 */

#ifndef _OUTLOOKINTERFACE_H
#define _OUTLOOKINTERFACE_H

#if defined(WIN32) && !defined(MAC)
#ifndef __IOutlookExtCallback_FWD_DEFINED__
#define __IOutlookExtCallback_FWD_DEFINED__
typedef interface IOutlookExtCallback IOutlookExtCallback;
#endif    /* __IOutlookExtCallback_FWD_DEFINED__ */ 

// Outlook defines this interface as an alternate to IExchExtCallback.
#ifndef __IOutlookExtCallback_INTERFACE_DEFINED__
#define __IOutlookExtCallback_INTERFACE_DEFINED__

EXTERN_C const IID IID_IOutlookExtCallback;
    interface DECLSPEC_UUID("0006720D-0000-0000-C000-000000000046")
    IOutlookExtCallback : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetObject(
            /* [out] */ IUnknown __RPC_FAR *__RPC_FAR *ppunk) = 0;
        virtual HRESULT STDMETHODCALLTYPE GetOfficeCharacter(
            /* [out] */ void __RPC_FAR *__RPC_FAR *ppmsotfc) = 0;
    };

DEFINE_GUID(IID_IOutlookExtCallback,
    0x0006720d,
    0x0000,
    0x0000,
    0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46);

#endif    /* __IOutlookExtCallback_INTERFACE_DEFINED__ */ 
#endif // defined(WIN32) && !defined(MAC)
#endif // _OUTLOOKINTERFACE_H