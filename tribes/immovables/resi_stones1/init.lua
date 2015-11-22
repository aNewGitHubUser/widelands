dirname = path.dirname(__file__)

tribes:new_immovable_type {
   msgctxt = "immovable",
   name = "resi_stones1",
   -- TRANSLATORS: This is a resource name used in lists of resources
   descname = pgettext("immovable", "Some Stones"),
   attributes = { "resi" },
   programs = {
      program = {
         "animate=idle 600000",
         "remove="
      }
   },

   animations = {
      idle = {
         template = "idle_??",
         directory = dirname,
         hotspot = { 7, 10 },
      },
   }
}