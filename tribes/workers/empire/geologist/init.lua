dirname = path.dirname(__file__)

animations = {
   idle = {
      template = "idle_??",
      directory = dirname,
      hotspot = { 9, 21 },
      fps = 10
   },
   hacking = {
      template = "hacking_??",
      directory = dirname,
      hotspot = { 11, 18 },
      fps = 10
   }
}
add_worker_animations(animations, "walk", dirname, "walk", {9, 21}, 10)
add_worker_animations(animations, "walkload", dirname, "walk", {9, 21}, 10)


tribes:new_worker_type {
   msgctxt = "empire_worker",
   name = "empire_geologist",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("empire_worker", "Geologist"),
   directory = dirname,
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      empire_carrier = 1,
      hammer = 1
   },

   programs = {
      -- Expedition is the main program
      -- The specialized geologist command walks the geologist around his starting
      -- location, executing the search program from time to time.
      expedition = {
         "geologist 15 5 search"
      },
      -- Search program, executed when we have found a place to hack on
      search = {
         "animation hacking 5000",
         "animation idle 2000",
         "playFX sound/hammering geologist_hammer 192",
         "animation hacking 3000",
         "geologist-find"
      }
   },

   animations = animations,
}