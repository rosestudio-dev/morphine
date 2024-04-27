//
// Created by whyiskra on 11.12.23.
//

#pragma once

#define nb_function(U) morphine_coroutine_t _nb_coro = (U); switch (mapi_callstate(_nb_coro)) {
#define nb_init case (0): { mapi_continue(_nb_coro, 1);
#define nb_state(s) goto _nb_end_leave; } case (s): { mapi_continue(_nb_coro, ((s)+1));
#define nb_immediately_state(s) goto _nb_end_leave; } case (s): { _nb_immediately_##s: mapi_continue(_nb_coro, ((s)+1));
#define nb_end }} mapi_errorf(_nb_coro, "Undefined state"); _nb_end_leave: return;

#define nb_immediately_continue(s) do { goto _nb_immediately_##s; } while(0)
#define nb_continue(s) do { mapi_continue(_nb_coro, (s)); goto _nb_end_leave; } while(0)
#define nb_leave() do { mapi_leave(_nb_coro); goto _nb_end_leave; } while(0)
#define nb_return(x) do { { x; } mapi_return(_nb_coro); goto _nb_end_leave; } while(0)
