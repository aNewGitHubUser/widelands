/*
 * Copyright (C) 2002-2004, 2006-2011 by the Widelands Development Team
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

#include "wui/interactive_player.h"

#include <boost/bind.hpp>
#include <boost/format.hpp>
#include <boost/lambda/bind.hpp>
#include <boost/lambda/construct.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/type_traits.hpp>

#include "base/i18n.h"
#include "base/macros.h"
#include "economy/flag.h"
#include "game_io/game_loader.h"
#include "graphic/font_handler.h"
#include "logic/building.h"
#include "logic/cmd_queue.h"
#include "logic/constants.h"
#include "logic/constructionsite.h"
#include "logic/immovable.h"
#include "logic/message_queue.h"
#include "logic/player.h"
#include "logic/productionsite.h"
#include "logic/soldier.h"
#include "logic/tribe.h"
#include "profile/profile.h"
#include "ui_basic/unique_window.h"
#include "wui/building_statistics_menu.h"
#include "wui/debugconsole.h"
#include "wui/encyclopedia_window.h"
#include "wui/fieldaction.h"
#include "wui/game_chat_menu.h"
#include "wui/game_main_menu.h"
#include "wui/game_main_menu_save_game.h"
#include "wui/game_message_menu.h"
#include "wui/game_objectives_menu.h"
#include "wui/game_options_menu.h"
#include "wui/general_statistics_menu.h"
#include "wui/overlay_manager.h"
#include "wui/stock_menu.h"
#include "wui/ware_statistics_menu.h"

using Widelands::Building;
using Widelands::Map;

namespace  {

// This function is the callback for recalculation of field overlays
int32_t int_player_overlay_callback_function
	(Widelands::TCoords<Widelands::FCoords> const c, InteractivePlayer& iap)
{
	assert(iap.get_player());
	return iap.get_player()->get_buildcaps(c);
}

}  // namespace

InteractivePlayer::InteractivePlayer
	(Widelands::Game        &       _game,
	 Section                &       global_s,
	 Widelands::PlayerNumber const plyn,
	 bool                     const multiplayer)
	:
	InteractiveGameBase (_game, global_s, NONE, multiplayer, multiplayer),
	m_auto_roadbuild_mode(global_s.get_bool("auto_roadbuild_mode", true)),
	m_flag_to_connect(Widelands::Coords::null()),

// Chat is different, as m_chatProvider needs to be checked when toggling
// Buildhelp is different as it does not toggle a UniqueWindow
// Minimap is different as it warps and stuff

#define INIT_BTN_this(image_key, name, tooltip)                       \
 TOOLBAR_BUTTON_COMMON_PARAMETERS(name),                                      \
 g_gr->cataloged_image(image_key),                      \
 tooltip                                                                      \


#define INIT_BTN(image_key, name, tooltip)                            \
 TOOLBAR_BUTTON_COMMON_PARAMETERS(name),                                      \
 g_gr->cataloged_image(image_key),                      \
 tooltip                                                                      \


m_toggle_chat
	(INIT_BTN_this
	 (ImageCatalog::Keys::kMenuChat, "chat", _("Chat"))),
m_toggle_options_menu
	(INIT_BTN
	 (ImageCatalog::Keys::kMenuOptions, "options_menu", _("Options"))),
m_toggle_statistics_menu
	(INIT_BTN
	 (ImageCatalog::Keys::kMenuStatistics, "statistics_menu", _("Statistics"))),
m_toggle_objectives
	(INIT_BTN
	 (ImageCatalog::Keys::kMenuObjectives, "objectives", _("Objectives"))),
m_toggle_minimap
	(INIT_BTN_this
	 (ImageCatalog::Keys::kMenuMinimap, "minimap", _("Minimap"))),
m_toggle_buildhelp
	(INIT_BTN_this
	 (ImageCatalog::Keys::kMenuBuildhelp, "buildhelp", _("Show Building Spaces (on/off)"))),
m_toggle_message_menu
	(INIT_BTN
	 (ImageCatalog::Keys::kMenuMessagesOld, "messages", _("Messages"))),
m_toggle_help
	(INIT_BTN
	 (ImageCatalog::Keys::kHelp, "help", _("Tribal Ware Encyclopedia")))

{
	m_toggle_chat.sigclicked.connect
		(boost::bind(&InteractivePlayer::toggle_chat, this));
	m_toggle_options_menu.sigclicked.connect
		(boost::bind(&UI::UniqueWindow::Registry::toggle, boost::ref(m_options)));
	m_toggle_statistics_menu.sigclicked.connect
		(boost::bind(&UI::UniqueWindow::Registry::toggle, boost::ref(m_statisticsmenu)));
	m_toggle_objectives.sigclicked.connect
		(boost::bind(&UI::UniqueWindow::Registry::toggle, boost::ref(m_objectives)));
	m_toggle_minimap.sigclicked.connect
		(boost::bind(&InteractivePlayer::toggle_minimap, this));
	m_toggle_buildhelp.sigclicked.connect
		(boost::bind(&InteractivePlayer::toggle_buildhelp, this));
	m_toggle_message_menu.sigclicked.connect
		(boost::bind(&UI::UniqueWindow::Registry::toggle, boost::ref(m_message_menu)));
	m_toggle_help.sigclicked.connect
		(boost::bind(&UI::UniqueWindow::Registry::toggle, boost::ref(m_encyclopedia)));

	// TODO(unknown): instead of making unneeded buttons invisible after generation,
	// they should not at all be generated. -> implement more dynamic toolbar UI
	m_toolbar.add(&m_toggle_options_menu,    UI::Box::AlignLeft);
	m_toolbar.add(&m_toggle_statistics_menu, UI::Box::AlignLeft);
	m_toolbar.add(&m_toggle_minimap,         UI::Box::AlignLeft);
	m_toolbar.add(&m_toggle_buildhelp,       UI::Box::AlignLeft);
	if (multiplayer) {
		m_toolbar.add(&m_toggle_chat,            UI::Box::AlignLeft);
		m_toggle_chat.set_visible(false);
		m_toggle_chat.set_enabled(false);
	}

	m_toolbar.add(&m_toggle_help,            UI::Box::AlignLeft);
	m_toolbar.add(&m_toggle_objectives,      UI::Box::AlignLeft);
	m_toolbar.add(&m_toggle_message_menu,    UI::Box::AlignLeft);

	set_player_number(plyn);
	fieldclicked.connect(boost::bind(&InteractivePlayer::node_action, this));

	adjust_toolbar_position();

#define INIT_BTN_HOOKS(registry, btn)                                        \
 registry.on_create = std::bind(&UI::Button::set_perm_pressed, &btn, true);  \
 registry.on_delete = std::bind(&UI::Button::set_perm_pressed, &btn, false); \
 if (registry.window) btn.set_perm_pressed(true);                            \

	INIT_BTN_HOOKS(m_chat, m_toggle_chat)
	INIT_BTN_HOOKS(m_options, m_toggle_options_menu)
	INIT_BTN_HOOKS(m_statisticsmenu, m_toggle_statistics_menu)
	INIT_BTN_HOOKS(minimap_registry(), m_toggle_minimap)
	INIT_BTN_HOOKS(m_objectives, m_toggle_objectives)
	INIT_BTN_HOOKS(m_encyclopedia, m_toggle_help)
	INIT_BTN_HOOKS(m_message_menu, m_toggle_message_menu)

	m_encyclopedia.open_window = [this] {new EncyclopediaWindow(*this, m_encyclopedia);};
	m_options.open_window = [this] {new GameOptionsMenu(*this, m_options, m_mainm_windows);};
	m_statisticsmenu.open_window = [this] {
		new GameMainMenu(*this, m_statisticsmenu, m_mainm_windows);
	};
	m_objectives.open_window = [this] {new GameObjectivesMenu(this, m_objectives);};
	m_message_menu.open_window = [this] {new GameMessageMenu(*this, m_message_menu);};
	m_mainm_windows.stock.open_window = [this] {new StockMenu(*this, m_mainm_windows.stock);};

#ifndef NDEBUG //  only in debug builds
	addCommand
		("switchplayer",
		 boost::bind(&InteractivePlayer::cmdSwitchPlayer, this, _1));
#endif
}

InteractivePlayer::~InteractivePlayer() {
#define DEINIT_BTN_HOOKS(registry, btn)                                                            \
	registry.on_create = 0;                                                                         \
	registry.on_delete = 0;

	DEINIT_BTN_HOOKS(m_chat, m_toggle_chat)
	DEINIT_BTN_HOOKS(m_options, m_toggle_options_menu)
	DEINIT_BTN_HOOKS(m_statisticsmenu, m_toggle_statistics_menu)
	DEINIT_BTN_HOOKS(minimap_registry(), m_toggle_minimap)
	DEINIT_BTN_HOOKS(m_objectives, m_toggle_objectives)
	DEINIT_BTN_HOOKS(m_encyclopedia, m_toggle_help)
	DEINIT_BTN_HOOKS(m_message_menu, m_toggle_message_menu)
}


/*
===============
Called just before the game starts, after postload, init and gfxload
===============
*/
void InteractivePlayer::start()
{
	postload();
}

