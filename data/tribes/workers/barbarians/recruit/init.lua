dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = path.list_files(dirname .. "idle_??.png"),
      hotspot = { 16, 31 },
      fps = 5
   }
}
add_walking_animations(animations, "walk", dirname, "walk", {16, 31}, 10)


tribes:new_worker_type {
   msgctxt = "barbarians_worker",
   name = "barbarians_recruit",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("barbarians_worker", "Recruit"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {}, -- This will give the worker the property "buildable"

   animations = animations,
}