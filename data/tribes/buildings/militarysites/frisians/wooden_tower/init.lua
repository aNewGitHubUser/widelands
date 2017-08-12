dirname = path.dirname(__file__)

tribes:new_militarysite_type {
   msgctxt = "frisians_building",
   name = "frisians_wooden_tower",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("frisians_building", "Wooden Tower"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "small",
   vision_range = 17,

   buildcost = {
      log = 5,
      thatch_reed = 1
   },
   return_on_dismantle = {
      log = 3
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 47, 82 },
      }
   },

   aihints = {
      expansion = true,
      fighting = false,
      mountain_conqueror = true,
      prohibited_till = 1200
   },

   max_soldiers = 1,
   heal_per_second = 60,
   conquers = 6,
   prefer_heroes = false,

   messages = {
      occupied = _"Your soldiers have occupied your wooden tower.",
      aggressor = _"Your wooden tower discovered an aggressor.",
      attack = _"Your wooden tower is under attack.",
      defeated_enemy = _"The enemy defeated your soldiers at the wooden tower.",
      defeated_you = _"Your soldiers defeated the enemy at the wooden tower."
   },
}
