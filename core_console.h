#ifndef _CORE_CONSOLE_H
#define _CORE_CONSOLE_H

struct console;
struct env;

void	core_console_new(struct console *c, struct env *env);
void	core_console_printf(const char *fmt, ...);
#if 0
void	core_console_free(struct core_console *cmds);
#endif

#endif
