//
// Created by whyiskra on 09.12.23.
//

#pragma once

#include <stddef.h>
#include <stdint.h>

struct crc32_buf {
    uint32_t crc;
};

struct crc32_buf crc32_init(void);
uint32_t crc32_result(struct crc32_buf *impl);
void crc32_char(struct crc32_buf *impl, uint8_t c);
void crc32_string(struct crc32_buf *impl, uint8_t *buf, size_t len);
