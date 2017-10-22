dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   msgctxt = "frisians_building",
   name = "frisians_reindeer_farm",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("frisians_building", "Reindeer Farm"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "big",

   buildcost = {
      brick = 4,
      granite = 1,
      log = 5,
      thatch_reed = 3
   },
   return_on_dismantle = {
      brick = 2,
      granite = 1,
      log = 2,
      thatch_reed = 1
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 81, 66 },
      },
      working = {
         pictures = path.list_files(dirname .. "idle_??.png"), --TODO no animation yet
         hotspot = { 81, 66 },
      },
   },

   aihints = {
      recruitment = true,
      prohibited_till = 500,
      basic_amount = 1 --TODO this is not a basic building
   },

   working_positions = {
      frisians_reindeer_breeder = 1
   },

   inputs = {
      { name = "barley", amount = 8 },
      { name = "water", amount = 8 }
   },
   outputs = {
      "frisians_reindeer",
      "fur",
      "meat",
   },

   programs = {
      work = {
         -- TRANSLATORS: Completed/Skipped/Did not start working because ...
         descname = pgettext("frisians_building", "working"),
         actions = {
            "call=recruit_deer",
            "call=make_fur",
            "call=recruit_deer",
            "call=make_fur",
            "call=recruit_deer",
            "call=make_fur_meat",
            "return=skipped",
         }
      },
      recruit_deer = {
         -- TRANSLATORS: Completed/Skipped/Did not start rearing cattle because ...
         descname = pgettext("frisians_building", "rearing reindeer"),
         actions = {
            "return=skipped unless economy needs frisians_reindeer",
            "sleep=15000",
            "consume=barley water",
            "animate=working 15000",
            "recruit=frisians_reindeer"
         }
      },
      make_fur = {
         -- TRANSLATORS: Completed/Skipped/Did not start producing fur because ...
         descname = pgettext("frisians_building", "producing fur"),
         actions = {
            "return=skipped unless economy needs fur",
            "sleep=15000",
            "consume=barley water",
            "animate=working 20000",
            "produce=fur"
         }
      },
      make_fur_meat = {
         -- TRANSLATORS: Completed/Skipped/Did not start producing fur because ...
         descname = pgettext("frisians_building", "producing fur"),
         actions = {
            "return=skipped unless economy needs fur",
            "sleep=15000",
            "consume=barley water",
            "animate=working 20000",
            "produce=fur meat"
         }
      },
   },
}
