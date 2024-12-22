//
// Created by whyiskra on 11.12.23.
//

#pragma once

#define maux_nb_function(U) morphine_coroutine_t _maux_nb_coro = (U); switch (mapi_callstate(_maux_nb_coro)) {
#define maux_nb_end         }} mapi_error(_maux_nb_coro, "undefined state"); _maux_nb_end_leave: morphine_unused; return;

#define maux_nb_init     case (0): { mapi_continue(_maux_nb_coro, 1);
#define maux_nb_state(s) goto _maux_nb_end_leave; } case (s): { _maux_nb_immediately_##s: morphine_unused; mapi_continue(_maux_nb_coro, (s) + 1);

#define maux_nb_im_continue(s) do { goto _maux_nb_immediately_##s; } while(0)
#define maux_nb_continue(s)    do { mapi_continue(_maux_nb_coro, (s)); goto _maux_nb_end_leave; } while(0)
#define maux_nb_leave()        do { mapi_leave(_maux_nb_coro); goto _maux_nb_end_leave; } while(0)
#define maux_nb_return(x)      do { { x; } mapi_return(_maux_nb_coro); goto _maux_nb_end_leave; } while(0)

#define maux_nb_operation(op, s) do { mapi_continue(_maux_nb_coro, (s)); if (mapi_op(_maux_nb_coro, (op))) { goto _maux_nb_end_leave; } mapi_result(_maux_nb_coro); maux_nb_im_continue(s); } while(0)
#define maux_nb_call(args, s)    do { mapi_continue(_maux_nb_coro, (s)); mapi_call(_maux_nb_coro, (args)); goto _maux_nb_end_leave; } while(0)
#define maux_nb_ecall(args, s)   do { mapi_continue(_maux_nb_coro, (s)); mapi_ecall(_maux_nb_coro, (args)); goto _maux_nb_end_leave; } while(0)
#define maux_nb_scall(args, s)   do { mapi_continue(_maux_nb_coro, (s)); mapi_scall(_maux_nb_coro, (args)); goto _maux_nb_end_leave; } while(0)
#define maux_nb_escall(args, s)  do { mapi_continue(_maux_nb_coro, (s)); mapi_escall(_maux_nb_coro, (args)); goto _maux_nb_end_leave; } while(0)
