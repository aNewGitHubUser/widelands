/*
 * Copyright (C) 2002-2008, 2010-2011 by the Widelands Development Team
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

#ifndef WL_UI_FSMENU_CAMPAIGN_SELECT_H
#define WL_UI_FSMENU_CAMPAIGN_SELECT_H

#include "ui_fsmenu/base.h"
#include "ui_basic/button.h"
#include "ui_basic/listselect.h"
#include "ui_basic/multilinetextarea.h"
#include "ui_basic/textarea.h"
#include "ui_fsmenu/load_map_or_game.h"

/*
 * Fullscreen Menu for all Campaigns
 */

/*
 * UI 1 - Selection of Campaign
 *
 */
struct FullscreenMenuCampaignSelect : public FullscreenMenuLoadMapOrGame {
	FullscreenMenuCampaignSelect();

	int32_t get_campaign();

protected:
	void clicked_ok() override;

private:
	void campaign_selected(uint32_t);
	void fill_list();

	UI::Textarea                  m_title;
	UI::Textarea                  m_label_campname;
	UI::MultilineTextarea         m_ta_campname;
	UI::Textarea                  m_label_difficulty;
	UI::MultilineTextarea         m_ta_difficulty;
	UI::Textarea                  m_label_description;
	UI::MultilineTextarea         m_ta_description;

	UI::Listselect<const char *>  m_list;

	/// Variables used for exchange between the two Campaign UIs and
	/// Game::run_campaign
	int32_t                       campaign;
};
/*
 * UI 2 - Selection of a map
 *
 */
struct FullscreenMenuCampaignMapSelect : public FullscreenMenuLoadMapOrGame {
	FullscreenMenuCampaignMapSelect();

	std::string get_map();
	void set_campaign(uint32_t);

private:
	void map_selected(uint32_t);
	void fill_list();

	UI::Textarea                  m_title;
	UI::Textarea                  m_subtitle;
	UI::Textarea                  m_label_mapname;
	UI::MultilineTextarea         m_ta_mapname;
	UI::Textarea                  m_label_author;
	UI::MultilineTextarea         m_ta_author;
	UI::Textarea                  m_label_description;
	UI::MultilineTextarea         m_ta_description;

	UI::Listselect<std::string>   m_list;
	uint32_t                      campaign;
	std::string                   campmapfile;
};

#endif  // end of include guard: WL_UI_FSMENU_CAMPAIGN_SELECT_H
