-- This include can be removed when all help texts have been defined.
include "tribes/scripting/help/global_helptexts.lua"

function building_helptext_lore ()
   -- TRANSLATORS#: Lore helptext for a building
   return no_lore_text_yet()
end

function building_helptext_lore_author ()
   -- TRANSLATORS#: Lore author helptext for a building
   return no_lore_author_text_yet()
end

function building_helptext_purpose()
   -- TRANSLATORS: Purpose helptext for a building
   return pgettext("building", "Digs coal out of the ground in mountain terrain.")
end

function building_helptext_note()
   -- TRANSLATORS: Note helptext for a building
   return pgettext("frisians_building", "This mine exploits only %s of the resource. From there on out, it will only have a 5%% chance of finding any coal."):bformat("1/2")
end

function building_helptext_performance()
   -- TRANSLATORS: Performance helptext for a building
   return pgettext("frisians_building", "If the food supply is steady, the coal mine can produce two pieces of coal in %s on average."):bformat(ngettext("%d second", "%d seconds", 85):bformat(85))
end