void InteractivePlayer::think()
{
	InteractiveBase::think();

	if (m_flag_to_connect) {
		Widelands::Field & field = egbase().map()[m_flag_to_connect];
		if (upcast(Widelands::Flag const, flag, field.get_immovable())) {
			if (!flag->has_road() && !is_building_road())
				if (m_auto_roadbuild_mode) {
					//  There might be a fieldaction window open, showing a button
					//  for roadbuilding. If that dialog remains open so that the
					//  button is clicked, we would enter roadbuilding mode while
					//  we are already in roadbuilding mode from the call below.
					//  That is not allowed. Therefore we must delete the
					//  fieldaction window before entering roadbuilding mode here.
					delete m_fieldaction.window;
					m_fieldaction.window = nullptr;
					warp_mouse_to_node(m_flag_to_connect);
					set_sel_pos
						(Widelands::NodeAndTriangle<>
						 	(m_flag_to_connect,
						 	 Widelands::TCoords<>
						 	 	(m_flag_to_connect, Widelands::TCoords<>::D)));
					start_build_road(m_flag_to_connect, field.get_owned_by());
				}
			m_flag_to_connect = Widelands::Coords::null();
		}
	}
	if (is_multiplayer()) {
		m_toggle_chat.set_visible(m_chatenabled);
		m_toggle_chat.set_enabled(m_chatenabled);
	}
	{
		ImageCatalog::Keys image_key = ImageCatalog::Keys::kMenuMessagesOld;

		std::string msg_tooltip = _("Messages");
		if
			(uint32_t const nr_new_messages =
			 	player().messages().nr_messages(Widelands::Message::New))
		{
			image_key = ImageCatalog::Keys::kMenuMessagesNew;
			msg_tooltip =
			   (boost::format(ngettext("%u new message", "%u new messages", nr_new_messages)) %
			    nr_new_messages).str();
		}
		m_toggle_message_menu.set_pic(g_gr->cataloged_image(image_key));
		m_toggle_message_menu.set_tooltip(msg_tooltip);
	}
}


