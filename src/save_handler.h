/*
 * Copyright (C) 2002-2009 by the Widelands Development Team
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

#ifndef SAVE_HANDLER_H
#define SAVE_HANDLER_H

#include <stdint.h>
#include <cstring>
#include <string>


namespace Widelands {struct Game;}

// default autosave interval in minutes
#define DEFAULT_AUTOSAVE_INTERVAL 15

class SaveHandler {
	int32_t m_lastSaveTime;
	bool m_initialized;
	bool m_allow_autosaving;

	void initialize(int32_t currenttime);


public:
	SaveHandler() : m_initialized(false), m_allow_autosaving(true) {}
	void think(Widelands::Game &, int32_t currenttime);
	std::string create_file_name(std::string dir, std::string filename);
	bool save_game
		(Widelands::Game   &,
		 std::string const & filename,
		 std::string       * error = 0);

	static std::string get_base_dir() {return "save";}
	void set_allow_autosaving(bool t) {m_allow_autosaving = t;}
	bool get_allow_autosaving() {return m_allow_autosaving;}
};

#endif
