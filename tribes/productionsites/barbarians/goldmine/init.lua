dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   name = "barbarians_goldmine",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = _"Gold Mine",
   size = "mine",
   enhancement = "barbarians_goldmine_deep",

   buildcost = {
		log = 4,
		granite = 2
	},
	return_on_dismantle = {
		log = 2,
		granite = 1
	},

	-- TRANSLATORS: Helptext for a building: Gold Mine
   helptext = "", -- NOCOM(GunChleoc): See what we can shift over from help.lua here

   animations = {
		idle = {
			pictures = { dirname .. "idle_\\d+.png" },
			hotspot = { 21, 36 },
		},
		build = {
			pictures = { dirname .. "build_\\d+.png" },
			hotspot = { 21, 36 },
		},
		working = {
			pictures = { dirname .. "working_\\d+.png" },
			hotspot = { 21, 36 },
		},
		empty = {
			pictures = { dirname .. "empty_\\d+.png" },
			hotspot = { 21, 36 },
		},
	},

   aihints = {
		mines = "gold",
		mines_percent = 30,
		prohibited_till = 1200
   },

	working_positions = {
		barbarians_miner = 1
	},

   inputs = {
		ration = 6
	},
   outputs = {
		"gold_ore"
   },

	programs = {
		work = {
			-- TRANSLATORS: Completed/Skipped/Did not start mining because ...
			descname = _"mining",
			actions = {
				"sleep=45000",
				"return=skipped unless economy needs gold_ore",
				"consume=ration",
				"animate=working 20000",
				"mine=gold 2 33 5 17",
				"produce=gold_ore"
			}
		},
	},
	out_of_resource_notification = {
		title = _"Main Gold Vein Exhausted",
		message =
			_"This gold mine’s main vein is exhausted. Expect strongly diminished returns on investment." .. " " ..
			-- TRANSLATORS: "it" is a mine.
			_"You should consider enhancing, dismantling or destroying it.",
		delay_attempts = 0
	},
}
