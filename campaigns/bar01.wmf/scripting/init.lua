-- =======================================================================
--                       Barbarians Campaign Mission 1
-- =======================================================================

set_textdomain("scenario_bar01.wmf")

include "scripting/coroutine.lua"
include "scripting/objective_utils.lua"
include "scripting/infrastructure.lua"

-- ==========
-- Constants
-- ==========
plr = wl.Game().players[1]
map = wl.Game().map

home = map:get_field(12,10)
al_thunran = map:get_field(5,46)
grave = map:get_field(24,28)

-- ===============
-- Initialization
-- ===============
include "map:scripting/starting_conditions.lua"


-- ==============
-- Mission Logic
-- ==============
include "map:scripting/mission_thread.lua"