/**
 * Called for every game after loading (from a savegame or just from a map
 * during single/multiplayer/scenario).
 */
void InteractivePlayer::postload()
{
	Map & map = egbase().map();
	OverlayManager & overlay_manager = map.overlay_manager();
	overlay_manager.show_buildhelp(false);
	overlay_manager.register_overlay_callback_function
			(boost::bind(&int_player_overlay_callback_function, _1, boost::ref(*this)));

	// Connect buildhelp button to reflect build help state. Needs to be
	// done here rather than in the constructor as the map is not present then.
	// This code assumes that the InteractivePlayer object lives longer than
	// the overlay_manager. Otherwise remove the hook in the deconstructor.
	egbase().map().overlay_manager().onBuildHelpToggle =
		boost::bind(&UI::Button::set_perm_pressed, &m_toggle_buildhelp, _1);
	m_toggle_buildhelp.set_perm_pressed(buildhelp());

	// Recalc whole map for changed owner stuff
	map.recalc_whole_map(egbase().world());

	// Close game-relevant UI windows (but keep main menu open)
	delete m_fieldaction.window;
	m_fieldaction.window = nullptr;

	hide_minimap();
}


void InteractivePlayer::popup_message
	(Widelands::MessageId const id, const Widelands::Message & message)
{
	m_message_menu.create();
	dynamic_cast<GameMessageMenu&>(*m_message_menu.window)
	.show_new_message(id, message);
}


//  Toolbar button callback functions.
void InteractivePlayer::toggle_chat() {
	if (m_chat.window)
		delete m_chat.window;
	else if (m_chatProvider)
		GameChatMenu::create_chat_console(this, m_chat, *m_chatProvider);
}

