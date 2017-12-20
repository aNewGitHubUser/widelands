dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   msgctxt = "frisians_building",
   name = "frisians_aqua_farm",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("frisians_building", "Aqua Farm"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "small",

   buildcost = {
      brick = 2,
      log = 2,
      thatch_reed = 1
   },
   return_on_dismantle = {
      brick = 1,
      log = 1
   },

   animations = {
      idle = {
         pictures = path.list_files (dirname .. "idle_??.png"),
         hotspot = { 62, 94 },
         fps = 10,
      },
      unoccupied = {
         pictures = path.list_files (dirname .. "unoccupied_?.png"),
         hotspot = { 62, 94 },
      },
   },

   aihints = {
      prohibited_till = 900,
      requires_helper = true
   },

   working_positions = {
      frisians_fisher = 1
   },

   inputs = {
      { name = "fruit", amount = 8 },
      { name = "water", amount = 8 },
   },
   outputs = {
      "fish"
   },

   programs = {
      work = {
         -- TRANSLATORS: Completed/Skipped/Did not start working because ...
         descname = _"working",
         actions = {
            "call=plant",
            "call=fish",
            "return=skipped",
         },
      },
      plant = {
         -- TRANSLATORS: Completed/Skipped/Did not start breeding fish because ...
         descname = _"breeding fish",
         actions = {
            "return=skipped unless economy needs fish",
            "sleep=23000",
            "consume=fruit water:3",
            "worker=breed_in_pond"
         },
      },
      fish = {
         -- TRANSLATORS: Completed/Skipped/Did not start fishing because ...
         descname = _"fishing",
         actions = {
            "return=skipped unless economy needs fish",
            "sleep=9000",
            "worker=fish_in_pond"
         },
      },
   },
}
