//
// Created by whyiskra on 11.12.23.
//

#pragma once

#define maux_nb_function(U) morphine_coroutine_t _maux_nb_coro = (U); switch (mapi_callstate(_maux_nb_coro)) {
#define maux_nb_end }} mapi_errorf(_maux_nb_coro, "Undefined state"); _maux_nb_end_leave: return;

#define maux_nb_init case (0): { mapi_continue(_maux_nb_coro, 1);
#define maux_nb_state(s) goto _maux_nb_end_leave; } case (s): { mapi_continue(_maux_nb_coro, ((s)+1));
#define maux_nb_immediately_state(s) goto _maux_nb_end_leave; } case (s): { _maux_nb_immediately_##s: mapi_continue(_maux_nb_coro, ((s)+1));

#define maux_nb_immediately_continue(s) do { goto _maux_nb_immediately_##s; } while(0)
#define maux_nb_continue(s) do { mapi_continue(_maux_nb_coro, (s)); goto _maux_nb_end_leave; } while(0)
#define maux_nb_leave() do { mapi_leave(_maux_nb_coro); goto _maux_nb_end_leave; } while(0)
#define maux_nb_return(x) do { { x; } mapi_return(_maux_nb_coro); goto _maux_nb_end_leave; } while(0)
