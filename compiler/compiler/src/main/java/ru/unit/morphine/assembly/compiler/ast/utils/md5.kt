package ru.unit.morphine.assembly.compiler.ast.utils

import java.security.MessageDigest

fun String.md5() = MessageDigest
    .getInstance("MD5")
    .digest(this.toByteArray(Charsets.UTF_8))
    .joinToString(separator = "") { byte ->
        "%02x".format(byte)
    }