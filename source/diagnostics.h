#pragma once

void diagnostics_init(void);
int diagnostics_enabled(void);
__attribute__((format(printf, 1, 2)))
void diagnostics_log(const char* fmt, ...);
