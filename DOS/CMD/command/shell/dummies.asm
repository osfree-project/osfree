; $Id: dummies.asm 364 2002-04-02 18:13:06Z skaus $

; Dummy drivers to be included into FreeCOM itself

include model.inc

_TEXT segment word public 'CODE' use16

        public dummy_criter_handler_
        public end_dummy_criter_handler_
dummy_criter_handler_:
        mov al, 3                       ; always fail
        iret
end_dummy_criter_handler_:

_TEXT ends

      end
