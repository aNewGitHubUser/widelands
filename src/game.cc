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

#include "game.h"

#include "cmd_check_eventchain.h"
#include "computer_player.h"
#include "events/event.h"
#include "events/event_chain.h"
#include "interactive_player.h"
#include "interactive_spectator.h"
#include "fullscreen_menu_launchgame.h"
#include "fullscreen_menu_loadgame.h"
#include "fullscreen_menu_loadreplay.h"
#include "game_loader.h"
#include "game_tips.h"
#include "gamecontroller.h"
#include "graphic.h"
#include "i18n.h"
#include "layered_filesystem.h"
#include "network.h"
#include "player.h"
#include "playercommand.h"
#include "productionsite.h"
#include "replay.h"
#include "soldier.h"
#include "sound/sound_handler.h"
#include "tribe.h"
#include "widelands_fileread.h"
#include "widelands_filewrite.h"
#include "widelands_map_loader.h"
#include "wlapplication.h"

#include "ui_progresswindow.h"

#include "log.h"

#include "upcast.h"

#include "timestring.h"

#include <string>
#include <cstring>

namespace Widelands {

/// Define this to get lots of debugging output concerned with syncs
//#define SYNC_DEBUG

#ifdef SYNC_DEBUG
struct SyncDebugWrapper : public StreamWrite {
	SyncDebugWrapper(Game* g, StreamWrite* target)
	{
		m_game = g;
		m_target = target;
		m_counter = 0;
	}

	void Data(const void * const data, const size_t size)
	{
		assert(m_target);

		log("[sync:%08u t=%6u]", m_counter, m_game->get_gametime());
		for (size_t i = 0; i < size; ++i)
			log(" %02x", (static_cast<uint8_t const *>(data))[i]);
		log("\n");

		m_target->Data(data, size);
		m_counter += size;
	}

	void Flush()
	{
		assert(m_target);

		m_target->Flush();
	}

public:
	Game* m_game;
	StreamWrite* m_target;
	uint32_t m_counter;
};
#endif

struct GameInternals {
	MD5Checksum<StreamWrite> synchash;
#ifdef SYNC_DEBUG
	SyncDebugWrapper syncwrapper;
#endif
	GameController* ctrl;

	GameInternals(Game* g)
#ifdef SYNC_DEBUG
		: syncwrapper(g, &synchash)
#endif
	{
		static_cast<void>(g);
	}

