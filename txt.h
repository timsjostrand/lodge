#ifndef _TXT_H
#define _TXT_H

#include <stdint.h>

#include "str.h"

typedef char * txt_t;

txt_t		txt_new(const strview_t str);
void		txt_free(txt_t txt);

txt_t		txt_reserve(txt_t txt, size_t new_size);
txt_t		txt_grow(txt_t txt, size_t fit);

size_t		txt_count(txt_t txt);

txt_t		txt_insert(txt_t txt, size_t index, const strview_t sub);
void		txt_delete(txt_t txt, size_t index, size_t count);
void		txt_delete_from_tail(txt_t txt, size_t count);

void		txt_trim(txt_t txt);
int			txt_begins_with(txt_t txt, const strview_t sub);
int			txt_ends_with(txt_t txt, const strview_t sub);

strview_t	txt_to_strview(txt_t txt);

#endif
