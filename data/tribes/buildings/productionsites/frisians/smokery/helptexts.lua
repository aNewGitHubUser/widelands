-- This include can be removed when all help texts have been defined.
include "tribes/scripting/help/global_helptexts.lua"

function building_helptext_lore()
   -- TRANSLATORS#: Lore helptext for a building
   return no_lore_text_yet()
end

function building_helptext_lore_author()
   -- TRANSLATORS#: Lore author helptext for a building
   return no_lore_author_text_yet()
end

function building_helptext_purpose()
   -- TRANSLATORS: Purpose helptext for a building
   return pgettext("building", "Smokes fish and meat using logs. Only smoked meat and fish are good enough to be eaten by miners and soldiers.")
end

function building_helptext_note()
   -- TRANSLATORS#: Note helptext for a building
   return ""
end

function building_helptext_performance()
   -- TRANSLATORS#: Performance helptext for a building
   return pgettext("frisians_building", "The smokery needs %s to smoke two fish or two meat."):bformat(ngettext("%d second", "%d seconds", 46):bformat(46))
end
