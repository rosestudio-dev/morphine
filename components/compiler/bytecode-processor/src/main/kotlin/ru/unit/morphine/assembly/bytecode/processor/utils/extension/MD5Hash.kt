package ru.unit.orchestra_features.processor.utils.extension

import java.security.MessageDigest

fun String.md5() = MessageDigest
    .getInstance("MD5")
    .digest(this.toByteArray(Charsets.UTF_8))
    .joinToString(separator = "") { byte ->
        "%02x".format(byte)
    }