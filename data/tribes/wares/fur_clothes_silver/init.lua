dirname = path.dirname(__file__)

tribes:new_ware_type {
   msgctxt = "ware",
   name = "fur_clothes_silver",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Studded Fur Clothes"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   default_target_quantity = {
      frisians = 3,
   },
   preciousness = {
      frisians = 2,
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle.png"),
         hotspot = { 6, 9 },
      },
   }
}
