/*
 * Copyright (C) 2006-2013 by the Widelands Development Team
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

#include <SDL.h>

#include "render/gl_surface_texture.h"
#include "render/sdl_helper.h"
#include "render/sdl_surface.h"

#include "surface.h"

extern bool g_opengl;

Surface* Surface::create(SDL_Surface* surf) {
#ifdef USE_OPENGL
	if (g_opengl) {
		return new GLSurfaceTexture(surf);
	}
#endif
	SDL_Surface * surface = SDL_DisplayFormatAlpha(surf);
	SDL_FreeSurface(surf);
	return new SDLSurface(surface);
}

Surface* Surface::create(uint16_t w, uint16_t h) {
#ifdef USE_OPENGL
	if (g_opengl) {
		return new GLSurfaceTexture(w, h);
	} else
#endif
	{
		SDL_Surface* tsurf = empty_sdl_surface(w, h);
		SDL_Surface* surf = SDL_DisplayFormatAlpha(tsurf);
		SDL_FreeSurface(tsurf);
		return new SDLSurface(surf);
	}
}


