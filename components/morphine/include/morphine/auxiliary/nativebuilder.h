//
// Created by whyiskra on 11.12.23.
//

#pragma once

#define nb_function(s) morphine_state_t _nb_state = (s); switch (mapi_callstate(_nb_state)) {
#define nb_init case (0): { mapi_continue(_nb_state, 1);
#define nb_state(s) goto _nb_end_leave; } case (s): { mapi_continue(_nb_state, (s+1));
#define nb_state_custom_direction(s, n) goto _nb_end_leave; } case (s): { morphine_continue(_nb_state, (n));
#define nb_end }} mapi_errorf(_nb_state, "Undefined state"); _nb_end_leave: return;

#define nb_continue(s) do { mapi_continue(_nb_state, (s)); goto _nb_end_leave; } while(0)
#define nb_leave() do { mapi_leave(_nb_state); goto _nb_end_leave; } while(0)
#define nb_return(s) do { { s; } mapi_return(_nb_state); goto _nb_end_leave; } while(0)
