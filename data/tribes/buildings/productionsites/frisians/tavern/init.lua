dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   msgctxt = "frisians_building",
   name = "frisians_tavern",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("frisians_building", "Tavern"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "medium",
   enhancement = "frisians_drinking_hall",

   buildcost = {
      brick = 4,
      log = 1,
      thatch_reed = 2
   },
   return_on_dismantle = {
      brick = 2,
      thatch_reed = 1
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 57, 88 },
      },
      build = {
         pictures = path.list_files(dirname .. "build_??.png"),
         hotspot = { 57, 88 },
      },
      working = {
         pictures = path.list_files(dirname .. "working_??.png"),
         hotspot = { 57, 88 },
      },
   },

   aihints = {
      prohibited_till = 400,
      very_weak_ai_limit = 1,
      weak_ai_limit = 2,
      basic_amount = 1
   },

   working_positions = {
      frisians_landlady = 1
   },

   inputs = {
      --{ name = "beer", amount = 4 },
      { name = "smoked_fish", amount = 4 },
      { name = "smoked_meat", amount = 4 },
      { name = "fruit", amount = 4 },
      { name = "bread_frisians", amount = 4 },
   },
   outputs = {
      "ration"
   },

   programs = {
      work = {
         -- TRANSLATORS: Completed/Skipped/Did not start working because ...
         descname = _"working",
         actions = {
            "call=produce_fast",
            "call=produce_cheap",
            "return=skipped"
         },
      },
      produce_cheap = {
         -- TRANSLATORS: Completed/Skipped/Did not start preparing a ration because ...
         descname = _"preparing a ration",
         actions = {
            "return=skipped unless economy needs ration",
            "sleep=12000",
            "consume=smoked_fish,smoked_meat,fruit,bread_frisians",
            "animate=working 14000",
            "produce=ration"
         },
      },
      produce_fast = {
         -- TRANSLATORS: Completed/Skipped/Did not start preparing rations because ...
         descname = _"preparing rations",
         actions = {
            "return=skipped unless economy needs ration",
            "sleep=6000",
            "consume=smoked_fish,smoked_meat fruit,bread_frisians",
            "animate=working 18000",
            "produce=ration:2"
         },
      },
   },
}
