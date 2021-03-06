/*
 * modified 1999 by Red Hat, Inc. for use in gfloppy, by jrb@redhat.com
 */

#include "config.h"

#include <glib/gi18n.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <sys/ioctl.h>

#include <linux/types.h>
#ifdef HAVE_LINUX_FD_H
#include <linux/fd.h>
#endif

#include <string.h>
#include "gfloppy.h"
#include "fdformat.h"

struct floppy_struct param;
extern int errno;

void
fd_print (GFloppy *floppy, MessageType type, char *string)
{
	char *buf;

	buf = g_strdup_printf ("%c%s", type, string);

	write (floppy->message[1], buf, strlen (buf));
	write (floppy->message[1], "\000", 1);

	g_free (buf);
}

/* accepts open file handle for block device to be formatted */
static int
format_disk (GFloppy *floppy, gint ctrl)
{
	struct format_descr descr;
	int track;
	char *msg;

	fd_print (floppy, MSG_MESSAGE, _("Formatting the disk..."));

	if (ioctl (ctrl, FDFMTBEG, NULL) < 0) {
		fd_print (floppy, MSG_ERROR, _("I don't know what this is, but it's very wrong."));
		return -1;
	}

	for (track = 0; track < param.track; track++) {
		descr.track = track;
		descr.head = 0;
		if (ioctl(ctrl,FDFMTTRK,(long) &descr) < 0) {
			msg = g_strdup_printf (_("Error formatting track #%d"),track);
			fd_print (floppy, MSG_ERROR, msg);
			g_free (msg);
			return -1;
		}

		msg = g_strdup_printf ("%3d", (gint)(((float)track)/param.track * 33.3));
		fd_print (floppy, MSG_PROGRESS, msg);
		g_free (msg);

		if (param.head == 2) {
			descr.head = 1;
			if (ioctl(ctrl,FDFMTTRK,(long) &descr) < 0) {
				msg = g_strdup_printf (_("Error formatting track #%d"),track);
				fd_print (floppy, MSG_ERROR, msg);
				g_free (msg);

				return -1;
			}
		}
	}
	if (ioctl(ctrl,FDFMTEND,NULL) < 0) {
		fd_print (floppy, MSG_ERROR, _("Error during completion of formatting"));
		return -1;
	}
	/* XXXX - need to tell parent we're finished */
	fd_print (floppy, MSG_MESSAGE, _("Formatting the disk... Done"));
	return 0;
}


static gint
verify_disk (GFloppy *floppy)
{
	unsigned char *data;
	int fd, cyl_size, cyl, count;
	char *msg;

	cyl_size = param.sect*param.head*512;
	data = (unsigned char *) g_malloc (cyl_size);

	/* XXXX - tell parent we're starting verify stage */
	fd_print (floppy, MSG_MESSAGE, _("Verifying the format..."));

	if ((fd = open(floppy->device,O_RDONLY)) < 0) {
		switch (errno) {
		case EROFS:
			msg = g_strconcat (_("Unable to write to the floppy.\n\nPlease confirm that it is not write-protected."), NULL);
			break;
		case EACCES:
			msg = g_strdup_printf (_("Insufficient permissions to open floppy device %s."), floppy->device);
			break;
		case ENODEV:
		case ENXIO:
			msg = g_strdup_printf (_("Unable to access the floppy disk.\n\nPlease confirm that it is in the drive\nwith the drive door shut."));
			break;
		default:
			msg = g_strdup_printf (_("Generic error accessing floppy device %s.\n\nError code %s:%d"),
					       floppy->device, strerror (errno), errno);
			break;
		}

		fd_print (floppy, MSG_ERROR, msg);
		g_free (msg);

		return -1;
	}

	for (cyl = 0; cyl < param.track; cyl++) {
		int read_bytes;

		/* XXXX - let parent know status */
		msg = g_strdup_printf ("%3d", 33 + (gint)(((float)cyl)/param.track * 33.3));
		fd_print (floppy, MSG_PROGRESS, msg);
		g_free (msg);

		read_bytes = read(fd,data,cyl_size);

		if (read_bytes != cyl_size) {
			if(read_bytes < 0)
				msg =  g_strdup_printf(_("Read Error:\nProblem reading cylinder %d, expected %d, read %d"),
						       cyl, cyl_size, read_bytes);
			else
				msg = g_strdup_printf (_("Problem reading cylinder %d, expected %d, read %d"),
						       cyl, cyl_size, read_bytes);

			fd_print (floppy, MSG_ERROR, msg);
			g_free (msg);

			return -1;
		}

		for (count = 0; count < cyl_size; count++)
			if (data[count] != FD_FILL_BYTE) {
				msg = g_strdup_printf (_("Bad data in cyl %d.  Continuing... "),cyl);
				fd_print (floppy, MSG_MESSAGE, msg);
				g_free (msg);

				break;
			}
	}

	if (close(fd) < 0) {
		msg = g_strdup_printf (_("Error closing device %s"), floppy->device);
		fd_print (floppy, MSG_ERROR, msg);
		g_free (msg);

		return -1;
	}

	fd_print (floppy, MSG_MESSAGE, _("Verifying the format... Done"));

	return 0;
}

gint
fdformat_disk (GFloppy *floppy)
{
	gint ctrl;
	char *msg;

	if (access(floppy->device,W_OK) < 0) {
		msg = g_strdup_printf (_("Unable to write to device %s"), floppy->device);
		fd_print (floppy, MSG_ERROR, msg);
		g_free (msg);

		return -1;
	}

	if ((ctrl = open(floppy->device,O_RDWR)) < 0) {
		switch (errno) {
		case EROFS:
			msg = g_strdup (_("Unable to write to the floppy.\n\nPlease confirm that it is not write-protected."));
			break;
		case EACCES:
			msg = g_strdup_printf (_("Insufficient permissions to open floppy device %s."), floppy->device);
			break;
		case ENODEV:
		case ENXIO:
			msg = g_strdup_printf (_("Unable to access the floppy disk.\n\nPlease confirm that it is in the drive\nwith the drive door shut."));
			break;
		default:
			msg = g_strdup_printf (_("Generic error accessing floppy device %s.\n\nError code %s"),
					       floppy->device, strerror (errno));
			break;
		}

		fd_print (floppy, MSG_ERROR, msg);
		g_free (msg);

		return -1;
	}
	
	if (ioctl(ctrl,FDGETPRM,(long) &param) < 0) {
		fd_print (floppy, MSG_ERROR, _("Could not determine current floppy geometry."));

		return -1;
	}

/*	printf(_("%s-sided, %d tracks, %d sec/track. Total capacity %d kB."), 
	param.head ? _("Double") : _("Single"),param.track,param.sect,param.size >> 1);*/
	if (format_disk (floppy, ctrl) != 0) {
		close (ctrl);
		return -1;
	}
	close (ctrl);
	if (verify_disk(floppy) != 0)
		return -1;
	return 0;
}