	void SyncReset()
	{
#ifdef SYNC_DEBUG
		syncwrapper.m_counter = 0;
#endif

		synchash.Reset();
		log("[sync] Reset\n");
	}
};


Game::Game() :
m(new GameInternals(this)),
m_state   (gs_notrunning),
m_speed   (1),
cmdqueue  (this),
m_replayreader(0),
m_replaywriter(0),
m_realtime(WLApplication::get()->get_time())
{
	m->ctrl = 0;
	g_sound_handler.m_the_game = this;
	m_last_stats_update = 0;
	m_player_cmdserial = 0;
}

Game::~Game()
{
	if (m_replayreader) {
		delete m_replayreader;
		m_replayreader = 0;
	}
	if (m_replaywriter) {
		delete m_replaywriter;
		m_replaywriter = 0;
	}
	g_sound_handler.m_the_game = NULL;

	delete m;
	m = 0;
}


/**
 * Returns true if cheat codes have been activated (single-player only)
 */
bool Game::get_allow_cheats()
{
	return true;
}


/**
 * \return a pointer to the \ref Interactive_Player if any.
 * \note This function may return 0 (in particular, it will return 0 during
 * playback)
 */
Interactive_Player* Game::get_ipl()
{
	return dynamic_cast<Interactive_Player*>(get_iabase());
}


void Game::set_game_controller(GameController* ctrl)
{
	m->ctrl = ctrl;
}


bool Game::run_splayer_map_direct(const char* mapname, bool scenario) {
	assert(!get_map());

	set_map(new Map);

	FileSystem* fs = g_fs->MakeSubFileSystem(mapname);
	m_maploader = new WL_Map_Loader(*fs, &map());
	UI::ProgressWindow loaderUI;
	GameTips tips (loaderUI);

	// Loading the locals for the campaign
	if (scenario) {
		loaderUI.step (_("Preloading a map")); // Must be printed before loading the scenarios textdomain, else it won't be translated.
		i18n::Textdomain textdomain(mapname);
		log("Loading the locals for scenario. file: %s.mo\n", mapname);
		m_maploader->preload_map(scenario);
	} else {
		//  We are not loading a scenario, so no ingame texts to be translated.
		loaderUI.step (_("Preloading a map"));
		m_maploader->preload_map(scenario);
	}

	const std::string background = map().get_background();
	if (background.size() > 0)
		loaderUI.set_background(background);
	loaderUI.step (_("Loading a world"));
	m_maploader->load_world();

	// We have to create the players here.
	const Player_Number nr_players = map().get_nrplayers();
	iterate_player_numbers(p, nr_players) {
		loaderUI.stepf (_("Adding player %u"), p);
		add_player
			(p,
			 p == 1 ? Player::Human : Player::AI, //  FIXME allow the user to contorol another player than 1, and allow the computer to control player 1
			 map().get_scenario_player_tribe(p),
			 map().get_scenario_player_name (p));
	}

	set_iabase(new Interactive_Player(*this, 1));

	loaderUI.step (_("Loading a map")); // Must be printed before loading the scenarios textdomain, else it won't be translated.

	// Reload campaign textdomain
	if (scenario) {
		i18n::Textdomain textdomain(mapname);
		m_maploader->load_map_complete(this, true);
	} else
		m_maploader->load_map_complete(this, false);
	delete m_maploader; m_maploader = 0;

	return run(loaderUI);
}


/**
 * Initialize the game based on the given settings.
 */
void Game::init(UI::ProgressWindow & loaderUI, const GameSettings& settings) {
	g_gr->flush(PicMod_Menu);

	loaderUI.step(_("Preloading map"));

	assert(!get_map());
	set_map(new Map);

	m_maploader = map().get_correct_loader(settings.mapfilename.c_str());
	m_maploader->preload_map(false);

	loaderUI.step(_("Configuring players"));
	for (uint32_t i = 0; i < settings.players.size(); ++i) {
		const PlayerSettings& player = settings.players[i];

		if
			(player.state == PlayerSettings::stateClosed ||
			 player.state == PlayerSettings::stateOpen)
			continue;

		int32_t type = Player::Human;

		if (player.state == PlayerSettings::stateComputer)
			type = Player::AI;

		add_player(i+1, type, player.tribe, player.name);
		get_player(i+1)->init(false);
	}

	loaderUI.step(_("Loading map"));
	m_maploader->load_map_complete(this, false); // if code==2 is a scenario
	delete m_maploader;
	m_maploader = 0;
}


/**
 * Load a game
 * argument defines if this is a single player game (true)
 * or networked (false)
 */
bool Game::run_load_game(const bool is_splayer, std::string filename) {
	assert(is_splayer); // TODO: net game saving not supported

	if (filename.empty()) {
		Fullscreen_Menu_LoadGame ssg(*this);
		if (ssg.run() > 0)
			filename = ssg.filename();
		else
			return false;
	}

	UI::ProgressWindow loaderUI;
	GameTips tips (loaderUI);

	// We have to create an empty map, otherwise nothing will load properly
	set_map(new Map);

	{
		std::auto_ptr<FileSystem> const fs
			(g_fs->MakeSubFileSystem(filename.c_str()));

		set_iabase(new Interactive_Player(*this, 1)); //  FIXME memory leak!!!

		Game_Loader gl(*fs, this);
		loaderUI.step(_("Loading..."));
		gl.load_game();
	}

	return run(loaderUI, true);
}

/**
 * Display the fullscreen menu to choose a replay,
 * then start the interactive replay.
 */
bool Game::run_replay()
{
	Fullscreen_Menu_LoadReplay rm(this);

	if (rm.run() <= 0)
		return false;

	log("Selected replay: %s\n", rm.filename().c_str());

	UI::ProgressWindow loaderUI;
	GameTips tips (loaderUI);

	// We have to create an empty map, otherwise nothing will load properly
	set_map(new Map);

	set_iabase(new Interactive_Spectator(this)); //  FIXME memory leak???

	loaderUI.step(_("Loading..."));

	m_replayreader = new ReplayReader(*this, rm.filename());

	return run(loaderUI, true);
}


void Game::load_map (const char* filename)
{
	m_maploader = (new Map())->get_correct_loader(filename);
	assert (m_maploader!=0);
	m_maploader->preload_map(0);
	set_map (&m_maploader->map());
}


/**
 * Called for every game after loading (from a savegame or just from a map
 * during single/multiplayer/scenario).
 *
 * Ensure that players and player controllers are setup properly (in particular
 * AI and the \ref Interactive_Player if any).
 */
void Game::postload()
{
	Editor_Game_Base::postload();

	assert(get_iabase() != 0);

	// Set up computer controlled players
	// unless we're watching a replay
	if (upcast(Interactive_Player, ipl, get_iabase())) {
		const Player_Number nr_players = map().get_nrplayers();
		iterate_players_existing(p, nr_players, *this, plr) {
			if      (plr->get_type() == Player::AI)
				cpl.push_back (new Computer_Player(*this, p));
		}
	}

	get_iabase()->postload();
}


/**
 * This runs a game, including game creation phase.
 *
 * The setup and loading of a game happens (or rather: will happen) in three
 * stages.
 * 1.  First of all, the host (or single player) configures the game. During this
 *     time, only short descriptions of the game data (such as map headers)are
 *     loaded to minimize loading times.
 * 2a. Once the game is about to start and the configuration screen is finished,
 *     all logic data (map, tribe information, building information) is loaded
 *     during postload.
 * 2b. If a game is created, initial player positions are set. This step is
 *     skipped when a game is loaded.
 * 3.  After this has happened, the game graphics are loaded.
 *
 * \return true if a game actually took place, false otherwise
 */
bool Game::run(UI::ProgressWindow & loader_ui, bool is_savegame) {
	postload();

	if (not is_savegame) {
		std::string step_description = _("Creating player infrastructure");
		// Prepare the players (i.e. place HQs)
		const Player_Number nr_players = map().get_nrplayers();
		iterate_players_existing(p, nr_players, *this, plr) {
			step_description += ".";
			loader_ui.step(step_description);
			plr->init(true);
		}

		if (get_ipl())
			get_ipl()->move_view_to(map().get_starting_pos(get_ipl()->get_player_number()));

		// Prepare the map, set default textures
		map().recalc_default_resources();
		map().mem().remove_unreferenced();
		map().mtm().remove_unreferenced();

		// Finally, set the scenario names and tribes to represent
		// the correct names of the players
		iterate_player_numbers(p, nr_players) {
			const Player * const plr = get_player(p);
			const std::string                                             no_name;
			const std::string &  tribe_name = plr ? plr->tribe().name() : no_name;
			const std::string & player_name = plr ? plr->    get_name() : no_name;
			map().set_scenario_player_tribe(p,  tribe_name);
			map().set_scenario_player_name (p, player_name);
		}

		// Everything prepared, send the first trigger event
		// We lie about the sender here. Hey, what is one lie in a lifetime?
		enqueue_command (new Cmd_CheckEventChain(get_gametime(), -1));
	}

	if (!m_replayreader) {
		log("Starting replay writer\n");

		// Derive a replay filename from the current time
		std::string fname(REPLAY_DIR);
		fname += '/';
		fname += timestring();
		if (m->ctrl)
			fname += ' ' + m->ctrl->getGameDescription();
		fname += REPLAY_SUFFIX;

		m_replaywriter = new ReplayWriter(*this, fname);
		log("Replay writer has started\n");
	}

	m->SyncReset();

	load_graphics(loader_ui);

	g_sound_handler.change_music("ingame", 1000, 0);

	m_state = gs_running;

	get_iabase()->run();

	g_sound_handler.change_music("menu", 1000, 0);

	cleanup_objects();
	delete get_iabase();
	set_iabase(0);

	for (uint32_t i=0; i<cpl.size(); ++i)
		delete cpl[i];

	g_gr->flush(PicMod_Game);
	g_anim.flush();

	m_state = gs_notrunning;

	return true;
}


/**
 * think() is called by the UI objects initiated during Game::run()
 * during their modal loop.
 * Depending on the current state we advance game logic and stuff,
 * running the cmd queue etc.
 */
void Game::think()
{
	if (m->ctrl)
		m->ctrl->think();

	if (m_state == gs_running) {
		for (uint32_t i = 0;i < cpl.size(); ++i)
			cpl[i]->think();

		if
			(not m_general_stats.size()
			 or
			 get_gametime() - m_last_stats_update > STATISTICS_SAMPLE_TIME)
		{
			sample_statistics();

			const Player_Number nr_players = map().get_nrplayers();
			iterate_players_existing(p, nr_players, *this, plr)
				plr->sample_statistics();

			m_last_stats_update = get_gametime();
		}

		int32_t frametime;

		if (m->ctrl) {
			frametime = m->ctrl->getFrametime();
		} else {
			frametime = -m_realtime;
			m_realtime =  WLApplication::get()->get_time();
			frametime += m_realtime;

			frametime *= get_speed();

			// Maybe we are too fast...
			// Note that the time reported by WLApplication might jump backwards
			// when playback stops.
			if (frametime <= 0)
				return;

			// prevent frametime escalation in case the game logic is the performance bottleneck
			if (frametime > 1000)
				frametime = 1000;
		}

		if (m_replayreader) {
			for (;;) {
				Command* cmd = m_replayreader->GetNextCommand(get_gametime() + frametime);
				if (!cmd)
					break;

				enqueue_command(cmd);
			}

			if (m_replayreader->EndOfReplay())
				dynamic_cast<Interactive_Spectator*>(get_iabase())->end_of_game();
		}

		cmdqueue.run_queue(frametime, get_game_time_pointer());

		g_gr->animate_maptextures(get_gametime());

		// check if autosave is needed
		m_savehandler.think(*this, m_realtime);
	}
}


/**
 * Cleanup for load
 * \deprecated
 * \todo Get rid of this. Prefer to delete and recreate Game-style objects
 * Note that this needs fixes in the editor.
 */
void Game::cleanup_for_load
(const bool flush_graphics, const bool flush_animations)
{
	m_state = gs_notrunning;

	Editor_Game_Base::cleanup_for_load(flush_graphics, flush_animations);
	for
		(std::vector<Tribe_Descr*>::iterator it = m_tribes.begin();
		 it != m_tribes.end();
		 ++it)
		delete *it;
	m_tribes.clear();
	get_cmdqueue()->flush();
	while (cpl.size()) {
		delete cpl[cpl.size()-1];
		cpl.pop_back();
	}

	// Statistics
	m_last_stats_update = 0;
	m_general_stats.clear();
}


/**
 * Game logic code may write to the synchronization
 * token stream. All written data will be hashed and can be used to
 * check for network or replay desyncs.
 *
 * \return the synchronization token stream
 *
 * \note This is returned as a \ref StreamWrite object to prevent
 * the caller from messing with the checksumming process.
 */
StreamWrite & Game::syncstream()
{
#ifdef SYNC_DEBUG
	return m->syncwrapper;
#else
	return m->synchash;
#endif
}


/**
 * Calculate the current synchronization checksum and copy
 * it into the given array, without affecting the subsequent
 * checksumming process.
 *
 * \return the checksum
 */
md5_checksum Game::get_sync_hash() const
{
	MD5Checksum<StreamWrite> copy(m->synchash);

	copy.FinishChecksum();
	return copy.GetChecksum();
}


/**
 * Return a random value that can be used in parallel game logic
 * simulation.
 *
 * \note Do NOT use for random events in the UI or other display code.
 */
uint32_t Game::logic_rand()
{
	uint32_t r = rng.rand();
	syncstream().Unsigned32(r);
	return r;
}


/**
 * All player-issued commands must enter the queue through this function.
 * It takes the appropriate action, i.e. either add to the cmd_queue or send
 * across the network.
 */
void Game::send_player_command (PlayerCommand* pc)
{
	if (m->ctrl) {
		m->ctrl->sendPlayerCommand(pc);
		return;
	}

	pc->set_cmdserial(++m_player_cmdserial);
	enqueue_command (pc);
}


/**
 * Actually enqueue a command.
 *
 * \note In a network game, player commands are only allowed to enter the
 * command queue after being accepted by the networking logic via
 * \ref send_player_command, so you must never enqueue a player command directly.
 */
void Game::enqueue_command (Command * const cmd)
{
	if (m_replaywriter)
		if (upcast(PlayerCommand, plcmd, cmd))
			m_replaywriter->SendPlayerCommand(plcmd);

	cmdqueue.enqueue(cmd);
}

// we might want to make these inlines:
void Game::send_player_bulldoze (PlayerImmovable* pi)
{
	send_player_command (new Cmd_Bulldoze(get_gametime(), pi->get_owner()->get_player_number(), pi));
}

void Game::send_player_build (int32_t pid, const Coords& coords, int32_t id)
{
	send_player_command (new Cmd_Build(get_gametime(), pid, coords, id));
}

void Game::send_player_build_flag (int32_t pid, const Coords& coords)
{
	send_player_command (new Cmd_BuildFlag(get_gametime(), pid, coords));
}

void Game::send_player_build_road (int32_t pid, Path & path)
{
	send_player_command (new Cmd_BuildRoad(get_gametime(), pid, path));
}

void Game::send_player_flagaction (Flag* flag, int32_t action)
{
	send_player_command
		(new Cmd_FlagAction
		 (get_gametime(), flag->get_owner()->get_player_number(), flag, action));
}

void Game::send_player_start_stop_building (Building* b)
{
	send_player_command
		(new Cmd_StartStopBuilding
		 (get_gametime(), b->get_owner()->get_player_number(), b));
}

void Game::send_player_enhance_building (Building* b, int32_t id)
{
	assert(id!=-1);

	send_player_command
		(new Cmd_EnhanceBuilding
		 (get_gametime(), b->get_owner()->get_player_number(), b, id));
}

void Game::send_player_change_training_options(Building* b, int32_t atr, int32_t val)
{
	send_player_command
		(new Cmd_ChangeTrainingOptions
		 (get_gametime(), b->get_owner()->get_player_number(), b, atr, val));
}

void Game::send_player_drop_soldier (Building* b, int32_t ser)
{
	assert(ser != -1);
	send_player_command
		(new Cmd_DropSoldier
		 (get_gametime(), b->get_owner()->get_player_number(), b, ser));
}

void Game::send_player_change_soldier_capacity (Building* b, int32_t val)
{
	send_player_command
		(new Cmd_ChangeSoldierCapacity
		 (get_gametime(),
		  b->get_owner()->get_player_number(),
		  b,
		  val));
}

/////////////////////// TESTING STUFF
void Game::send_player_enemyflagaction
(const Flag * const flag,
 const int32_t action,
 const Player_Number who_attacks,
 const int32_t num_soldiers,
 const int32_t type)
{
	send_player_command
		(new Cmd_EnemyFlagAction
		 (get_gametime(),
		  who_attacks,
		  flag,
		  action,
		  who_attacks,
		  num_soldiers,
		  type));
}


/**
 * Sample global statistics for the game.
 */
void Game::sample_statistics()
{
	// Update general stats
	std::vector< uint32_t > land_size; land_size.resize(map().get_nrplayers());
	std::vector< uint32_t > nr_buildings; nr_buildings.resize(map().get_nrplayers());
	std::vector< uint32_t > nr_kills; nr_kills.resize(map().get_nrplayers());
	std::vector< uint32_t > miltary_strength; miltary_strength.resize(map().get_nrplayers());
	std::vector< uint32_t > nr_workers; nr_workers.resize(map().get_nrplayers());
	std::vector< uint32_t > nr_wares; nr_wares.resize(map().get_nrplayers());
	std::vector< uint32_t > productivity; productivity.resize(map().get_nrplayers());

	std::vector< uint32_t > nr_production_sites; nr_production_sites.resize(map().get_nrplayers());

	//  We walk the map, to gain all needed information.
	Map const &  themap = map();
	Extent const extent = themap.extent();
	iterate_Map_FCoords(themap, extent, fc) {
		if (fc.field->get_owned_by())
			++land_size[fc.field->get_owned_by() - 1];

			// Get the immovable
		if (upcast(Building, building, fc.field->get_immovable()))
			if (building->get_position() == fc) { // only count main location
				uint8_t const player_index =
					building->owner().get_player_number() - 1;
				++nr_buildings[player_index];

				//  If it is a productionsite, add its productivity.
				if (upcast(ProductionSite, productionsite, building)) {
					++nr_production_sites[player_index];
					productivity[player_index] +=
						productionsite->get_statistics_percent();
				}
			}

			// Now, walk the bobs
		for (Bob const * b = fc.field->get_first_bob(); b; b = b->get_next_bob())
			if (upcast(Soldier const, s, b))
				miltary_strength[s->get_owner()->get_player_number() - 1] +=
					s->get_level(atrTotal) + 1; //  So that level 0 also counts.
	}

	// Number of workers / wares
	const Player_Number nr_players = map().get_nrplayers();
	iterate_players_existing(p, nr_players, *this, plr) {
		uint32_t wostock = 0;
		uint32_t wastock = 0;

		for (uint32_t j = 0; j < plr->get_nr_economies(); ++j) {
			Economy* eco = plr->get_economy_by_number(j);
			const Tribe_Descr & tribe = plr->tribe();
			for (int32_t wareid = 0; wareid < tribe.get_nrwares(); ++wareid)
				wastock += eco->stock_ware(wareid);
			for (int32_t workerid = 0; workerid < tribe.get_nrworkers(); ++workerid) {
				if (tribe.get_worker_descr(workerid)->get_worker_type() == Worker_Descr::CARRIER)
					continue;
				wostock += eco->stock_worker(workerid);
			}
		}
		nr_wares  [p - 1] = wastock;
		nr_workers[p - 1] = wostock;
	}

	// Now, divide the statistics
	for (uint32_t i = 0; i < map().get_nrplayers(); ++i) {
		if (productivity[i])
			productivity[i] /= nr_production_sites[i];
	}

	// Now, push this on the general statistics
	m_general_stats.resize(map().get_nrplayers());
	for (uint32_t i = 0; i < map().get_nrplayers(); ++i) {
		m_general_stats[i].land_size.push_back(land_size[i]);
		m_general_stats[i].nr_buildings.push_back(nr_buildings[i]);
		m_general_stats[i].nr_kills.push_back(nr_kills[i]);
		m_general_stats[i].miltary_strength.push_back(miltary_strength[i]);
		m_general_stats[i].nr_workers.push_back(nr_workers[i]);
		m_general_stats[i].nr_wares.push_back(nr_wares[i]);
		m_general_stats[i].productivity.push_back(productivity[i]);
	}
}


/**
 * Read statistics data from a file.
 *
 * \param fr UNDOCUMENTED
 * \param version indicates the kind of statistics file, which may be
 *   0 - old style statistics (from the time when statistics were kept in
 *       Interactive_Player)
 *   1 - current version
 *
 * \todo Document parameter fr
 * \todo Would it make sense to not support the old style anymore?
 */
void Game::ReadStatistics(FileRead& fr, uint32_t version)
{
	if (version == 0 || version == 1) {
		if (version >= 1) {
			m_last_stats_update = fr.Unsigned32();
		}

		// Read general statistics
		uint32_t entries = fr.Unsigned16();
		const Player_Number nr_players = map().get_nrplayers();
		m_general_stats.resize(nr_players);

		iterate_players_existing(p, nr_players, *this, plr) {
			m_general_stats[p - 1].land_size       .resize(entries);
			m_general_stats[p - 1].nr_workers      .resize(entries);
			m_general_stats[p - 1].nr_buildings    .resize(entries);
			m_general_stats[p - 1].nr_wares        .resize(entries);
			m_general_stats[p - 1].productivity    .resize(entries);
			m_general_stats[p - 1].nr_kills        .resize(entries);
			m_general_stats[p - 1].miltary_strength.resize(entries);
		}

		iterate_players_existing(p, nr_players, *this, plr)
			for (uint32_t j = 0; j < m_general_stats[p - 1].land_size.size(); ++j)
			{
				m_general_stats[p - 1].land_size       [j] = fr.Unsigned32();
				m_general_stats[p - 1].nr_workers      [j] = fr.Unsigned32();
				m_general_stats[p - 1].nr_buildings    [j] = fr.Unsigned32();
				m_general_stats[p - 1].nr_wares        [j] = fr.Unsigned32();
				m_general_stats[p - 1].productivity    [j] = fr.Unsigned32();
				m_general_stats[p - 1].nr_kills        [j] = fr.Unsigned32();
				m_general_stats[p - 1].miltary_strength[j] = fr.Unsigned32();
			}
	} else
		throw wexception("Unsupported version %i", version);
}


/**
 * Write general statistics to the given file.
 */
void Game::WriteStatistics(FileWrite& fw)
{
	fw.Unsigned32(m_last_stats_update);

	// General statistics
	// First, we write the size of the statistics arrays
	uint32_t entries = 0;

	const Player_Number nr_players = map().get_nrplayers();
	iterate_players_existing(p, nr_players, *this, plr)
		if (m_general_stats.size()) {
			entries = m_general_stats[p - 1].land_size.size();
			break;
		}

	fw.Unsigned16(entries);

	iterate_players_existing(p, nr_players, *this, plr)
		for (uint32_t j = 0; j < entries; ++j) {
			fw.Unsigned32(m_general_stats[p - 1].land_size       [j]);
			fw.Unsigned32(m_general_stats[p - 1].nr_workers      [j]);
			fw.Unsigned32(m_general_stats[p - 1].nr_buildings    [j]);
			fw.Unsigned32(m_general_stats[p - 1].nr_wares        [j]);
			fw.Unsigned32(m_general_stats[p - 1].productivity    [j]);
			fw.Unsigned32(m_general_stats[p - 1].nr_kills        [j]);
			fw.Unsigned32(m_general_stats[p - 1].miltary_strength[j]);
		}
}

};
