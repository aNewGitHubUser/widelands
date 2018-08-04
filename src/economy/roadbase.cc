/*
 * Copyright (C) 2004-2018 by the Widelands Development Team
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

#include "economy/roadbase.h"

#include "base/macros.h"
#include "economy/economy.h"
#include "economy/flag.h"
#include "economy/request.h"
#include "logic/editor_game_base.h"
#include "logic/game.h"
#include "logic/map_objects/map_object.h"
#include "logic/map_objects/tribes/carrier.h"
#include "logic/map_objects/tribes/tribe_descr.h"
#include "logic/player.h"

namespace Widelands {

/**
 * Most of the actual work is done in init.
 */
RoadBase::RoadBase(const RoadBaseDescr& d, RoadType type)
   : PlayerImmovable(d), idle_index_(0), type_(type) {
	flags_[0] = flags_[1] = nullptr;
	flagidx_[0] = flagidx_[1] = -1;
}

int32_t RoadBase::get_size() const {
	return SMALL;
}

bool RoadBase::get_passable() const {
	return true;
}

BaseImmovable::PositionList RoadBase::get_positions(const EditorGameBase& egbase) const {
	const Map& map = egbase.map();
	Coords curf = map.get_fcoords(path_.get_start());

	PositionList rv;
	const Path::StepVector::size_type nr_steps = path_.get_nsteps();
	for (Path::StepVector::size_type steps = 0; steps < nr_steps + 1; ++steps) {
		if (steps > 0 && steps < path_.get_nsteps())
			rv.push_back(curf);

		if (steps < path_.get_nsteps())
			map.get_neighbour(curf, path_[steps], &curf);
	}
	return rv;
}

Flag& RoadBase::base_flag() {
	return *flags_[FlagStart];
}

/**
 * Return the cost of getting from fromflag to the other flag.
 */
int32_t RoadBase::get_cost(FlagId fromflag) {
	return cost_[fromflag];
}

/**
 * Set the new path, calculate costs.
 * You have to set start and end flags before calling this function.
 */
void RoadBase::set_path(EditorGameBase& egbase, const Path& path) {
	assert(path.get_nsteps() >= 2);
	assert(path.get_start() == flags_[FlagStart]->get_position());
	assert(path.get_end() == flags_[FlagEnd]->get_position());

	path_ = path;
	egbase.map().calc_cost(path, &cost_[FlagStart], &cost_[FlagEnd]);

	// Figure out where carriers should idle
	idle_index_ = path.get_nsteps() / 2;
}

/**
 * Add road/waterway markings to the map
 */
void RoadBase::mark_map(EditorGameBase& egbase) {
	const Map& map = egbase.map();
	FCoords curf = map.get_fcoords(path_.get_start());

	const Path::StepVector::size_type nr_steps = path_.get_nsteps();
	for (Path::StepVector::size_type steps = 0; steps < nr_steps + 1; ++steps) {
		if (steps > 0 && steps < path_.get_nsteps())
			set_position(egbase, curf);

		// mark the road that leads up to this field
		if (steps > 0) {
			const Direction dir = get_reverse_dir(path_[steps - 1]);
			if (dir == WALK_SW || dir == WALK_SE || dir == WALK_E)
				egbase.set_road(curf, dir, type_);
		}

		// mark the road that leads away from this field
		if (steps < path_.get_nsteps()) {
			const Direction dir = path_[steps];
			if (dir == WALK_SW || dir == WALK_SE || dir == WALK_E)
				egbase.set_road(curf, dir, type_);
			map.get_neighbour(curf, dir, &curf);
		}
	}
}

/**
 * Remove road markings from the map
 */
void RoadBase::unmark_map(EditorGameBase& egbase) {
	const Map& map = egbase.map();
	FCoords curf = map.get_fcoords(path_.get_start());

	const Path::StepVector::size_type nr_steps = path_.get_nsteps();
	for (Path::StepVector::size_type steps = 0; steps < nr_steps + 1; ++steps) {
		if (steps > 0 && steps < path_.get_nsteps())
			set_position(egbase, curf);

		// mark the road that leads up to this field
		if (steps > 0) {
			const Direction dir = get_reverse_dir(path_[steps - 1]);
			if (dir == WALK_SW || dir == WALK_SE || dir == WALK_E)
				egbase.set_road(curf, dir, RoadType::kNone);
		}

		// mark the road that leads away from this field
		if (steps < path_.get_nsteps()) {
			const Direction dir = path_[steps];
			if (dir == WALK_SW || dir == WALK_SE || dir == WALK_E)
				egbase.set_road(curf, dir, RoadType::kNone);
			map.get_neighbour(curf, dir, &curf);
		}
	}
}

/**
 * Initialize the RoadBase.
 */
bool RoadBase::init(EditorGameBase& egbase) {
	PlayerImmovable::init(egbase);

	if (2 <= path_.get_nsteps())
		link_into_flags(egbase);
	return true;
}

/**
 * This links into the flags, calls a carrier
 * and so on. This was formerly done in init (and
 * still is for normal games). But for save game loading
 * we needed to have this road already registered
 * as Map Object, thats why this is moved
 */
void RoadBase::link_into_flags(EditorGameBase& egbase) {
	assert(path_.get_nsteps() >= 2);

	// Link into the flags (this will also set our economy)
	{
		const Direction dir = path_[0];
		flags_[FlagStart]->attach_road(dir, this);
		flagidx_[FlagStart] = dir;
	}

	const Direction dir = get_reverse_dir(path_[path_.get_nsteps() - 1]);
	flags_[FlagEnd]->attach_road(dir, this);
	flagidx_[FlagEnd] = dir;

	Economy::check_merge(*flags_[FlagStart], *flags_[FlagEnd]);

	// Mark Fields
	mark_map(egbase);
}

/**
 * Cleanup the road
 */
void RoadBase::cleanup(EditorGameBase& egbase) {

	// Unmark Fields
	unmark_map(egbase);

	// Unlink from flags (also clears the economy)
	flags_[FlagStart]->detach_road(flagidx_[FlagStart]);
	flags_[FlagEnd]->detach_road(flagidx_[FlagEnd]);

	Economy::check_split(*flags_[FlagStart], *flags_[FlagEnd]);

	if (upcast(Game, game, &egbase)) {
		flags_[FlagStart]->update_wares(*game, flags_[FlagEnd]);
		flags_[FlagEnd]->update_wares(*game, flags_[FlagStart]);
	}

	PlayerImmovable::cleanup(egbase);
}

void RoadBase::assign_carrier(Carrier&, uint8_t) {
}

bool RoadBase::notify_ware(Game&, FlagId) {
	return false;
}

/**
 * Workers' economies are fixed by PlayerImmovable, but we need to handle
 * any requests ourselves.
 */
void RoadBase::set_economy(Economy* const e) {
	PlayerImmovable::set_economy(e);
}

}  // namespace Widelands
