//
// Created by why-iskra on 09.04.2024.
//

#include "morphine/misc/uuid.h"

bool uuidI_equal(struct uuid a, struct uuid b) {
    return (a.most_significant_bits == b.most_significant_bits) &&
           (a.least_significant_bits == b.least_significant_bits);
}
