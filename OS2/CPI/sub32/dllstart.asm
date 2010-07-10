        name    dllstart
.386p
        assume  nothing

_TEXT   segment use32 word public 'CODE'

        public  __DLLstart_

        assume  cs:_TEXT

__DLLstart_ proc near
        xor eax, eax
        ret 8
__DLLstart_ endp

_TEXT   ends

        end     __DLLstart_
