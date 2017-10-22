dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   msgctxt = "frisians_building",
   name = "frisians_collectors_house",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("frisians_building", "Fruit Collector´s House"),
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
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 50, 55 },
      }
   },

   aihints = {
      prohibited_till = 500,
      requires_helper = true
   },

   working_positions = {
      frisians_fruit_collector = 1
   },

   outputs = {
      "fruit"
   },

   programs = {
      work = {
         -- TRANSLATORS: Completed/Skipped/Did not start gathering berries because ...
         descname = _"gathering berries",
         actions = {
            "sleep=21000",
            "worker=harvest",
         }
      },
   },
}
