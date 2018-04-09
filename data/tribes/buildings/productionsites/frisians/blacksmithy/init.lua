dirname = path.dirname (__file__)

tribes:new_productionsite_type {
   msgctxt = "frisians_building",
   name = "frisians_blacksmithy",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext ("frisians_building", "Blacksmithy"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "medium",

   buildcost = {
      brick = 2,
      granite = 1,
      log = 1,
      thatch_reed = 2
   },
   return_on_dismantle = {
      brick = 1,
      granite = 1,
      thatch_reed = 1
   },

   animations = {
      idle = {
         pictures = path.list_files (dirname .. "idle_??.png"),
         hotspot = {56, 73},
         fps = 10,
      },
      unoccupied = {
         pictures = path.list_files (dirname .. "unoccupied_?.png"),
         hotspot = {56, 61},
      },
      working = {
         pictures = path.list_files (dirname .. "working_??.png"),
         hotspot = {56, 73},
         fps = 10,
      },
   },

   aihints = {
      prohibited_till = 400,
      basic_amount = 1,
      weak_ai_limit = 2,
      very_weak_ai_limit = 1,
   },

   working_positions = {
      frisians_blacksmith = 1
   },

   inputs = {
      { name = "iron", amount = 7 },
      { name = "log", amount = 7 },
      { name = "thatch_reed", amount = 7 }
   },
   outputs = {
      "felling_ax",
      "pick",
      "scythe",
      "shovel",
      "basket",
      "hunting_spear",
      "fishing_net",
      "bread_paddle",
      "kitchen_tools",
      "hammer",
      "fire_tongs",
      "needles",
   },

   programs = {
      work = {
         -- TRANSLATORS: Completed/Skipped/Did not start working because ...
         descname = _"working",
         actions = {
            "call=produce_4",
            "call=produce_2",
            "call=produce_10",
            "call=produce_11",
            "call=produce_1",
            "call=produce_3",
            "call=produce_12",
            "call=produce_5",
            "call=produce_9",
            "call=produce_8",
            "call=produce_7",
            "call=produce_6",
            "return=skipped",
         },
      },
      produce_1 = {
         -- TRANSLATORS: Completed/Skipped/Did not start forging a felling ax because ...
         descname = _"forging a felling ax",
         actions = {
            "return=skipped unless economy needs felling_ax",
            "sleep=32000",
            "consume=log iron",
            "animate=working 35000",
            "produce=felling_ax"
         },
      },
      produce_2 = {
         -- TRANSLATORS: Completed/Skipped/Did not start forging a pick because ...
         descname = _"forging a pick",
         actions = {
            "return=skipped unless economy needs pick",
            "sleep=32000",
            "consume=log iron",
            "animate=working 35000",
            "produce=pick"
         },
      },
      produce_3 = {
         -- TRANSLATORS: Completed/Skipped/Did not start making a scythe because ...
         descname = _"making a scythe",
         actions = {
            "return=skipped unless economy needs scythe",
            "sleep=32000",
            "consume=log iron",
            "animate=working 35000",
            "produce=scythe"
         },
      },
      produce_4 = {
         -- TRANSLATORS: Completed/Skipped/Did not start making a shovel because ...
         descname = _"making a shovel",
         actions = {
            "return=skipped unless economy needs shovel",
            "sleep=32000",
            "consume=log iron",
            "animate=working 35000",
            "produce=shovel"
         },
      },
      produce_5 = {
         -- TRANSLATORS: Completed/Skipped/Did not start making a basket because ...
         descname = _"making a basket",
         actions = {
            "return=skipped unless economy needs basket",
            "sleep=32000",
            "consume=thatch_reed log",
            "animate=working 35000",
            "produce=basket"
         },
      },
      produce_6 = {
         -- TRANSLATORS: Completed/Skipped/Did not start making a hunting spear because ...
         descname = _"making a hunting spear",
         actions = {
            "return=skipped unless economy needs hunting_spear",
            "sleep=32000",
            "consume=log iron",
            "animate=working 35000",
            "produce=hunting_spear"
         },
      },
      produce_7 = {
         -- TRANSLATORS: Completed/Skipped/Did not start making a fishing net because ...
         descname = _"making a fishing net",
         actions = {
            "return=skipped unless economy needs fishing_net",
            "sleep=32000",
            "consume=thatch_reed:2",
            "animate=working 35000",
            "produce=fishing_net"
         },
      },
      produce_8 = {
         -- TRANSLATORS: Completed/Skipped/Did not start making a bread paddle because ...
         descname = _"making a bread paddle",
         actions = {
            "return=skipped unless economy needs bread_paddle",
            "sleep=32000",
            "consume=log iron",
            "animate=working 35000",
            "produce=bread_paddle"
         },
      },
      produce_9 = {
         -- TRANSLATORS: Completed/Skipped/Did not start making kitchen tools because ...
         descname = _"making kitchen tools",
         actions = {
            "return=skipped unless economy needs kitchen_tools",
            "sleep=32000",
            "consume=log iron",
            "animate=working 35000",
            "produce=kitchen_tools"
         },
      },
      produce_10 = {
         -- TRANSLATORS: Completed/Skipped/Did not start making a hammer because ...
         descname = _"making a hammer",
         actions = {
            "return=skipped unless economy needs hammer",
            "sleep=32000",
            "consume=log iron",
            "animate=working 35000",
            "produce=hammer"
         },
      },
      produce_11 = {
         -- TRANSLATORS: Completed/Skipped/Did not start making fire tongs because ...
         descname = _"making fire tongs",
         actions = {
            "return=skipped unless economy needs fire_tongs",
            "sleep=32000",
            "consume=iron",
            "animate=working 35000",
            "produce=fire_tongs"
         },
      },
      produce_12 = {
         -- TRANSLATORS: Completed/Skipped/Did not start forging needles because ...
         descname = _"forging needles",
         actions = {
            "return=skipped unless economy needs needles",
            "sleep=32000",
            "consume=iron",
            "animate=working 35000",
            "produce=needles:2"
         },
      },
   },
}