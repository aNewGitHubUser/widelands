/*
 * Copyright (C) 2002-2018 by the Widelands Development Team
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

#include "logic/map_objects/tribes/dismantlesite.h"

#include <cstdio>

#include <boost/format.hpp>

#include "base/i18n.h"
#include "base/macros.h"
#include "base/wexception.h"
#include "economy/wares_queue.h"
#include "graphic/animation.h"
#include "graphic/rendertarget.h"
#include "graphic/text_constants.h"
#include "logic/editor_game_base.h"
#include "logic/game.h"
#include "logic/map_objects/tribes/tribe_descr.h"
#include "logic/map_objects/tribes/worker.h"

namespace Widelands {

/**
  * The contents of 'table' are documented in
  * /data/tribes/buildings/partially_finished/dismantlesite/init.lua
  */

DismantleSiteDescr::DismantleSiteDescr(const std::string& init_descname,
                                       const LuaTable& table,
                                       const EditorGameBase& egbase)
   : BuildingDescr(init_descname, MapObjectType::DISMANTLESITE, table, egbase) {
	add_attribute(MapObject::Attribute::CONSTRUCTIONSITE);  // Yep, this is correct.
}

Building& DismantleSiteDescr::create_object() const {
	return *new DismantleSite(*this);
}

/*
==============================

IMPLEMENTATION

==============================
*/

DismantleSite::DismantleSite(const DismantleSiteDescr& gdescr) : PartiallyFinishedBuilding(gdescr) {
}

DismantleSite::DismantleSite(const DismantleSiteDescr& gdescr,
                             EditorGameBase& egbase,
                             const Coords& c,
                             Player* plr,
                             bool loading,
                             Building::FormerBuildings& former_buildings)
   : PartiallyFinishedBuilding(gdescr) {
	position_ = c;
	set_owner(plr);

	assert(!former_buildings.empty());
	for (DescriptionIndex former_idx : former_buildings) {
		old_buildings_.push_back(former_idx);
	}
	const BuildingDescr* cur_descr = owner().tribe().get_building_descr(old_buildings_.back());
	set_building(*cur_descr);

	if (loading) {
		Building::init(egbase);
	} else {
		init(egbase);
	}
}

/*
===============
Print completion percentage.
===============
*/
void DismantleSite::update_statistics_string(std::string* s) {
	unsigned int percent = (get_built_per64k() * 100) >> 16;
	*s = (boost::format("<font color=%s>%s</font>") % UI_FONT_CLR_DARK.hex_value() %
	      (boost::format(_("%u%% dismantled")) % percent))
	        .str();
}

/*
===============
Initialize the construction site by starting orders
===============
*/
bool DismantleSite::init(EditorGameBase& egbase) {
	PartiallyFinishedBuilding::init(egbase);

	for (const auto& ware : count_returned_wares(this)) {
		WaresQueue* wq = new WaresQueue(*this, ware.first, ware.second);
		wq->set_filled(ware.second);
		wares_.push_back(wq);
		work_steps_ += ware.second;
	}
	return true;
}

/*
===============
Count which wares you get back if you dismantle the given building
===============
*/
const Buildcost DismantleSite::count_returned_wares(Building* building) {
	Buildcost result;
	for (DescriptionIndex former_idx : building->get_former_buildings()) {
		const BuildingDescr* former_descr = building->owner().tribe().get_building_descr(former_idx);
		const Buildcost& return_wares = former_idx != building->get_former_buildings().front() ?
		                                   former_descr->returned_wares_enhanced() :
		                                   former_descr->returned_wares();

		for (const auto& ware : return_wares) {
			// TODO(GunChleoc): Once we have trading, we might want to return all wares again.
			if (building->owner().tribe().has_ware(ware.first)) {
				result[ware.first] += ware.second;
			}
		}
	}
	return result;
}

/*
===============
Construction sites only burn if some of the work has been completed.
===============
*/
bool DismantleSite::burn_on_destroy() {
	if (work_completed_ >= work_steps_)
		return false;  // completed, so don't burn

	return true;
}

/*
===============
Called by our builder to get instructions.
===============
*/
bool DismantleSite::get_building_work(Game& game, Worker& worker, bool) {
	if (&worker != builder_.get(game)) {
		// Not our construction worker; e.g. a miner leaving a mine
		// that is supposed to be enhanced. Make him return to a warehouse
		worker.pop_task(game);
		worker.start_task_leavebuilding(game, true);
		return true;
	}

	if (!work_steps_)           //  Happens for building without buildcost.
		schedule_destroy(game);  //  Complete the building immediately.

	// Check if one step has completed
	if (static_cast<int32_t>(game.get_gametime() - work_steptime_) >= 0 && working_) {
		++work_completed_;

		for (uint32_t i = 0; i < wares_.size(); ++i) {
			WaresQueue& wq = *wares_[i];

			if (!wq.get_filled())
				continue;

			wq.set_filled(wq.get_filled() - 1);
			wq.set_max_size(wq.get_max_size() - 1);

			// Update statistics
			get_owner()->ware_produced(wq.get_index());

			const WareDescr& wd = *owner().tribe().get_ware_descr(wq.get_index());
			WareInstance& ware = *new WareInstance(wq.get_index(), &wd);
			ware.init(game);
			worker.start_task_dropoff(game, ware);

			working_ = false;
			return true;
		}
	}

	if (work_completed_ >= work_steps_) {
		schedule_destroy(game);

		worker.pop_task(game);
		// No more building, so move to the flag
		worker.start_task_move(game, WALK_SE, worker.descr().get_right_walk_anims(false), true);
		worker.set_location(nullptr);
	} else if (!working_) {
		work_steptime_ = game.get_gametime() + DISMANTLESITE_STEP_TIME;
		worker.start_task_idle(game, worker.descr().get_animation("work"), DISMANTLESITE_STEP_TIME);

		working_ = true;
	}
	return true;
}

/*
===============
Draw it.
===============
*/
void DismantleSite::draw(uint32_t gametime,
                         const TextToDraw draw_text,
                         const Vector2f& point_on_dst,
                         float scale,
                         RenderTarget* dst) {
	uint32_t tanim = gametime - animstart_;
	const RGBColor& player_color = get_owner()->get_playercolor();

	// Draw the construction site marker
	dst->blit_animation(point_on_dst, scale, anim_, tanim, player_color);

	// Blit bottom part of the animation according to dismantle progress
	dst->blit_animation(point_on_dst, scale, building_->get_unoccupied_animation(), tanim,
	                    player_color, 100 - ((get_built_per64k() * 100) >> 16));

	// Draw help strings
	draw_info(draw_text, point_on_dst, scale, dst);
}
}
