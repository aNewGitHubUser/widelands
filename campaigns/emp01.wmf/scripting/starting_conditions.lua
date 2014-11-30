-- =======================================================================
--                                 Player 1
-- =======================================================================
p1:forbid_buildings("all")

hq = p1:place_building("headquarters_shipwreck",
   wl.Game().map:get_field(6, 62), false, true)
hq:set_wares{
   helmet = 4,
   spear_wooden = 5,
   ax_empire = 6,
   bread_paddle = 2,
   basket = 1,
   bread_empire = 8,
   cloth = 5,
   coal = 12,
   fire_tongs = 2,
   fish = 6,
   fishing_rod = 2,
   flour = 4,
   gold = 4,
   grape = 4,
   hammer = 12,
   hunting_spear = 2,
   iron = 12,
   iron_ore = 5,
   kitchen_tools = 4,
   marble = 25,
   marble_column = 6,
   meal = 4,
   meat = 6,
   pick = 14,
   ration = 12,
   saw = 3,
   scythe = 5,
   shovel = 6,
   granite = 40,
   log = 30,
   water = 12,
   wheat = 4,
   wine = 8,
   planks = 45,
   wool = 2,
}
hq:set_workers{
   armorsmith = 1,
   brewer = 1,
   builder = 10,
   carrier = 40,
   charcoal_burner = 1,
   geologist = 4,
   lumberjack = 3,
   miner = 4,
   stonemason = 2,
   toolsmith = 2,
   weaponsmith = 1,
}
hq:set_soldiers({0,0,0,0}, 45)

