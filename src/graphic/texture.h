/*
 * Copyright (C) 2002-2004, 2006, 2008-2010 by the Widelands Development Team
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#ifndef TEXTURE_H
#define TEXTURE_H

#include "colormap.h"
#include "picture_id.h"

#include "graphic/render/gl_picture_texture.h"

#include <boost/shared_ptr.hpp>
#include <stdint.h>
#include <vector>

/**
 * This contains all the road textures needed to render roads
 */
struct Road_Textures {
	PictureID pic_road_normal;
	PictureID pic_road_busy;
};

/** class Texture
*
* Texture represents are terrain texture, which is strictly
* TEXTURE_WIDTH by TEXTURE_HEIGHT pixels in size. It uses 8 bit color, and
* a pointer to the corresponding palette and color lookup table is
* provided.
*
* Currently, this is initialized from a 16 bit bitmap. This should be
* changed to load 8 bit bitmaps directly.
*/
struct Texture {
	Texture
		(char const & fnametempl, uint32_t frametime, const SDL_PixelFormat &);
	~Texture();

	const char * get_texture_picture() const {return m_texture_picture;}

	uint8_t * get_pixels   () const {return m_pixels;}
	uint8_t * get_curpixels() const {return m_curframe;}
	void    * get_colormap () const {return m_colormap->get_colormap();}

	uint32_t get_minimap_color(char shade);

	void animate(uint32_t time);
	void reset_was_animated() {m_was_animated = false;}
	bool was_animated() const throw () {return m_was_animated;}
#ifdef USE_OPENGL
	uint32_t getTexture() const
		{return m_glFrames.at(m_frame_num)->get_gl_texture();}
#endif

private:
	Colormap * m_colormap;
	uint8_t  * m_pixels;
	uint32_t   m_mmap_color;
	uint8_t  * m_curframe;
	int32_t    m_frame_num;
	char     * m_texture_picture;
	uint32_t   m_nrframes;
	uint32_t   m_frametime;
	bool       is_32bit;
	bool       m_was_animated;
#ifdef USE_OPENGL
	std::vector<boost::shared_ptr<GLPictureTexture> > m_glFrames;
#endif
};

#endif
