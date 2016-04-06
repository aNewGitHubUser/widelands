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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#ifndef WL_EDITOR_UI_MENUS_EDITOR_TOOL_CHANGE_RESOURCES_OPTIONS_MENU_H
#define WL_EDITOR_UI_MENUS_EDITOR_TOOL_CHANGE_RESOURCES_OPTIONS_MENU_H

#include "editor/ui_menus/editor_tool_options_menu.h"
#include "ui_basic/box.h"
#include "ui_basic/radiobutton.h"
#include "ui_basic/spinbox.h"
#include "ui_basic/textarea.h"

class EditorInteractive;
struct EditorIncreaseResourcesTool;

struct EditorToolChangeResourcesOptionsMenu :
	public EditorToolOptionsMenu
{
	EditorToolChangeResourcesOptionsMenu
		(EditorInteractive             &,
		 EditorIncreaseResourcesTool &,
		 UI::UniqueWindow::Registry     &);

private:
	EditorInteractive & eia();
	void change_resource();
	void update_change_by();
	void update_set_to();
	void update();

	UI::Box box_;
	UI::SpinBox change_by_;
	UI::SpinBox set_to_;
	UI::Box resources_box_;
	UI::Radiogroup radiogroup_;
	UI::Textarea cur_selection_;
	EditorIncreaseResourcesTool& increase_tool_;
};

#endif  // end of include guard: WL_EDITOR_UI_MENUS_EDITOR_TOOL_CHANGE_RESOURCES_OPTIONS_MENU_H
