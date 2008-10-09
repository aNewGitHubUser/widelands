/*
 * Copyright (C) 2002-2004, 2006-2008 by the Widelands Development Team
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

#ifndef WORKER_DESCR_H
#define WORKER_DESCR_H

#include "bob.h"
#include "immovable.h"

namespace Widelands {

/// \todo (Antonio Trueba#1#): Get rid of forward class declaration (chicked-and-egg problem)
class Worker;
struct WorkerProgram;


class Worker_Descr : public Bob::Descr
{
	friend struct Tribe_Descr;
	friend class Warehouse;
	friend struct WorkerProgram;

	struct CostItem {
		std::string name;   // name of ware
		uint32_t    amount;

		CostItem(const char* iname, int32_t iamount)
			: name(iname), amount(iamount)
		{}
	};

	typedef std::vector<CostItem> BuildCost;

public:
	typedef Ware_Index::value_t Index;
	enum Worker_Type {
		NORMAL = 0,
		CARRIER,
		SOLDIER,
	};

	Worker_Descr(const Tribe_Descr &, const std::string & name);
	virtual ~Worker_Descr();

	virtual Bob * create_object() const;

	virtual void load_graphics();

	bool              get_buildable() const throw () {return m_buildable;}
	const BuildCost & get_buildcost() const throw () {return m_buildcost;}

	const Tribe_Descr * get_tribe() const throw () {return m_owner_tribe;}
	const Tribe_Descr & tribe() const throw () {return *m_owner_tribe;}
	const std::string & descname() const throw () {return m_descname;}
	std::string get_helptext() const {return m_helptext;}

	uint32_t get_menu_pic() const throw () {return m_menu_pic;}
	const DirAnimations & get_walk_anims() const throw () {return m_walk_anims;}
	const DirAnimations & get_right_walk_anims(const bool carries_ware)
		const throw ()
	{return carries_ware ? m_walkload_anims : m_walk_anims;}
	const WorkerProgram* get_program(std::string programname) const;

	virtual Worker_Type get_worker_type() const {return NORMAL;}

	// For leveling
	int32_t get_max_exp() const throw () {return m_max_experience;}
	int32_t get_min_exp() const throw () {return m_min_experience;}
	Ware_Index becomes() const throw () {return m_becomes;}
	bool can_act_as(Ware_Index) const;

	Worker & create
		(Editor_Game_Base &, Player &, PlayerImmovable &, const Coords) const;

	typedef std::map<Worker_Descr *, std::string> becomes_map_t;
	virtual uint32_t movecaps() const throw ();

	typedef std::map<std::string, WorkerProgram *> ProgramMap;
	ProgramMap const & get_all_programs() const throw () {return m_programs;}

protected:
	virtual void parse
		(char const * dir, Profile *, becomes_map_t &, const EncodeData *);
	static Worker_Descr * create_from_dir
		(Tribe_Descr const &,
		 becomes_map_t     &,
		 char        const * directory,
		 EncodeData  const *);

	std::string   m_descname;       ///< Descriptive name
	std::string   m_helptext;       ///< Short (tooltip-like) help text
	char        * m_menu_pic_fname; ///< Filename of worker's icon
	uint32_t          m_menu_pic;       ///< Pointer to icon into picture stack
	DirAnimations m_walk_anims;
	DirAnimations m_walkload_anims;
	bool          m_buildable;
	BuildCost     m_buildcost;      ///< What and how much we need to build this worker
	int32_t           m_max_experience;
	int32_t           m_min_experience;
	Ware_Index    m_becomes;       /// Type that this type can become (or Null).
	ProgramMap    m_programs;
};

};

#endif
