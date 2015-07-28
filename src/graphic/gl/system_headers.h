/*
 * Copyright (C) 2006-2015 by the Widelands Development Team
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

#ifndef WL_GRAPHIC_GL_SYSTEM_HEADERS_H
#define WL_GRAPHIC_GL_SYSTEM_HEADERS_H

// This includes the correct OpenGL headers for us. Use this
// instead of including any system OpenGL headers yourself.

// So, GLEW is really a crappy piece of software, but for now we
// are stuck with it. Before making any changes here, see:
// https://www.opengl.org/wiki/OpenGL_Loading_Library
// and
// http://stackoverflow.com/questions/13558073/program-crash-on-glgenvertexarrays-call.
//
// TODO(sirver): glbinding seems to be a sane solution to the GL
// loading problem. (https://github.com/hpicgs/glbinding).

// GLEW must be first. Do not include any other GL headers, it
// should define all functions.

#ifdef USE_GLBINDING
#   include <glbinding/gl/gl.h>
#   include <glbinding/Binding.h>
// NOCOM(GunChleoc): Can this go? I could compile and run without this, but I don't know if I'm reaching this part of the code on my machine.
using namespace gl;
#else
#   include <GL/glew.h>
#endif

#endif  // end of include guard: WL_GRAPHIC_GL_SYSTEM_HEADERS_H
