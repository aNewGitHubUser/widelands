/*
 * Copyright (C) 2010 by the Widelands Development Team
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#ifndef COMPOSITEMODE_H
#define COMPOSITEMODE_H

/**
 * Defines composition operations performed while blitting.
 */
enum Composite {
	/**
	 * Perform a normal blitting operation that respects the alpha
	 * channel if present.
	 */
	CM_Normal = 0,

	/**
	 * Perform a solid blit that ignores the alpha channel information.
	 */
	CM_Solid,

	/**
	 * Copy all pixel information, including alpha channel information.
	 */
	CM_Copy
};

#endif // COMPOSITEMODE_H
