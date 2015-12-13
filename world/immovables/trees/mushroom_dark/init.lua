dirname = path.dirname(__file__)

terrain_affinity = {
   preferred_temperature = 100,
   preferred_humidity = 0.2,
   preferred_fertility = 0.8,
   pickiness = 0.8,
}

world:new_immovable_type{
   name = "mushroom_dark_wasteland_sapling",
   -- TRANSLATORS: This is a fictitious tree. Be creative if you want.
   descname = _ "Dark Mushroom Tree (Sapling)",
   editor_category = "trees_wasteland",
   size = "small",
   attributes = { "tree_sapling" },
   terrain_affinity = terrain_affinity,
   programs = {
      program = {
         "animate=idle 72500",
         "remove=80",
         "grow=mushroom_dark_wasteland_pole",
      },
   },
   animations = {
      idle = {
         template = "idle_?",
         directory = dirname .. "sapling/",
         hotspot = { 5, 12 },
         fps = 8,
      },
   },
}

world:new_immovable_type{
   name = "mushroom_dark_wasteland_pole",
   -- TRANSLATORS: This is a fictitious tree. Be creative if you want.
   descname = _ "Dark Mushroom Tree (Pole)",
   editor_category = "trees_wasteland",
   size = "small",
   attributes = {},
   terrain_affinity = terrain_affinity,
   programs = {
      program = {
         "animate=idle 72500",
         "remove=70",
         "grow=mushroom_dark_wasteland_mature",
      },
   },
   animations = {
      idle = {
         template = "idle_?",
         directory = dirname .. "pole/",
         hotspot = { 12, 28 },
         fps = 8,
      },
   },
}

world:new_immovable_type{
   name = "mushroom_dark_wasteland_mature",
   -- TRANSLATORS: This is a fictitious tree. Be creative if you want.
   descname = _ "Dark Mushroom Tree (Mature)",
   editor_category = "trees_wasteland",
   size = "small",
   attributes = {},
   terrain_affinity = terrain_affinity,
   programs = {
      program = {
         "animate=idle 75000",
         "remove=40",
         "grow=mushroom_dark_wasteland_old",
      },
   },
   animations = {
      idle = {
         template = "idle_?",
         directory = dirname .. "mature/",
         hotspot = { 18, 48 },
         fps = 8,
      },
   },
}

world:new_immovable_type{
   name = "mushroom_dark_wasteland_old",
   -- TRANSLATORS: This is a fictitious tree. Be creative if you want.
   descname = _ "Dark Mushroom Tree (Old)",
   -- TRANSLATORS: This is a fictitious tree. Be creative if you want.
   basename = _ "Dark Mushroom Tree",
   editor_category = "trees_wasteland",
   size = "small",
   attributes = { "tree" },
   terrain_affinity = terrain_affinity,
   programs = {
      program = {
         "animate=idle 1050000",
         "transform=deadtree2 25",
         "seed=mushroom_dark_wasteland_sapling",
      },
      fall = {
         "remove=",
      },
   },
   animations = {
      idle = {
         template = "idle_?",
         directory = dirname .. "old/",
         hotspot = { 24, 60 },
         fps = 15,
      },
   },
}
