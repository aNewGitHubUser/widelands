/*
 * Copyright (C) 2007-2018 by the Widelands Development Team
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

#include "map_io/map_players_view_packet.h"

#include <iostream>
#include <typeinfo>

#include <boost/format.hpp>

#include "base/log.h"
#include "base/macros.h"
#include "base/wexception.h"
#include "economy/flag.h"
#include "economy/road.h"
#include "io/fileread.h"
#include "io/filewrite.h"
#include "logic/editor_game_base.h"
#include "logic/field.h"
#include "logic/game_data_error.h"
#include "logic/map_objects/tribes/tribe_descr.h"
#include "logic/map_objects/world/world.h"
#include "logic/player.h"
namespace Widelands {

#define DIRNAME_TEMPLATE "player/%u/view"

constexpr uint8_t kCurrentPacketVersionUnseenTimes = 1;
#define UNSEEN_TIMES_FILENAME_TEMPLATE DIRNAME_TEMPLATE "/unseen_times_%u"

constexpr uint8_t kCurrentPacketVersionRoads = 2;
#define ROADS_FILENAME_TEMPLATE DIRNAME_TEMPLATE "/roads_%u"

constexpr uint8_t kCurrentPacketVersionTerrains = 2;
#define TERRAINS_FILENAME_TEMPLATE DIRNAME_TEMPLATE "/terrains_%u"

constexpr uint8_t kCurrentPacketVersionOwners = 0;
#define OWNERS_FILENAME_TEMPLATE DIRNAME_TEMPLATE "/owners_%u"

constexpr uint8_t kCurrentPacketVersionSurveys = 2;
#define SURVEYS_FILENAME_TEMPLATE DIRNAME_TEMPLATE "/surveys_%u"

constexpr uint8_t kCurrentPacketVersionSurveyAmounts = 2;
#define SURVEY_AMOUNTS_FILENAME_TEMPLATE DIRNAME_TEMPLATE "/survey_amounts_%u"

constexpr uint8_t kCurrentPacketVersionSurveyTimes = 1;
#define SURVEY_TIMES_FILENAME_TEMPLATE DIRNAME_TEMPLATE "/survey_times_%u"

constexpr uint8_t kCurrentPacketVersionHidden = 1;
#define HIDDEN_FILENAME_TEMPLATE DIRNAME_TEMPLATE "/hidden_%u"

constexpr uint8_t kCurrentPacketVersionVision = 1;
#define VISION_FILENAME_TEMPLATE DIRNAME_TEMPLATE "/vision_%u"

constexpr uint8_t kCurrentPacketVersionBorder = 1;
#define BORDER_FILENAME_TEMPLATE DIRNAME_TEMPLATE "/border_%u"

#define FILENAME_SIZE 64

enum {
	UNSEEN_NONE = 0,
	UNSEEN_TRIBEORWORLD = 1,
	UNSEEN_FLAG = 2,
	UNSEEN_BUILDING = 3,
	UNSEEN_PORTDOCK = 4
};

//  The map is traversed by row and column. In each step we process of one map
//  field (which is 1 node, 2 triangles and 3 edges that are stored together).
//  For this processing we need to keep track of 4 nodes:
//  *  f: the node of the processed field
//  * bl: the bottom left neighbour of f
//  * br: the bottom right neighbour of f
//  *  r: the right neighbour of f
//
//  The layout of the processing region is as follows:
//
//                     f------ r
//                    / \     /
//                   /  \    /
//                  / D \ R /
//                 /    \  /
//                /     \ /
//              bl------br

namespace {
#define OPEN_INPUT_FILE(filetype, file, filename, filename_template, version)                      \
	char(filename)[FILENAME_SIZE];                                                                  \
	snprintf(filename, sizeof(filename), filename_template, plnum, version);                        \
	filetype file;                                                                                  \
	try {                                                                                           \
		(file).open(fs, filename);                                                                   \
	} catch (const FileError&) {                                                                    \
		throw GameDataError("MapPlayersViewPacket::read: player %u:Could not open "                  \
		                    "\"%s\" for reading. This file should exist when \"%s\" exists",         \
		                    plnum, filename, unseen_times_filename);                                 \
	}

// Try to find the file with newest fitting version number
#define OPEN_INPUT_FILE_NEW_VERSION(filetype, file, filename, fileversion, filename_template,      \
                                    version)                                                       \
	uint8_t fileversion = version;                                                                  \
	filetype file;                                                                                  \
	char(filename)[FILENAME_SIZE];                                                                  \
	for (;; --fileversion) {                                                                        \
		snprintf(filename, sizeof(filename), filename_template, plnum, fileversion);                 \
		try {                                                                                        \
			(file).open(fs, filename);                                                                \
			break;                                                                                    \
		} catch (...) {                                                                              \
			if (fileversion == 0)                                                                     \
				throw GameDataError("MapPlayersViewPacket::read: player %u:Could not open "            \
				                    "\"%s\" for reading. This file should exist when \"%s\" exists",   \
				                    plnum, filename, unseen_times_filename);                           \
		}                                                                                            \
	}

// Using this macro, if no file exists, fileversion will be set to -1
#define OPEN_INPUT_FILE_NEW_VERSION_SILENT(filetype, file, filename, fileversion, file_templ, v)   \
	int8_t fileversion = v;                                                                         \
	filetype file;                                                                                  \
	char(filename)[FILENAME_SIZE];                                                                  \
	for (; fileversion >= -1; --fileversion) {                                                      \
		snprintf(filename, sizeof(filename), file_templ, plnum, fileversion);                        \
		try {                                                                                        \
			(file).open(fs, filename);                                                                \
			break;                                                                                    \
		} catch (...) {                                                                              \
		}                                                                                            \
	}

#define CHECK_TRAILING_BYTES(file, filename)                                                       \
	if (!(file).end_of_file())                                                                      \
		throw GameDataError(                                                                         \
		   "MapPlayersViewPacket::read: player %u:"                                                  \
		   "Found %lu trailing bytes in \"%s\"",                                                     \
		   plnum, static_cast<long unsigned int>((file).get_size() - (file).get_pos()), filename);
}  // namespace


void MapPlayersViewPacket::read(FileSystem& fs,
                                EditorGameBase& egbase,
                                bool const skip,
                                MapObjectLoader&)

{
	if (skip)
		return;

	const Map& map = egbase.map();
	const uint16_t mapwidth = map.get_width();
	const uint16_t mapheight = map.get_height();
	Field& first_field = map[0];
	const PlayerNumber nr_players = map.get_nrplayers();
	iterate_players_existing(plnum, nr_players, egbase, player) {
		Player::Field* const player_fields = player->fields_;
		uint32_t const gametime = egbase.get_gametime();

		char unseen_times_filename[FILENAME_SIZE];
		snprintf(unseen_times_filename, sizeof(unseen_times_filename), UNSEEN_TIMES_FILENAME_TEMPLATE,
		         plnum, kCurrentPacketVersionUnseenTimes);
		FileRead unseen_times_file;
		struct NotFound {};

		if (!unseen_times_file.try_open(fs, unseen_times_filename)) {
			log("MapPlayersViewPacket::read: WARNING: Could not open "
			    "\"%s\" for reading. Assuming that the game is from an old "
			    "version without player point of view. Will give player %u "
			    "knowledge of unseen nodes, edges and triangles (but not "
			    "resources).",
			    unseen_times_filename, plnum);

			for (FCoords first_in_row(Coords(0, 0), &first_field); first_in_row.y < mapheight;
			     ++first_in_row.y, first_in_row.field += mapwidth) {
				FCoords r = first_in_row, br = map.bl_n(r);
				MapIndex r_index = r.field - &first_field;
				MapIndex br_index = br.field - &first_field;
				Player::Field* r_player_field = player_fields + r_index;
				Player::Field* br_player_field = player_fields + br_index;
				Vision r_vision = r_player_field->vision;
				Vision br_vision = br_player_field->vision;
				do {
					const FCoords f = r;
					Player::Field& f_player_field = *r_player_field;
					const Vision f_vision = r_vision, bl_vision = br_vision;
					move_r(mapwidth, r, r_index);
					move_r(mapwidth, br, br_index);
					r_player_field = player_fields + r_index;
					br_player_field = player_fields + br_index;
					r_vision = r_player_field->vision;
					br_vision = br_player_field->vision;

					f_player_field.time_node_last_unseen = gametime;

					if (f_vision) {  //  node
						//  owner
						f_player_field.owner = f.field->get_owned_by();
						assert(f_player_field.owner < 0x20);

						//  map_object_descr
						const MapObjectDescr* map_object_descr;
						if (const BaseImmovable* base_immovable = f.field->get_immovable()) {
							map_object_descr = &base_immovable->descr();
							if (Road::is_road_descr(map_object_descr))
								map_object_descr = nullptr;
							else if (upcast(Building const, building, base_immovable))
								if (building->get_position() != f)
									//  TODO(unknown): This is not the building's main position
									//  so we can not see it. But it should be
									//  possible to see it from a distance somehow.
									map_object_descr = nullptr;
						} else
							map_object_descr = nullptr;
						f_player_field.map_object_descr = map_object_descr;
					}

					{  //  triangles
						//  Must be initialized because the rendering code is
						//  accessing it even for triangles that the player does not
						//  see (it is the darkening that actually hides the ground
						//  from the player).
						Field::Terrains terrains;
						terrains.d = terrains.r = 0;

						if (f_vision | bl_vision | br_vision)
							terrains.d = f.field->terrain_d();
						if (f_vision | br_vision | r_vision)
							terrains.r = f.field->terrain_r();
						f_player_field.terrains = terrains;
					}

					{  //  edges
						uint8_t mask = 0;
						if (f_vision | bl_vision)
							mask = RoadType::kMask << RoadType::kSouthWest;
						if (f_vision | br_vision)
							mask |= RoadType::kMask << RoadType::kSouthEast;
						if (f_vision | r_vision)
							mask |= RoadType::kMask << RoadType::kEast;
						f_player_field.roads = f.field->get_roads() & mask;
					}

					//  The player is not given information about resources that he
					//  has discovered, because there is no such information in old
					//  savegames, except for the resource indicators that are
					//  merely immovables with limited lifetime.
				} while (r.x);
			}
			return;
		}

		// Verify the vision values
		FileRead vision_file;
		bool have_vision = false;

		try {
			char fname[FILENAME_SIZE];
			snprintf(
			   fname, sizeof(fname), VISION_FILENAME_TEMPLATE, plnum, kCurrentPacketVersionVision);
			vision_file.open(fs, fname);
			have_vision = true;
		} catch (...) {
		}

		if (have_vision) {
			for (FCoords first_in_row(Coords(0, 0), &first_field); first_in_row.y < mapheight;
			     ++first_in_row.y, first_in_row.field += mapwidth) {
				FCoords r = first_in_row;
				MapIndex r_index = r.field - &first_field;
				Player::Field* r_player_field = player_fields + r_index;
				do {
					Player::Field& f_player_field = *r_player_field;
					move_r(mapwidth, r, r_index);
					r_player_field = player_fields + r_index;

					uint32_t file_vision = vision_file.unsigned_32();

					// There used to be a check here that the calculated, and the
					// loaded vision were the same. I removed this check, because
					// scripting could have given the player a permanent view of
					// this field. That's why we save this stuff in the first place!
					if (file_vision != f_player_field.vision)
						f_player_field.vision = file_vision;
				} while (r.x);
			}

			log("Vision check successful for player %u\n", plnum);
		}

		// Read the player's knowledge about all fields
		OPEN_INPUT_FILE_NEW_VERSION(FileRead, roads_file, roads_filename, road_file_version,
		                            ROADS_FILENAME_TEMPLATE, kCurrentPacketVersionRoads);

		OPEN_INPUT_FILE_NEW_VERSION(FileRead, terrains_file, terrains_filename, terrains_file_version,
		                            TERRAINS_FILENAME_TEMPLATE, kCurrentPacketVersionTerrains);

		OPEN_INPUT_FILE(FileRead, owners_file, owners_filename, OWNERS_FILENAME_TEMPLATE,
		                kCurrentPacketVersionOwners);

		OPEN_INPUT_FILE_NEW_VERSION(FileRead, surveys_file, surveys_filename, surveys_file_version,
		                            SURVEYS_FILENAME_TEMPLATE, kCurrentPacketVersionSurveys);

		OPEN_INPUT_FILE_NEW_VERSION(FileRead, survey_amounts_file, survey_amounts_filename,
		                            survey_amounts_file_version, SURVEY_AMOUNTS_FILENAME_TEMPLATE,
		                            kCurrentPacketVersionSurveyAmounts);

		OPEN_INPUT_FILE(FileRead, survey_times_file, survey_times_filename,
		                SURVEY_TIMES_FILENAME_TEMPLATE, kCurrentPacketVersionSurveyTimes);

		OPEN_INPUT_FILE_NEW_VERSION(FileRead, border_file, border_filename, border_file_version,
		                            BORDER_FILENAME_TEMPLATE, kCurrentPacketVersionBorder);

		OPEN_INPUT_FILE_NEW_VERSION_SILENT(FileRead, hidden_file, hidden_filename,
		                                   hidden_file_version, HIDDEN_FILENAME_TEMPLATE,
		                                   kCurrentPacketVersionHidden);

		for (FCoords first_in_row(Coords(0, 0), &first_field); first_in_row.y < mapheight;
		     ++first_in_row.y, first_in_row.field += mapwidth) {
			FCoords r = first_in_row, br = map.bl_n(r);
			MapIndex r_index = r.field - &first_field;
			MapIndex br_index = br.field - &first_field;
			Player::Field* r_player_field = player_fields + r_index;
			Player::Field* br_player_field = player_fields + br_index;
			Vision r_vision = r_player_field->vision;
			Vision br_vision = br_player_field->vision;
			bool r_everseen = r_vision, r_seen = 1 < r_vision;
			bool br_everseen = br_vision, br_seen = 1 < br_vision;
			do {
				const FCoords f = r;
				Player::Field& f_player_field = *r_player_field;
				const Vision f_vision = r_vision;
				const bool f_everseen = r_everseen;
				const bool bl_everseen = br_everseen;
				const bool f_seen = r_seen;
				const bool bl_seen = br_seen;
				move_r(mapwidth, r, r_index);
				move_r(mapwidth, br, br_index);
				r_player_field = player_fields + r_index;
				br_player_field = player_fields + br_index;
				r_vision = r_player_field->vision;
				br_vision = br_player_field->vision;
				r_everseen = r_vision;
				r_seen = 1 < r_vision;
				br_everseen = br_vision;
				br_seen = 1 < br_vision;

				//  Store the player's view of roads and ownership in these
				//  temporary variables and save it in the player when set.
				uint8_t roads = 0;
				PlayerNumber owner = 0;

				switch (f_vision) {  //  owner and map_object_descr
				case 0:
					//  The player has never seen this node, so he has no
					//  information about it. Neither should he be informed about
					//  it now.
					break;
				case 1: {
					//  The player has seen the node but does not see it now. Load
					//  his information about the node from file.
					try {
						f_player_field.time_node_last_unseen = unseen_times_file.unsigned_32();
					} catch (const FileRead::FileBoundaryExceeded&) {
						throw GameDataError(
						   "MapPlayersViewPacket::read: player %u: in "
						   "\"%s\":%lu: node (%i, %i): unexpected end of file "
						   "while reading time_node_last_unseen",
						   plnum, unseen_times_filename,
						   static_cast<long unsigned int>(unseen_times_file.get_pos() - 4), f.x, f.y);
					}

					try {
						owner = owners_file.unsigned_8();
					} catch (const FileRead::FileBoundaryExceeded&) {
						throw GameDataError(
						   "MapPlayersViewPacket::read: player %u: in "
						   "\"%s\":%lu: node (%i, %i): unexpected end of file "
						   "while reading owner",
						   plnum, unseen_times_filename,
						   static_cast<long unsigned int>(unseen_times_file.get_pos() - 1), f.x, f.y);
					}
					if (nr_players < owner) {
						throw GameDataError("MapPlayersViewPacket::read: player %u: in "
						                    "\"%s\":%lu & 0xf: node (%i, %i): Player thinks that "
						                    "this node is owned by player %u, but there are only %u "
						                    "players",
						                    plnum, owners_filename,
						                    static_cast<long unsigned int>(owners_file.get_pos() - 1),
						                    f.x, f.y, owner, nr_players);
					}
					// Read in whether this field had a border the last time it was seen
					if (border_file_version == kCurrentPacketVersionBorder) {
						uint8_t borders = border_file.unsigned_8();
						f_player_field.border = borders & 1;
						f_player_field.border_r = borders & 2;
						f_player_field.border_br = borders & 4;
						f_player_field.border_bl = borders & 8;
					} else {
						throw UnhandledVersionError("MapPlayersViewPacket - Border file",
						                            border_file_version, kCurrentPacketVersionBorder);
					}
					break;
				}
				default:
					//  The player currently sees the node. Therefore his
					//  information about the node has not been saved. Fill in the
					//  information from the game state.

					//  owner
					owner = f.field->get_owned_by();
					assert(owner <= nr_players);

					//  map_object_descr
					const MapObjectDescr* map_object_descr;
					if (const BaseImmovable* base_immovable = f.field->get_immovable()) {
						map_object_descr = &base_immovable->descr();
						if (Road::is_road_descr(map_object_descr))
							map_object_descr = nullptr;
						else if (upcast(Building const, building, base_immovable))
							if (building->get_position() != f)
								//  TODO(unknown): This is not the building's main position so
								//  we can not see it. But it should be possible
								//  to see it from a distance somehow.
								map_object_descr = nullptr;
					} else
						map_object_descr = nullptr;
					f_player_field.map_object_descr = map_object_descr;
					break;
				}

				//  triangles
				if (f_seen | bl_seen | br_seen) {
					//  The player currently sees the D triangle. Therefore his
					//  information about the triangle has not been saved. Fill in
					//  the information from the game state.
					f_player_field.terrains.d = f.field->terrain_d();
				} else if (f_everseen | bl_everseen | br_everseen) {
					//  The player has seen the D triangle but does not see it now.
					//  Load his information about the triangle from file.
					if (terrains_file_version == kCurrentPacketVersionTerrains) {
						f_player_field.terrains.d = terrains_file.unsigned_8();
					} else {
						throw UnhandledVersionError("MapPlayersViewPacket - Terrains",
						                            terrains_file_version, kCurrentPacketVersionTerrains);
					}
				}
				if (f_seen | br_seen | r_seen) {
					//  The player currently sees the R triangle. Therefore his
					//  information about the triangle has not been saved. Fill in
					//  the information from the game state.
					f_player_field.terrains.r = f.field->terrain_r();
				} else if (f_everseen | br_everseen | r_everseen) {
					//  The player has seen the R triangle but does not see it now.
					//  Load his information about the triangle from file.
					if (terrains_file_version == kCurrentPacketVersionTerrains) {
						f_player_field.terrains.r = terrains_file.unsigned_8();
					} else {
						throw UnhandledVersionError("MapPlayersViewPacket - Terrains",
						                            terrains_file_version, kCurrentPacketVersionTerrains);
					}
				}

				{  //  edges
					uint8_t mask = 0;
					if (f_seen | bl_seen) {
						mask = RoadType::kMask << RoadType::kSouthWest;
					} else if (f_everseen | bl_everseen) {
						//  The player has seen the SouthWest edge but does not see
						//  it now. Load his information about this edge from file.
						if (road_file_version == kCurrentPacketVersionRoads) {
							roads = roads_file.unsigned_8();
						} else {
							throw UnhandledVersionError("MapPlayersViewPacket - Road file",
							                            road_file_version, kCurrentPacketVersionRoads);
						}
					}
					if (f_seen | br_seen) {
						mask |= RoadType::kMask << RoadType::kSouthEast;
					} else if (f_everseen | br_everseen) {
						//  The player has seen the SouthEast edge but does not see
						//  it now. Load his information about this edge from file.
						if (road_file_version == kCurrentPacketVersionRoads) {
							roads |= roads_file.unsigned_8();
						} else {
							throw UnhandledVersionError("MapPlayersViewPacket - Road file",
							                            road_file_version, kCurrentPacketVersionRoads);
						}
					}
					if (f_seen | r_seen) {
						mask |= RoadType::kMask << RoadType::kEast;
					} else if (f_everseen | r_everseen) {
						//  The player has seen the      East edge but does not see
						//  it now. Load his information about this edge from file.
						if (road_file_version == kCurrentPacketVersionRoads) {
							roads |= roads_file.unsigned_8();
						} else {
							throw UnhandledVersionError("MapPlayersViewPacket - Road file",
							                            road_file_version, kCurrentPacketVersionRoads);
						}
					}
					roads |= f.field->get_roads() & mask;
				}

				//  Now save this information in the player field.
				f_player_field.roads = roads;
				f_player_field.owner = owner;

				//  geologic survey
				try {
					bool survey = false;
					if (surveys_file_version == kCurrentPacketVersionSurveys) {
						survey = (f_everseen & bl_everseen & br_everseen) && surveys_file.unsigned_8();
					} else {
						throw UnhandledVersionError("MapPlayersViewPacket - Surveys file",
						                            surveys_file_version, kCurrentPacketVersionSurveys);
					}
					if (survey) {
						if (survey_amounts_file_version == kCurrentPacketVersionSurveyAmounts) {
							f_player_field.resource_amounts.d = survey_amounts_file.unsigned_8();
						} else {
							throw UnhandledVersionError("MapPlayersViewPacket - Survey amounts",
							                            survey_amounts_file_version,
							                            kCurrentPacketVersionSurveyAmounts);
						}
						try {
							f_player_field
							   .time_triangle_last_surveyed[static_cast<int>(TriangleIndex::D)] =
							   survey_times_file.unsigned_32();
						} catch (const FileRead::FileBoundaryExceeded&) {
							throw GameDataError(
							   "MapPlayersViewPacket::read: player %u: in "
							   "\"%s\":%lu: node (%i, %i) t = D: unexpected end of "
							   "file while reading time_triangle_last_surveyed",
							   plnum, survey_times_filename,
							   static_cast<long unsigned int>(survey_times_file.get_pos() - 4), f.x, f.y);
						}
					}
				} catch (const FileRead::FileBoundaryExceeded&) {
					throw GameDataError("MapPlayersViewPacket::read: player %u: in \"%s\": "
					                    "node (%i, %i) t = D: unexpected end of file while reading "
					                    "survey bit",
					                    plnum, surveys_filename, f.x, f.y);
				}
				try {
					bool survey = false;
					if (surveys_file_version == kCurrentPacketVersionSurveys) {
						survey = (f_everseen & br_everseen & r_everseen) && surveys_file.unsigned_8();
					} else {
						throw UnhandledVersionError("MapPlayersViewPacket - Surveys file",
						                            surveys_file_version, kCurrentPacketVersionSurveys);
					}
					if (survey) {
						if (survey_amounts_file_version == kCurrentPacketVersionSurveyAmounts) {
							f_player_field.resource_amounts.r = survey_amounts_file.unsigned_8();
						} else {
							throw UnhandledVersionError("MapPlayersViewPacket - Survey amounts",
							                            survey_amounts_file_version,
							                            kCurrentPacketVersionSurveyAmounts);
						}
						try {
							f_player_field
							   .time_triangle_last_surveyed[static_cast<int>(TriangleIndex::R)] =
							   survey_times_file.unsigned_32();
						} catch (const FileRead::FileBoundaryExceeded&) {
							throw GameDataError(
							   "MapPlayersViewPacket::read: player %u: in "
							   "\"%s\":%lu: node (%i, %i) t = R: unexpected end of "
							   "file while reading time_triangle_last_surveyed",
							   plnum, survey_times_filename,
							   static_cast<long unsigned int>(survey_times_file.get_pos() - 4), f.x, f.y);
						}
					}
				} catch (const FileRead::FileBoundaryExceeded&) {
					throw GameDataError("MapPlayersViewPacket::read: player %u: in \"%s\": "
					                    "node (%i, %i) t = R: unexpected end of file while reading "
					                    "survey bit",
					                    plnum, surveys_filename, f.x, f.y);
				}
			} while (r.x);
		}

		// Read the number of explicitly hidden fields and then loop through them
		if (hidden_file_version == kCurrentPacketVersionHidden) {
			const uint32_t no_of_hidden_fields = hidden_file.unsigned_32();
			for (uint32_t i = 0; i < no_of_hidden_fields; ++i) {
				player->hidden_fields_.insert(
				   std::make_pair(hidden_file.unsigned_32(), hidden_file.unsigned_16()));
			}
		} else {
			throw UnhandledVersionError("MapPlayersViewPacket - Hidden fields file",
			                            hidden_file_version, kCurrentPacketVersionHidden);
		}

		CHECK_TRAILING_BYTES(unseen_times_file, unseen_times_filename);
		CHECK_TRAILING_BYTES(roads_file, roads_filename);
		CHECK_TRAILING_BYTES(terrains_file, terrains_filename);
		CHECK_TRAILING_BYTES(owners_file, owners_filename);
		CHECK_TRAILING_BYTES(surveys_file, surveys_filename);
		CHECK_TRAILING_BYTES(survey_amounts_file, survey_amounts_filename);
		CHECK_TRAILING_BYTES(survey_times_file, survey_times_filename);
		CHECK_TRAILING_BYTES(border_file, border_filename);
		CHECK_TRAILING_BYTES(hidden_file, hidden_filename);
	}
}

#define WRITE(file, filename_template, version)                                                    \
	snprintf(filename, sizeof(filename), filename_template, plnum, version);                        \
	(file).write(fs, filename);

void MapPlayersViewPacket::write(FileSystem& fs, EditorGameBase& egbase, MapObjectSaver&) {
	fs.ensure_directory_exists("player");
	const Map& map = egbase.map();
	const uint16_t mapwidth = map.get_width();
	const uint16_t mapheight = map.get_height();
	// TODO(unknown): make first_field const when FCoords has been templatized so it can
	// have "const Field * field;"
	Field& first_field = map[0];
	const PlayerNumber nr_players = map.get_nrplayers();
	iterate_players_existing_const(
	   plnum, nr_players, egbase,
	   player) if (const Player::Field* const player_fields = player->fields_) {
		FileWrite unseen_times_file;
		FileWrite roads_file;
		FileWrite terrains_file;
		FileWrite owners_file;
		FileWrite surveys_file;
		FileWrite survey_amounts_file;
		FileWrite survey_times_file;
		FileWrite hidden_file;
		FileWrite vision_file;
		FileWrite border_file;
		for (FCoords first_in_row(Coords(0, 0), &first_field); first_in_row.y < mapheight;
		     ++first_in_row.y, first_in_row.field += mapwidth) {
			FCoords r = first_in_row, br = map.bl_n(r);
			MapIndex r_index = r.field - &first_field;
			MapIndex br_index = br.field - &first_field;
			const Player::Field* r_player_field = player_fields + r_index;
			const Player::Field* br_player_field = player_fields + br_index;
			Vision r_vision = r_player_field->vision;
			Vision br_vision = br_player_field->vision;
			bool r_everseen = r_vision, r_seen = 1 < r_vision;
			bool br_everseen = br_vision, br_seen = 1 < br_vision;
			do {
				const Player::Field& f_player_field = *r_player_field;
				const bool f_everseen = r_everseen;
				const bool bl_everseen = br_everseen;
				const bool f_seen = r_seen;
				const bool bl_seen = br_seen;
				move_r(mapwidth, r, r_index);
				move_r(mapwidth, br, br_index);
				r_player_field = player_fields + r_index;
				br_player_field = player_fields + br_index;
				r_vision = r_player_field->vision;
				br_vision = br_player_field->vision;
				r_everseen = r_vision;
				r_seen = 1 < r_vision;
				br_everseen = br_vision;
				br_seen = 1 < br_vision;

				vision_file.unsigned_32(f_player_field.vision);

				if (!f_seen) {

					if (f_everseen) {  //  node
						unseen_times_file.unsigned_32(f_player_field.time_node_last_unseen);
						assert(f_player_field.owner < 0x20);
						owners_file.unsigned_8(f_player_field.owner);

						// write whether this field had a border the last time it was seen
						uint8_t borders = 0;
						borders |= f_player_field.border;
						borders |= f_player_field.border_r << 1;
						borders |= f_player_field.border_br << 2;
						borders |= f_player_field.border_bl << 3;
						border_file.unsigned_8(borders);
					}

					//  triangles
					if
					   //  the player does not see the D triangle now but has
					   //  seen it
					   ((!bl_seen) & (!br_seen) & (f_everseen | bl_everseen | br_everseen)) {
						terrains_file.unsigned_8(f_player_field.terrains.d);
					}
					if
					   //  the player does not see the R triangle now but has
					   //  seen it
					   ((!br_seen) & (!r_seen) & (f_everseen | br_everseen | r_everseen)) {
						terrains_file.unsigned_8(f_player_field.terrains.r);
					}

					//  edges
					if ((!bl_seen) && (f_everseen || bl_everseen))
						roads_file.unsigned_8(f_player_field.road_sw());
					if ((!br_seen) && (f_everseen || br_everseen))
						roads_file.unsigned_8(f_player_field.road_se());
					if ((!r_seen) && (f_everseen || r_everseen))
						roads_file.unsigned_8(f_player_field.road_e());
				}

				//  geologic survey
				if (f_everseen & bl_everseen & br_everseen) {
					const uint32_t time_last_surveyed =
					   f_player_field.time_triangle_last_surveyed[static_cast<int>(TriangleIndex::D)];
					const uint8_t has_info = time_last_surveyed != 0xffffffff;
					surveys_file.unsigned_8(has_info);
					if (has_info) {
						survey_amounts_file.unsigned_8(f_player_field.resource_amounts.d);
						survey_times_file.unsigned_32(time_last_surveyed);
					}
				}
				if (f_everseen & br_everseen & r_everseen) {
					const uint32_t time_last_surveyed =
					   f_player_field.time_triangle_last_surveyed[static_cast<int>(TriangleIndex::R)];
					const uint8_t has_info = time_last_surveyed != 0xffffffff;
					surveys_file.unsigned_8(has_info);
					if (has_info) {
						survey_amounts_file.unsigned_8(f_player_field.resource_amounts.r);
						survey_times_file.unsigned_32(time_last_surveyed);
					}
				}
			} while (r.x);
		}
		// Write the number of explicitly hidden fields and then loop through them
		hidden_file.unsigned_32(player->hidden_fields_.size());
		for (const auto& hidden : player->hidden_fields_) {
			hidden_file.unsigned_32(hidden.first);
			hidden_file.unsigned_16(hidden.second);
		}

		char filename[FILENAME_SIZE];

		fs.ensure_directory_exists(
		   (boost::format(DIRNAME_TEMPLATE) % static_cast<unsigned int>(plnum)).str());

		WRITE(unseen_times_file, UNSEEN_TIMES_FILENAME_TEMPLATE, kCurrentPacketVersionUnseenTimes);

		WRITE(roads_file, ROADS_FILENAME_TEMPLATE, kCurrentPacketVersionRoads);

		WRITE(terrains_file, TERRAINS_FILENAME_TEMPLATE, kCurrentPacketVersionTerrains);

		WRITE(owners_file, OWNERS_FILENAME_TEMPLATE, kCurrentPacketVersionOwners);

		WRITE(surveys_file, SURVEYS_FILENAME_TEMPLATE, kCurrentPacketVersionSurveys);

		WRITE(
		   survey_amounts_file, SURVEY_AMOUNTS_FILENAME_TEMPLATE, kCurrentPacketVersionSurveyAmounts);

		WRITE(survey_times_file, SURVEY_TIMES_FILENAME_TEMPLATE, kCurrentPacketVersionSurveyTimes);

		WRITE(hidden_file, HIDDEN_FILENAME_TEMPLATE, kCurrentPacketVersionHidden);

		WRITE(vision_file, VISION_FILENAME_TEMPLATE, kCurrentPacketVersionVision);

		WRITE(border_file, BORDER_FILENAME_TEMPLATE, kCurrentPacketVersionBorder);
	}
}
}
