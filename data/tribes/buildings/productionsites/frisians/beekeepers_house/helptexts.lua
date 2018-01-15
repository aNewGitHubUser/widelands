function building_helptext_lore ()
   -- TRANSLATORS: Lore helptext for a building
   return pgettext ("frisians_building", "Some say the bee stings, but I say, 'tis the bee's wax;")
end

function building_helptext_lore_author ()
   -- TRANSLATORS: Lore author helptext for a building
   return pgettext ("frisians_building", "Cade in Henry VI/2")
end

function building_helptext_purpose()
   -- TRANSLATORS: Purpose helptext for a building
   return pgettext("frisians_building", "Keeps bees and lets them swarm over flowering fields to produce honey.")
end

function building_helptext_note()
   -- TRANSLATORS: Note helptext for a building
   return "Needs medium-sized fields (barley, wheat, corn or blackroot) or bushes (berry bushes, thatch reed or grapevines) nearby."
end

function building_helptext_performance()
   -- TRANSLATORS: Performance helptext for a building
   return pgettext("frisians_building", "The beekeeper pauses %s before going to work again."):bformat(ngettext("%d second", "%d seconds", 45):bformat(45))
end
