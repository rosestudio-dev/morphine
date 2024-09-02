//
// Created by why-iskra on 02.09.2024.
//

#include <morphinel.h>

MORPHINE_API void mlapi_import_all(morphine_instance_t I) {
    (void) I;

#ifdef MORPHINEL_USE_MATH
    mapi_library_load(I, mllib_math());
#endif

#ifdef MORPHINEL_USE_FS
    mapi_library_load(I, mllib_fs());
#endif

#ifdef MORPHINEL_USE_SYSTEM
    mapi_library_load(I, mllib_system());
#endif

#ifdef MORPHINEL_USE_BIGINT
    mapi_library_load(I, mllib_bigint());
#endif
}
