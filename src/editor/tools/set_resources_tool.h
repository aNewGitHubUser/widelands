/*
 * Copyright (C) 2002-2004, 2006-2008, 2012 by the Widelands Development Team
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

#ifndef WL_EDITOR_TOOLS_SET_RESOURCES_TOOL_H
#define WL_EDITOR_TOOLS_SET_RESOURCES_TOOL_H

#include "editor/tools/tool.h"
#include "logic/mapregion.h"
#include "logic/widelands.h"

///  Decreases the resources of a node by a value.
struct EditorSetResourcesTool : public EditorTool {
	EditorSetResourcesTool()
		: EditorTool(*this, *this), cur_res_(0), set_to_(0)
	{}

	/**
	 * Sets the resources of the current to a fixed value
	*/
	int32_t handle_click_impl(const Widelands::World& world,
	                          Widelands::NodeAndTriangle<> center,
	                          EditorInteractive& parent,
	                          EditorActionArgs* args,
							  Widelands::Map* map) override;

	int32_t handle_undo_impl(const Widelands::World& world,
	                         Widelands::NodeAndTriangle<> center,
	                         EditorInteractive& parent,
	                         EditorActionArgs* args,
							 Widelands::Map* map) override;

	EditorActionArgs format_args_impl(EditorInteractive & parent) override;

	char const * get_sel_impl() const override {
		return "images/wui/editor/fsel_editor_set_resources.png";
	}

	uint8_t get_set_to() const       {return set_to_;}
	void set_set_to(uint8_t const n) {set_to_ = n;}
	Widelands::DescriptionIndex get_cur_res() const {return cur_res_;}
	void set_cur_res(Widelands::DescriptionIndex const res)
	{cur_res_ = res;}

private:
	Widelands::DescriptionIndex cur_res_;
	uint8_t set_to_;
};


#endif  // end of include guard: WL_EDITOR_TOOLS_SET_RESOURCES_TOOL_H