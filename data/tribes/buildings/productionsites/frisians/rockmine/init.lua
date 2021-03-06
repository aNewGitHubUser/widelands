dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   msgctxt = "frisians_building",
   name = "frisians_rockmine",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("frisians_building", "Rock Mine"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "mine",
   enhancement = "frisians_rockmine_deep",

   buildcost = {
      brick = 1,
      granite = 2,
      log = 2,
      thatch_reed = 1
   },
   return_on_dismantle = {
      brick = 1,
      granite = 1,
      log = 1
   },

   animations = {
      idle = {
         pictures = path.list_files (dirname .. "idle_??.png"),
         hotspot = {38, 94},
         fps = 10,
      },
      working = {
         pictures = path.list_files (dirname .. "working_??.png"),
         hotspot = {38, 94},
         fps = 10,
      },
      empty = {
         pictures = path.list_files (dirname .. "empty_??.png"),
         hotspot = {38, 94},
      },
      unoccupied = {
         pictures = path.list_files (dirname .. "unoccupied_?.png"),
         hotspot = {38, 72},
      },
   },

   aihints = {
      mines = "stones",
      mines_percent = 50,
      prohibited_till = 630
   },

   working_positions = {
      frisians_miner = 1
   },

   inputs = {
      { name = "ration", amount = 8 }
   },
   outputs = {
      "granite"
   },

   programs = {
      work = {
         -- TRANSLATORS: Completed/Skipped/Did not start mining granite because ...
         descname = _"mining granite",
         actions = {
            "sleep=45000",
            "return=skipped unless economy needs granite",
            "consume=ration",
            "animate=working 20000",
            "mine=stones 3 50 5 20",
            "produce=granite",
            "animate=working 20000",
            "mine=stones 3 50 5 20",
            "produce=granite"
         }
      },
   },
   out_of_resource_notification = {
      -- Translators: Short for "Out of ..." for a resource
      title = _"No Granite",
      heading = _"Main Granite Vein Exhausted",
      message =
         pgettext("frisians_building", "This rock mine’s main vein is exhausted. Expect strongly diminished returns on investment. You should consider enhancing, dismantling or destroying it."),
   },
}
