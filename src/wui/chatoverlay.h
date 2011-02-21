/*
 * Copyright (C) 2011 by the Widelands Development Team
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

#ifndef CHATOVERLAY_H
#define CHATOVERLAY_H

#include "ui_basic/panel.h"

#include <boost/scoped_ptr.hpp>

struct ChatProvider;

/**
 * The overlay that displays all new chat messages for some timeout on the main window.
 *
 * \see GameChatPanel, GameChatMenu
 */
struct ChatOverlay : public UI::Panel {
	ChatOverlay(UI::Panel * parent, int32_t x, int32_t y, int32_t w, int32_t h);
	~ChatOverlay();

	void setChatProvider(ChatProvider &);
	virtual void draw(RenderTarget &);
	virtual void think();

private:
	struct Impl;
	boost::scoped_ptr<Impl> m;
};

#endif // CHATOVERLAY_H