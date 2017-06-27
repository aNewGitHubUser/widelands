dirname = path.dirname(__file__)

tribes:new_trainingsite_type {
   msgctxt = "frisians_building",
   name = "frisians_training_arena",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("frisians_building", "Large Training Arena"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "big",

   buildcost = {
      brick = 8,
      granite = 6,
      log = 8,
      thatch_reed = 6
   },
   return_on_dismantle = {
      brick = 4,
      granite = 3,
      log = 4,
      thatch_reed = 3
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 112, 74 }
      }
   },

   aihints = {
      prohibited_till = 1500,
      trainingsite_type = "advanced",
      very_weak_ai_limit = 0,
      weak_ai_limit = 1
   },

   working_positions = {
      frisians_trainer = 1
   },

   inputs = {
      { name = "smoked_fish", amount = 6 },
      { name = "smoked_meat", amount = 6 },
      { name = "mead", amount = 6 },
      { name = "sweetbread", amount = 6 },
      { name = "sword_long", amount = 4 },
      { name = "sword_curved", amount = 4 },
      { name = "sword_double", amount = 4 },
      { name = "helmet_golden", amount = 2 },
      { name = "fur_clothes_golden", amount = 2 },
   },
   outputs = {
      "frisians_soldier",
      "iron_ore",
      "gold_ore",
   },

   ["soldier attack"] = {
      min_level = 3,
      max_level = 5,
      food = {
         {"smoked_fish", "smoked_meat"},
         {"mead"},
         {"sweetbread"}
      },
      weapons = {
         "sword_long",
         "sword_curved",
         "sword_double",
      }
   },
   ["soldier health"] = {
      min_level = 1,
      max_level = 1,
      food = {
         {"smoked_fish", "smoked_meat"},
         {"mead"},
         {"sweetbread"}
      },
      weapons = {
         "helmet_golden",
      }
   },
   ["soldier defense"] = {
      min_level = 1,
      max_level = 1,
      food = {
         {"smoked_fish", "smoked_meat"},
         {"mead"},
         {"sweetbread"}
      },
      weapons = {
         "fur_clothes_golden",
      }
   },

   programs = {
      sleep = {
         -- TRANSLATORS: Completed/Skipped/Did not start sleeping because ...
         descname = _"sleeping",
         actions = {
            "sleep=5000",
            "check_soldier=soldier attack 9", -- dummy check to get sleep rated as skipped - else it will change statistics
         }
      },
      upgrade_soldier_attack_3 = {
         -- TRANSLATORS: Completed/Skipped/Did not start upgrading ... because ...
         descname = pgettext("frisians_building", "upgrading warrior attack from level 3 to level 4"),
         actions = {
            "check_soldier=soldier attack 3", -- Fails when aren't any soldier of level 0 attack
            "sleep=30000",
            "check_soldier=soldier attack 3", -- Because the soldier can be expelled by the player
            "consume=sword_long:2 smoked_fish,smoked_meat mead sweetbread",
            "train=soldier attack 3 4",
            "produce=iron_ore:2 gold_ore:2"
         }
      },
      upgrade_soldier_attack_4 = {
         -- TRANSLATORS: Completed/Skipped/Did not start upgrading ... because ...
         descname = pgettext("frisians_building", "upgrading warrior attack from level 4 to level 5"),
         actions = {
            "check_soldier=soldier attack 4", -- Fails when aren't any soldier of level 0 attack
            "sleep=30000",
            "check_soldier=soldier attack 4", -- Because the soldier can be expelled by the player
            "consume=sword_curved:2 smoked_fish,smoked_meat mead sweetbread",
            "train=soldier attack 4 5",
            "produce=iron_ore:4"
         }
      },
      upgrade_soldier_attack_5 = {
         -- TRANSLATORS: Completed/Skipped/Did not start upgrading ... because ...
         descname = pgettext("frisians_building", "upgrading warrior attack from level 5 to level 6"),
         actions = {
            "check_soldier=soldier attack 5", -- Fails when aren't any soldier of level 0 attack
            "sleep=30000",
            "check_soldier=soldier attack 5", -- Because the soldier can be expelled by the player
            "consume=sword_double:2 smoked_fish,smoked_meat mead sweetbread",
            "train=soldier attack 5 6",
            "produce=iron_ore:2 gold_ore:2"
         }
      },
      upgrade_soldier_defense_1 = {
         -- TRANSLATORS: Completed/Skipped/Did not start upgrading ... because ...
         descname = pgettext("frisians_building", "upgrading warrior defense from level 1 to level 2"),
         actions = {
            "check_soldier=soldier defense 1", -- Fails when aren't any soldier of level 0 attack
            "sleep=30000",
            "check_soldier=soldier defense 1", -- Because the soldier can be expelled by the player
            "consume=fur_clothes_golden smoked_fish,smoked_meat mead sweetbread",
            "train=soldier defense 1 2",
            "produce=iron_ore"
         }
      },
      upgrade_soldier_health_1 = {
         -- TRANSLATORS: Completed/Skipped/Did not start upgrading ... because ...
         descname = pgettext("frisians_building", "upgrading warrior health from level 1 to level 2"),
         actions = {
            "check_soldier=soldier health 1", -- Fails when aren't any soldier of level 0 attack
            "sleep=30000",
            "check_soldier=soldier health 1", -- Because the soldier can be expelled by the player
            "consume=helmet_golden smoked_fish,smoked_meat mead sweetbread",
            "train=soldier health 1 2",
            "produce=iron_ore:2"
         }
      },
   },

   soldier_capacity = 6,
   trainer_patience = 3
}
