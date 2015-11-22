dirname = path.dirname(__file__)

animations = {
   idle = {
      template = "idle_??",
      directory = dirname,
      hotspot = { 7, 22 },
   }
}
add_worker_animations(animations, "walk", dirname, "walk", {23, 23}, 10)
add_worker_animations(animations, "walkload", dirname, "walkload", {11, 24}, 10)


tribes:new_worker_type {
   msgctxt = "barbarians_worker",
   name = "barbarians_brewer",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("barbarians_worker", "Brewer"),
   directory = dirname,
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      barbarians_carrier = 1
   },

   experience = 19,
   becomes = "barbarians_brewer_master",

   animations = animations,
}