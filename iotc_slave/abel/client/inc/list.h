/*
 * Doubly-linked list
 * Copyright (c) 2009, Jouni Malinen <j@w1.fi>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Alternatively, this software may be distributed under the terms of BSD
 * license.
 *
 * See README and COPYING for more details.
 */

#ifndef LIST_H
#define LIST_H

#ifndef NULL
#define NULL  ((void *)0)
#endif

/**
 * struct dl_list - Doubly-linked list
 */
struct dl_list
{
	struct dl_list *next;
	struct dl_list *prev;
};

void dl_list_init ( struct dl_list *list );
void dl_list_add ( struct dl_list *list, struct dl_list *item );
void dl_list_add_tail ( struct dl_list *list, struct dl_list *item );
void dl_list_del ( struct dl_list *item );
int dl_list_empty ( struct dl_list *list );
unsigned int dl_list_len ( struct dl_list *list );

#ifndef offsetof
#define offsetof(type, member) ((long) &((type *) 0)->member)
#endif

#define dl_list_entry(item, type, member) \
	((type *) ((char *) item - offsetof(type, member)))

#define dl_list_first(list, type, member) \
	(dl_list_empty((list)) ? NULL : \
	 dl_list_entry((list)->next, type, member))

#define dl_list_last(list, type, member) \
	(dl_list_empty((list)) ? NULL : \
	 dl_list_entry((list)->prev, type, member))


#define dl_list_for_each(item, list, type, member) \
	for (item = dl_list_entry((list)->next, type, member); \
			&item->member != (list); \
			item = dl_list_entry(item->member.next, type, member))

#define dl_list_for_each_safe(item, n, list, type, member) \
	for (item = dl_list_entry((list)->next, type, member), \
			n = dl_list_entry(item->member.next, type, member); \
			&item->member != (list); \
			item = n, n = dl_list_entry(n->member.next, type, member))

#define dl_list_for_each_reverse(item, list, type, member) \
	for (item = dl_list_entry((list)->prev, type, member); \
			&item->member != (list); \
			item = dl_list_entry(item->member.prev, type, member))

#define DEFINE_DL_LIST(name) \
	struct dl_list name = { &(name), &(name) }

#endif /* LIST_H */
