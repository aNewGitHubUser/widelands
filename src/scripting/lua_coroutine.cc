/*
 * Copyright (C) 2006-2014 by the Widelands Development Team
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

#include "scripting/lua_coroutine.h"

#include "scripting/c_utils.h"
#include "scripting/lua_errors.h"
#include "scripting/lua_game.h"
#include "scripting/lua_map.h"
#include "scripting/luna.h"
#include "scripting/persistence.h"

namespace {

// Push a reference to this coroutine into the registry so that it will not get
// garbage collected. Returns the index of the reference in the registry.
uint32_t reference_coroutine(lua_State* L) {
	assert(L != nullptr);
	lua_pushthread(L);
	return luaL_ref(L, LUA_REGISTRYINDEX);
}

// Unreference the coroutine with the given index in the registry again. I might then get garbage
// collected.
void unreference_coroutine(lua_State* L, uint32_t idx) {
	luaL_unref(L, LUA_REGISTRYINDEX, idx);
}

}  // namespace

LuaCoroutine::LuaCoroutine(lua_State * ms)
        : m_L(ms), m_idx(LUA_REFNIL), m_ninput_args(0), m_nreturn_values(0)
{
	if (m_L) {
		m_idx = reference_coroutine(m_L);
	}
}

LuaCoroutine::~LuaCoroutine() {
	unreference_coroutine(m_L, m_idx);
}

int LuaCoroutine::get_status() {
	return lua_status(m_L);
}

int LuaCoroutine::resume()
{
        int rv = lua_resume(m_L, m_ninput_args);
        m_ninput_args = 0;
        m_nreturn_values = lua_gettop(m_L);

	if (rv != 0 && rv != YIELDED) {
		throw LuaError(lua_tostring(m_L, -1));
	}

	return rv;
}

void LuaCoroutine::push_arg(const Widelands::Player * plr) {
	to_lua<LuaGame::L_Player>(m_L, new LuaGame::L_Player(plr->player_number()));
        m_ninput_args++;
}

void LuaCoroutine::push_arg(const Widelands::Coords & coords) {
	to_lua<LuaMap::L_Field>(m_L, new LuaMap::L_Field(coords));
	++m_nargs;
        ++m_ninput_args;
}
 
void LuaCoroutine::push_arg(const Widelands::Building_Descr* building_descr) {
        assert(building_descr != nullptr);
        to_lua<LuaMap::L_BuildingDescription>(m_L, new LuaMap::L_BuildingDescription(building_descr));
        ++m_ninput_args;
}
 
std::string LuaCoroutine::pop_string() {
        if (!m_nreturn_values) {
                return "";
        }
        if (!lua_isstring(m_L, -1)) {
                throw LuaError("pop_string(), but no string on the stack.");
        }
        const std::string return_value = lua_tostring(m_L, -1);
        lua_pop(m_L, 1);
        --m_nreturn_values;
        return return_value;
}
 
uint32_t LuaCoroutine::pop_uint32() {
        if (!m_nreturn_values) {
                return 0;
        }
        if (!lua_isnumber(m_L, -1)) {
                throw LuaError("pop_string(), but no integer on the stack.");
        }
        const uint32_t return_value = luaL_checkuint32(m_L, -1);
        lua_pop(m_L, 1);
        --m_nreturn_values;
        return return_value;
}
 
 
#define COROUTINE_DATA_PACKET_VERSION 2
uint32_t LuaCoroutine::write
	(lua_State * parent, Widelands::FileWrite & fw,
	 Widelands::Map_Map_Object_Saver & mos)
{
	// Clean out the garbage before we write this.
	lua_gc(m_L, LUA_GCCOLLECT, 0);

	fw.Unsigned8(COROUTINE_DATA_PACKET_VERSION);

	// The current numbers of arguments on the stack
	fw.Unsigned32(m_nargs);
        fw.Unsigned32(m_ninput_args);
        fw.Unsigned32(m_nreturn_values);

	// Empty table + object to persist on the stack Stack
	lua_newtable(parent);
	lua_pushthread(m_L);
	lua_xmove (m_L, parent, 1);

	const uint32_t nwritten = persist_object(parent, fw, mos);

	// Clean out the garbage again.
	lua_gc(m_L, LUA_GCCOLLECT, 0);

	return nwritten;
}

void LuaCoroutine::read
	(lua_State * parent, Widelands::FileRead & fr,
	 Widelands::Map_Map_Object_Loader & mol, uint32_t size)
{
	uint8_t version = fr.Unsigned8();

	if (version > COROUTINE_DATA_PACKET_VERSION)
		throw wexception("Unknown data packet version: %i\n", version);

	// The current numbers of arguments on the stack
	m_nargs = fr.Unsigned32();
        m_ninput_args = fr.Unsigned32();
        if (version > 1) {
                m_nreturn_values = fr.Unsigned32();
        }

	// Empty table + object to persist on the stack Stack
	unpersist_object(parent, fr, mol, size);

	m_L = luaL_checkthread(parent, -1);
	lua_pop(parent, 1);

	// Cache a lua reference to this object so that it does not
	// get garbage collected
	lua_pushthread(m_L);
	m_idx = luaL_ref(m_L, LUA_REGISTRYINDEX);

	// Clean out the garbage again.
	lua_gc(m_L, LUA_GCCOLLECT, 0);
}