bool InteractivePlayer::can_see(Widelands::PlayerNumber const p) const
{
	return p == player_number() || player().see_all();
}
bool InteractivePlayer::can_act(Widelands::PlayerNumber const p) const
{
	return p == player_number();
}
Widelands::PlayerNumber InteractivePlayer::player_number() const
{
	return m_player_number;
}


/// Player has clicked on the given node; bring up the context menu.
void InteractivePlayer::node_action()
{
	const Map & map = egbase().map();
	if (1 < player().vision(Map::get_index(get_sel_pos().node, map.get_width())))
	{
		// Special case for buildings
		if (upcast(Building, building, map.get_immovable(get_sel_pos().node)))
			if (can_see(building->owner().player_number()))
				return building->show_options(*this);

		if (!is_building_road()) {
			if (try_show_ship_window())
				return;
		}

		// everything else can bring up the temporary dialog
		show_field_action(this, get_player(), &m_fieldaction);
	}
}

/**
 * Global in-game keypresses:
 * \li Space: toggles buildhelp
 * \li i: show stock (inventory)
 * \li m: show minimap
 * \li o: show objectives window
 * \li c: toggle census
 * \li s: toggle building statistics
 * \li Home: go to starting position
 * \li PageUp/PageDown: change game speed
 * \li Pause: pauses the game
 * \li Return: write chat message
*/
bool InteractivePlayer::handle_key(bool const down, SDL_Keysym const code)
{
	if (down) {
		switch (code.sym) {
		case SDLK_SPACE:
			toggle_buildhelp();
			return true;

		case SDLK_i:
			m_mainm_windows.stock.toggle();
			return true;

		case SDLK_m:
			toggle_minimap();
			return true;

		case SDLK_n:
			m_message_menu.toggle();
			return true;

		case SDLK_o:
			m_objectives.toggle();
			return true;

		case SDLK_c:
			set_display_flag(dfShowCensus, !get_display_flag(dfShowCensus));
			return true;

		case SDLK_s:
			if (code.mod & (KMOD_LCTRL | KMOD_RCTRL))
				new GameMainMenuSaveGame(*this, m_mainm_windows.savegame);
			else
				set_display_flag
					(dfShowStatistics, !get_display_flag(dfShowStatistics));
			return true;

		case SDLK_KP_7:
			if (code.mod & KMOD_NUM)
				break;
			/* no break */
		case SDLK_HOME:
			move_view_to(game().map().get_starting_pos(m_player_number));
			return true;

		case SDLK_KP_ENTER:
		case SDLK_RETURN:
			if (!m_chatProvider | !m_chatenabled || !is_multiplayer())
				break;

			if (!m_chat.window)
				GameChatMenu::create_chat_console(this, m_chat, *m_chatProvider);

			return true;
		default:
			break;
		}
	}

	return InteractiveGameBase::handle_key(down, code);
}

/**
 * Set the player and the visibility to this
 * player
 */
void InteractivePlayer::set_player_number(uint32_t const n) {
	m_player_number = n;
}


/**
 * Cleanup any game-related data before loading a new game
 * while a game is currently playing.
 */
void InteractivePlayer::cleanup_for_load() {}

void InteractivePlayer::cmdSwitchPlayer(const std::vector<std::string> & args)
{
	if (args.size() != 2) {
		DebugConsole::write("Usage: switchplayer <nr>");
		return;
	}

	int const n = atoi(args[1].c_str());
	if (n < 1 || n > MAX_PLAYERS || !game().get_player(n)) {
		DebugConsole::write(str(boost::format("Player #%1% does not exist.") % n));
		return;
	}

	DebugConsole::write
		(str
			(boost::format("Switching from #%1% to #%2%.")
		 	 % static_cast<int>(m_player_number) % n));
	m_player_number = n;
	Map              &       map             = egbase().map();
	OverlayManager  &       overlay_manager = map.overlay_manager();
	Widelands::Extent  const extent          = map.extent         ();
	for (uint16_t y = 0; y < extent.h; ++y)
		for (uint16_t x = 0; x < extent.w; ++x)
			overlay_manager.recalc_field_overlays
				(map.get_fcoords(Widelands::Coords(x, y)));
	if
		(UI::UniqueWindow * const building_statistics_window =
		 	m_mainm_windows.building_stats.window)
		dynamic_cast<BuildingStatisticsMenu&>
			(*building_statistics_window)
			.update();
}
