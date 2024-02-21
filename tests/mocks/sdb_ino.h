#pragma once
// Mocks generic methods available only in the project's root sdb.ino

#include "common.h"

void sdbPanic(char* msg) {
    ERROR_PRINTF( ("[SDB] PANIC!\n" ) );
}
