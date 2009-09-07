/*
 * Copyright (C) 2008-2009 by the Widelands Development Team
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

#ifndef EVENT_BUILDING_H
#define EVENT_BUILDING_H

#include "event.h"
#include "soldier_counts.h"
#include "widelands_geometry.h"

#include <vector>

struct Event_Building_Option_Menu;

namespace Widelands {

struct Tribe_Descr;

struct Event_Building : Event {
	friend struct ::Event_Building_Option_Menu;
	Event_Building(char const * Name, State const S) :
		Event          (Name, S),
		m_location     (Coords::Null()),
		m_required_suitability  (std::numeric_limits<int32_t>::min()),
		m_sufficient_suitability(std::numeric_limits<int32_t>::max()),
		m_distance_min          (0),
		m_distance_max          (0),
		m_player       (1),
		m_ware_counts  (0),
		m_worker_counts(0)
	{}
	Event_Building
		(Section &, Editor_Game_Base &,
		 Tribe_Descr const * = 0, Building_Index = Building_Index::Null());
	~Event_Building();

	bool has_option_menu() const {return false;}
	int32_t option_menu(Editor_Interactive &) __attribute__ ((noreturn));

	void set_player(Player_Number);
	void set_position(Coords);
	State run(Game &);

	void Write(Section &, Editor_Game_Base &) const;

private:
	Coords         m_location;
	int32_t        m_required_suitability, m_sufficient_suitability;
	uint8_t        m_distance_min, m_distance_max;
	Player_Number  m_player;
	Building_Index m_building;
	uint32_t     * m_ware_counts;
	uint32_t     * m_worker_counts;
	Soldier_Counts  m_soldier_counts;
};

};

#endif
