dirname = path.dirname (__file__)

animations = {
   idle = {
      pictures = path.list_files (dirname .. "idle_??.png"),
      hotspot = { 101, 141 },
      fps = 10
   }
}
add_walking_animations (animations, "sail", dirname, "sail", {114, 139}, 10)


tribes:new_ship_type {
   name = "frisians_ship",
   -- TRANSLATORS: This is the ship's name used in lists of units
   descname = _"Ship",
   capacity = 30,
   vision_range = 4,
   animations = animations,
}
