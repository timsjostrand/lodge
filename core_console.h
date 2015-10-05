#ifndef _CORE_CONSOLE_H
#define _CORE_CONSOLE_H

struct console;
struct graphics;

void core_console_init(struct graphics *g, struct console *c);
void core_console_free();
void core_console_printf(const char *fmt, ...);

#endif
