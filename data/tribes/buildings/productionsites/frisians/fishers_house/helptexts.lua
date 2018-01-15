function building_helptext_lore ()
   -- TRANSLATORS: Lore helptext for a building
   return pgettext ("frisians_building", "These fishers tell the infirmities of men,/And from their wat'ry empire recollect/All that may men approve or men detect!")
end

function building_helptext_lore_author ()
   -- TRANSLATORS: Lore author helptext for a building
   return pgettext ("frisians_building", "Pericles Prince of Tyre")
end

function building_helptext_purpose()
   -- TRANSLATORS: Purpose helptext for a building
   return pgettext("frisians_building", "Fishes on the coast near the fisher’s hut.")
end

function building_helptext_note()
   -- TRANSLATORS: Note helptext for a building
   return pgettext("frisians_building", "The fisher’s hut needs water full of fish within the work area.")
end

function building_helptext_performance()
   -- TRANSLATORS: Performance helptext for a building
   return pgettext("frisians_building", "The fisher pauses %s before going to work again."):bformat(ngettext("%d second", "%d seconds", 16):bformat(16))
end
