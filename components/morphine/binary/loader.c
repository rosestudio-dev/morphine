//
// Created by why-iskra on 31.03.2024.
//

#include "morphine/binary/loader.h"
#include "loader/process.h"
#include "loader/binary.h"

struct proto *loaderI_load(
    morphine_state_t S,
    morphine_loader_init_t init,
    morphine_loader_read_t read,
    morphine_loader_finish_t finish,
    void *args
) {
    return process(S, init, read, finish, args, binary);
}
