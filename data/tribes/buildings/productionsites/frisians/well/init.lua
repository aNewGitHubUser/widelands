dirname = path.dirname (__file__)

tribes:new_productionsite_type {
   msgctxt = "frisians_building",
   name = "frisians_well",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext ("frisians_building", "Well"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "small",

   buildcost = {
      log = 2,
      granite = 1,
      brick = 1
   },
   return_on_dismantle = {
      log = 1,
      brick = 1
   },

   animations = {
      idle = {
         pictures = path.list_files (dirname .. "idle_?.png"),
         hotspot = {24, 41},
      },
      working = {
         pictures = path.list_files (dirname .. "working_??.png"),
         hotspot = {25, 41},
         fps = 10,
      },
   },

   aihints = {
      mines_water = true,
      prohibited_till = 300,
      basic_amount = 1
   },

   working_positions = {
      frisians_carrier = 1
   },

   outputs = {
      "water"
   },

   programs = {
      work = {
         -- TRANSLATORS: Completed/Skipped/Did not start mining water because ...
         descname = _"mining water",
         actions = {
            "sleep=20000",
            "animate=working 20000",
            "mine=water 1 100 65 2",
            "produce=water",
         }
      },
   },

   out_of_resource_notification = {
      -- Translators: Short for "Out of ..." for a resource
      title = _"No Water",
      heading = _"Out of Water",
      message = pgettext ("frisians_building", "The carrier working at this well can’t find any water in his well."),
      productivity_threshold = 33
   },
}
