dirname = path.dirname (__file__)

animations = {
   idle = {
      pictures = path.list_files (dirname .. "idle_??.png"),
      hotspot = { 21, 25 },
   },
   hacking = {
      pictures = path.list_files (dirname .. "hacking_??.png"),
      hotspot = { 26, 31 },
      fps = 10
   }
}
add_walking_animations (animations, "walk", dirname, "walk", {21, 25}, 15)
add_walking_animations (animations, "walkload", dirname, "walkload", {23, 27}, 15)

tribes:new_worker_type {
   msgctxt = "frisians_worker",
   name = "frisians_stonemason",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext ("frisians_worker", "Stonemason"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      frisians_carrier = 1,
      pick = 1
   },

   programs = {
      cut_granite = {
         "findobject attrib:rocks radius:6",
         "walk object",
         "animation hacking 10000",
         "object shrink",
         "createware granite",
         "return"
      }
   },

   animations = animations,
}
