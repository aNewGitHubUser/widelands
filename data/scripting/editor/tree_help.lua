-- RST
-- tree_help.lua
-- -------------
--
-- This script returns a formatted entry for the tree help in the editor.
-- Pass the internal tree name to the coroutine to select the tree type.

include "scripting/richtext.lua"
include "scripting/help.lua"

return {
   func = function(tree_name)
      set_textdomain("widelands_editor")
      local world = wl.World();
      local tree = wl.Editor():get_immovable_description(tree_name)
      local result = li_image(tree.representative_image, "")

      -- TRANSLATORS: A header in the editor help. Terrains preferred by a type of tree.
      result = result .. vspace(3) .. h2(vspace(12) .. _"Preferred terrains") .. vspace(3)
      result = result .. terrain_affinity_help(tree)

      return {
         title = tree.species,
         text = result
      }
   end
}
