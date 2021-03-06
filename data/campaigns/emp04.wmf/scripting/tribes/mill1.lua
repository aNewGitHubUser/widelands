dirname = "tribes/buildings/productionsites/empire/mill/"

tribes:new_productionsite_type {
   msgctxt = "empire_building",
   name = "empire_mill1",
   descname = "Mill",
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "medium",

   return_on_dismantle = {
      log = 1,
      granite = 2,
      marble = 1
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 41, 87 },
      },
      working = {
         pictures = path.list_files(dirname .. "working_??.png"),
         hotspot = { 41, 87 },
         fps = 25
      },
   },

   aihints = {

   },

   working_positions = {
      empire_miller = 1
   },

   inputs = {
      { name = "wheat", amount = 6 }
   },
   outputs = {
      "flour"
   },

   programs = {
      work = {
         descname = "grinding wheat",
         actions = {
            "sleep=5000",
            "return=skipped unless economy needs flour",
            "consume=wheat:2",
            "playsound=sound/mill mill_turning 240",
            "animate=working 10000",
            "produce=flour"
         }
      },
   },
}
