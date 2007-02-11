/*
 * formatterbase.c - Define a modular system for formatting filesystems
 *
 * Copyright 2007 Riccardo Setti <giskard@autistici.org>
 * 		  Paul Betts <paul.betts@gmail.com>
 *
 *
 * License:
 *
 * This package is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This package is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this package; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#include <stdio.h>
#include <string.h>

#include <glib.h>
#include <glib/gi18n.h>
#include "formatterbase.h"

gboolean
formatter_can_format(GSList* formatter_list, const char* fs, const char* blockdev)
{
	GSList* iter = formatter_list;
	for(iter = formatter_list; iter != NULL; iter = g_slist_next(iter)) {
		Formatter* current = iter->data;
		if( !current->fops.can_format )
			continue;

		if( *(current->fops.can_format)(formatter_list, fs, blockdev))
			return TRUE;
	}
	return FALSE;
}

gboolean
formatter_do_format(GSList* formatter_list, const char* fs, GHashTable* options, GError** error)
{
	GSList* iter = formatter_list;
	for(iter = formatter_list; iter != NULL; iter = g_slist_next(iter)) {
		Formatter* current = iter->data;
		if( !current->fops.can_format ||
		    !*(current->fops.can_format)(formatter_list, fs, blockdev) )
			continue;
		g_assert(current->fops.do_format != NULL);
		return *(current->fops.do_format)(formatter_list, fs, options, error);
	}

	return FALSE;
}

void formatter_list_free(GSList* formatter_list)
{
	GSList* iter = formatter_list;
	for(iter = formatter_list; iter != NULL; iter = g_slist_next(iter)) {
		Formatter* current = iter->data;
		if(current->fops.unref)
			*(current->fops.unref)();
		g_free(current);
	}
	g_slist_remove_all(formatter_list);
}
