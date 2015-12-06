#ifndef _CORE_CONSOLE_H
#define _CORE_CONSOLE_H

struct console;

void	core_console_new(struct console *c);
void	core_console_printf(const char *fmt, ...);
#if 0
void	core_console_free(struct core_console *cmds);
#endif

#endif
