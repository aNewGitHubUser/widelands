/*
 * Copyright (C) 2002-2004, 2006, 2008-2009 by the Widelands Development Team
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

#ifndef EVENT_ALLOW_BUILDING_H
#define EVENT_ALLOW_BUILDING_H

#include "event.h"

#include "logic/widelands.h"

struct Event_Allow_Building_Option_Menu;

namespace Widelands {

struct Editor_Game_Base;
struct Tribe_Descr;

/**
 * Allows/denies the player to build a certain building
 */
struct Event_Allow_Building : public Event {
	friend struct ::Event_Allow_Building_Option_Menu;
	Event_Allow_Building(char const * name, State);
	Event_Allow_Building
		(Section &, Editor_Game_Base &, Tribe_Descr const * = 0);

	int32_t option_menu(Editor_Interactive &);

	void set_player(Player_Number);
	State run(Game &);

	void Write(Section &, Editor_Game_Base &) const;

	void set_allow(bool t) {m_allow = t;}
	bool get_allow() {return m_allow;}

private:
	Player_Number  m_player;
	Building_Index m_building;
	bool           m_allow;
};

}

#endif
