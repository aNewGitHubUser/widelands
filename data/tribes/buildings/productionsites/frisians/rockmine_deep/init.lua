dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   msgctxt = "frisians_building",
   name = "frisians_rockmine_deep",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("frisians_building", "Deep Rock Mine"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "mine",

   enhancement_cost = {
      brick = 2,
      granite = 1,
      log = 1,
      thatch_reed = 2
   },
   return_on_dismantle_on_enhanced = {
      brick = 1,
      log = 1,
      thatch_reed = 1
   },

   animations = {
      idle = {
         pictures = path.list_files (dirname .. "idle_??.png"),
         hotspot = {27, 65},
         fps = 10,
      },
      working = {
         pictures = path.list_files (dirname .. "working_??.png"),
         hotspot = {27, 66},
         fps = 10,
      },
      empty = {
         pictures = path.list_files (dirname .. "empty_?.png"),
         hotspot = {27, 48},
      },
      unoccupied = {
         pictures = path.list_files (dirname .. "unoccupied_?.png"),
         hotspot = {27, 48},
      },
   },

   aihints = {
      mines = "stones",
   },

   working_positions = {
      frisians_miner = 1,
      frisians_miner_master = 1,
   },

   inputs = {
      { name = "meal", amount = 8 }
   },
   outputs = {
      "granite"
   },

   programs = {
      work = {
         -- TRANSLATORS: Completed/Skipped/Did not start mining granite because ...
         descname = _"mining granite",
         actions = {
            "sleep=40000",
            "return=skipped unless economy needs granite",
            "consume=meal",
            "animate=working 12000",
            "mine=stones 3 100 10 5",
            "produce=granite",
            "animate=working 12000",
            "mine=stones 3 100 10 5",
            "produce=granite",
            "animate=working 12000",
            "mine=stones 3 100 10 5",
            "produce=granite"
         }
      },
   },
   out_of_resource_notification = {
      -- Translators: Short for "Out of ..." for a resource
      title = _"No Granite",
      heading = _"Main Granite Vein Exhausted",
      message =
         pgettext("frisians_building", "This rock mine’s main vein is exhausted. Expect strongly diminished returns on investment. This mine can’t be enhanced any further, so you should consider dismantling or destroying it."),
   },
}