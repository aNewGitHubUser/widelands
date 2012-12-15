/*
 * Copyright (C) 2006-2012 by the Widelands Development Team
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

#ifndef GL_SURFACE_H
#define GL_SURFACE_H

#include <boost/scoped_array.hpp>

#define NO_SDL_GLEXT
#include <GL/glew.h>
#include <SDL_opengl.h>

#include "graphic/surface.h"

class GLSurface : public Surface {
public:
	virtual ~GLSurface() {}

	/// Interface implementations
	virtual uint32_t get_w() const;
	virtual uint32_t get_h() const;
	virtual uint8_t * get_pixels() const;
	virtual void set_pixel(uint32_t x, uint32_t y, uint32_t clr);
	virtual uint32_t get_pixel(uint32_t x, uint32_t y);

	virtual void blit(const Point&, const IPicture*, const Rect& srcrc, Composite cm);
	virtual void fill_rect(const Rect&, RGBAColor);
	virtual void draw_rect(const Rect&, RGBColor);
	virtual void brighten_rect(const Rect&, int32_t factor);

	virtual void draw_line (int32_t x1, int32_t y1, int32_t x2, int32_t y2,
			const RGBColor&, uint8_t width);

protected:

	/// Logical width and height of the surface
	uint32_t m_w, m_h;

	/// Pixel data, while the texture is locked
	boost::scoped_array<uint8_t> m_pixels;
};

#endif /* end of include guard: GL_SURFACE_H */

