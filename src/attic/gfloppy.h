/* gfloppy.h
 *
 * Copyright (C) 1999 Red Hat, Inc.
 *
 * This program is free software; you can redistribute it and/or 
 * modify it under the terms of the GNU General Public License as 
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 */



#ifndef __GFLOPPY_H__
#define __GFLOPPY_H__

#include <glib.h>

typedef enum {
	GFLOPPY_E2FS = 0,
	GFLOPPY_FAT  = 1
} GFloppyType;

typedef enum {
	GFLOPPY_144M = 0,
	GFLOPPY_12M  = 1,
	GFLOPPY_720K = 2,
	GFLOPPY_360K = 3
} GFloppySize;

typedef enum {
	GFLOPPY_FORMAT_QUICK = 0,
	GFLOPPY_FORMAT_STANDARD = 1,
	GFLOPPY_FORMAT_THOROUGH = 2,
	GFLOPPY_FORMAT_END = 3
} GFloppyFormattingMode;

typedef struct _GFloppy GFloppy;
struct _GFloppy {
	GFloppyType type;
	GFloppySize size;
	GFloppyFormattingMode formatting_mode;

	gint nblocks;      /* total number of 512 byte blocks */
	gint nbadblocks;       /* number of bad blocks found during scan
				  (only available in the parent process) */
	GList *badblocks_list; /* list of the bad blocks found during scan
				  (only available in the child process) */

	gboolean mkdosfs_backend; /* If true, the mkdosfs FAT formatting
				     will be used */
	char *volume_name;     /* the label to be assigned to the disk */

	char *device;          /* ie. /dev/fd0, /dev/fd0H1440 or /dev/floppy/0 (devfs) */
	char *mdevice;         /* ie. a: or b: */

        /* child process which does actual work */
	gint pid;

	/* gtk input handler id */
	gint handler_id;

	/* Protocol for message pipe.
	 * B<number>  - Inform the main process that the block <number> is corrupted
	 * E<message> - Spawn an error dialog with <message> as the message
	 * M<message> - Puts <message> in the progress dialog.
	 * P###       - Update the progress bar with ### as the percent - range 000->100  */
	gint message[2];

	/* Our commands */
	char *badblocks_cmd;
	char *mformat_cmd;
	char *mke2fs_cmd;
	char *mkdosfs_cmd;
};
/* This should do the format of the floppy. */
void format_floppy (GFloppy *floppy);

/* Return -1 if "device" is not a valid device on the system, and 0 if it is. */
/* NOTE: It should not actually check the media. */
typedef enum {
	GFLOPPY_NO_DEVICE,
	GFLOPPY_INVALID_PERMISSIONS,
	GFLOPPY_DEVICE_DISCONNECTED,
	GFLOPPY_DEVICE_OK
} GFloppyStatus;

GFloppyStatus test_floppy_device (char *device);
gint	      floppy_block_size  (GFloppySize size);

#endif

