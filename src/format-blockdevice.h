/*
 * gnome-format-blockdevice.h - discover block device
 *
 * Copyright 2007 Riccardo Setti <giskard@autistici.org>
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

typedef struct FormatBlockDevice FormatBlockDevice;

struct FormatBlockDevice {

  LibHalContext *hal_ctx;
  LibHalVolume *volume;

  char **udis;
  int num_udis;
};

FormatBlockDevice *find_volume ();
void find_device_infos (FormatBlockDevice *device, char *udi);
  
