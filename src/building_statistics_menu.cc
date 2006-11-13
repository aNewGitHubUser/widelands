/*
 * Copyright (C) 2002-2004, 2006 by the Widelands Development Team
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include <vector>
#include "building.h"
#include "building_statistics_menu.h"
#include "error.h"
#include "graphic.h"
#include "i18n.h"
#include "interactive_player.h"
#include "player.h"
#include "productionsite.h"
#include "rendertarget.h"
#include "tribe.h"
#include "ui_button.h"
#include "ui_progressbar.h"
#include "ui_table.h"
#include "ui_textarea.h"
#include "tribe.h"
#include "wui_plot_area.h"

#define BUILDING_LIST_HEIGHT 220
#define BUILDING_LIST_WIDTH  320

#define LOW_PROD 25

#define UPDATE_TIME 1000  // 1 second, real time

/*
===============
Building_Statistics_Menu::Building_Statistics_Menu

Create all the buttons etc...
===============
*/
Building_Statistics_Menu::Building_Statistics_Menu
(Interactive_Player & parent, UI::UniqueWindow::Registry & registry)
:
UI::UniqueWindow(&parent, &registry, 400, 400, _("Building Statistics")),
m_parent(&parent)
{

   // First, we must decide about the size
   int spacing=5;
   int offsx=spacing;
   int offsy=30;
   int posx=offsx;
   int posy=offsy;

   // Caption
   UI::Textarea* tt=new UI::Textarea(this, 0, 0, _("Building Statistics"), Align_Left);
   tt->set_pos((get_inner_w()-tt->get_w())/2, 5);

   // Building list
   m_table=new UI::Table(this, (get_inner_w()-BUILDING_LIST_WIDTH)/2, offsy, BUILDING_LIST_WIDTH, BUILDING_LIST_HEIGHT, Align_Left, UI::Table::UP);
   m_table->add_column(_("Name").c_str(), UI::Table::STRING, 160);
   m_table->add_column(_("Prod").c_str(), UI::Table::STRING, 40);
   m_table->add_column(_("Owned").c_str(), UI::Table::STRING, 40);
   m_table->add_column(_("Build").c_str(), UI::Table::STRING, 40);
   m_table->selected.set(this, &Building_Statistics_Menu::table_changed);

   posy += BUILDING_LIST_HEIGHT + 2*spacing;
   m_end_of_table_y = posy;

   // let place for Picture
   posx = get_inner_w() / 4 + spacing;

   // Toggle when to run button
   UI::Textarea* ta = new UI::Textarea(this, posx, posy, get_inner_w()/4, 24, _("Total Productivity: "), Align_CenterLeft );
   m_progbar = new UI::Progress_Bar(this, posx + ta->get_w() + spacing, posy, get_inner_w() - ( posx + ta->get_w() + spacing) - spacing, 24, UI::Progress_Bar::Horizontal);
   m_progbar->set_total(100);
   posy += 25;

   // owned
   new UI::Textarea(this, posx, posy, get_inner_w()/4, 24, _("Owned: "), Align_CenterLeft);
   m_owned = new UI::Textarea(this, posx+ta->get_w(), posy, 100, 24, "", Align_CenterLeft);

	m_btn[0] = new UI::IDButton<Building_Statistics_Menu, Jump_Targets>
		(this,
		 get_inner_w() - 58, posy, 24, 24,
		 4,
		 g_gr->get_picture(PicMod_UI, "pics/scrollbar_left.png"),
		 &Building_Statistics_Menu::clicked_jump, this, Prev_Owned);

	m_btn[1] = new UI::IDButton<Building_Statistics_Menu, Jump_Targets>
		(this,
		 get_inner_w() - 29, posy, 24, 24,
		 4,
		 g_gr->get_picture(PicMod_UI, "pics/scrollbar_right.png"),
		 &Building_Statistics_Menu::clicked_jump, this, Next_Owned);

   posy += 25;

   // build
   new UI::Textarea(this, posx, posy, get_inner_w()/4, 24, _("In Build: "), Align_CenterLeft);
   m_build = new UI::Textarea(this, posx+ta->get_w(), posy, 100, 24, "", Align_CenterLeft);

	m_btn[2] = new UI::IDButton<Building_Statistics_Menu, Jump_Targets>
		(this,
		 get_inner_w() - 58, posy, 24, 24,
		 4,
		 g_gr->get_picture(PicMod_UI, "pics/scrollbar_left.png"),
		 &Building_Statistics_Menu::clicked_jump, this, Prev_Construction);

	m_btn[3] = new UI::IDButton<Building_Statistics_Menu, Jump_Targets>
		(this,
		 get_inner_w() - 29, posy, 24, 24,
		 4,
		 g_gr->get_picture(PicMod_UI, "pics/scrollbar_right.png"),
		 &Building_Statistics_Menu::clicked_jump, this, Next_Construction);

   posy += 25;

   // Jump to unproductive
   new UI::Textarea(this, posx, posy, get_inner_w()/4, 24, _("Jump to unproductive: "), Align_CenterLeft);

	m_btn[4] = new UI::IDButton<Building_Statistics_Menu, Jump_Targets>
		(this,
		 get_inner_w() - 58, posy, 24, 24,
		 4,
		 g_gr->get_picture(PicMod_UI, "pics/scrollbar_left.png"),
		 &Building_Statistics_Menu::clicked_jump, this, Prev_Unproductive);

	m_btn[5] = new UI::IDButton<Building_Statistics_Menu, Jump_Targets>
		(this,
		 get_inner_w() - 29, posy, 24, 24,
		 4,
		 g_gr->get_picture(PicMod_UI, "pics/scrollbar_right.png"),
		 &Building_Statistics_Menu::clicked_jump, this, Next_Unproductive);

   posy += 25;

   // TODO: help button
	new UI::Button<Building_Statistics_Menu>
		(this,
		 spacing, get_inner_w() - 37, 32, 32,
		 0,
		 g_gr->get_picture(PicMod_Game, "pics/menu_help.png"),
		 &Building_Statistics_Menu::clicked_help, this);

   m_lastupdate = m_parent->get_game()->get_gametime();
   m_anim = 0;
   update();

   m_last_building_index = 0;
}

/*
===============
Building_Statistics_Menu::~Building_Statistics_Menu

Unregister from the registry pointer
===============
*/
Building_Statistics_Menu::~Building_Statistics_Menu()
{
}

/*
 * Think
 *
 * Update this statistic
 */
void Building_Statistics_Menu::think( void ) {
   int gs = m_parent->get_game()->get_speed();

   if(gs==0) gs = 1;

   if((m_parent->get_game()->get_gametime() - m_lastupdate)/gs > UPDATE_TIME ) {
      update();
      m_lastupdate = m_parent->get_game()->get_gametime();
   }
}

/*
 * draw()
 *
 * Draw this window
 */
void Building_Statistics_Menu::draw(RenderTarget* dst) {
   if(m_anim) {
      dst->drawanim(5 + get_inner_w()/8, m_end_of_table_y + (get_inner_h() - m_end_of_table_y) / 2, m_anim, 0, 0);
   }

	// Draw all the panels etc. above the background
	UI::Window::draw(dst);
}

/*
 * validate if this pointer is ok
 */
int Building_Statistics_Menu::validate_pointer(int* id, int size) {
   if(*id < 0)
      *id = size-1;
   if(*id >= size)
      *id = 0;

   return *id;
}

/*
===========
called when the ok button has been clicked
===========
*/
void Building_Statistics_Menu::clicked_help() {
      log("TODO: help not implemented\n");
}


void Building_Statistics_Menu::clicked_jump(Jump_Targets id) {
   const std::vector< Interactive_Player::Building_Stats >& vec = m_parent->get_building_statistics(m_selected);

   bool found = true; // We think, we always find a proper building

	switch (id) {
	case Prev_Owned:
         /* jump prev building */
         m_last_building_index--;
         break;

	case Next_Owned:
         /* Jump next building */
         m_last_building_index++;
         break;

	case Prev_Construction:
         /* Jump to prev constructionsite */
         {
            int curindex = m_last_building_index;
            while( validate_pointer(&(--m_last_building_index), vec.size()) != curindex )
               if( vec[m_last_building_index].is_constructionsite ) break;
         }
         break;

	case Next_Construction:
         /* Jump to next constructionsite */
         {
            int curindex = m_last_building_index;
            while( validate_pointer(&(++m_last_building_index), vec.size()) != curindex )
               if( vec[m_last_building_index].is_constructionsite ) break;
         }
         break;

	case Prev_Unproductive:
         {
            int curindex = m_last_building_index;
            found = false;
            while( validate_pointer(&(--m_last_building_index), vec.size()) != curindex )
               if( !vec[m_last_building_index].is_constructionsite ) {
                  Building* b = ((Building*)m_parent->get_game()->get_map()->get_field(vec[m_last_building_index].pos)->get_immovable());
                  if( b->get_building_type() == Building::PRODUCTIONSITE) {
                     if(((ProductionSite*)b)->get_statistics_percent() <= LOW_PROD ) {
                        found = true;
                        break;
                     }
                  }
               }
            if(!found) { // Now look at the old
               Building* b = ((Building*)m_parent->get_game()->get_map()->get_field(vec[m_last_building_index].pos)->get_immovable());
               if( b->get_building_type() == Building::PRODUCTIONSITE)
                  if(((ProductionSite*)b)->get_statistics_percent() < LOW_PROD )
                     found = true;
            }
         }
         break;

	case Next_Unproductive:
         {
            int curindex = m_last_building_index;
            found = false;
            while( validate_pointer(&(++m_last_building_index), vec.size()) != curindex )
               if( !vec[m_last_building_index].is_constructionsite ) {
                  Building* b = ((Building*)m_parent->get_game()->get_map()->get_field(vec[m_last_building_index].pos)->get_immovable());
                  if( b->get_building_type() == Building::PRODUCTIONSITE) {
                     if(((ProductionSite*)b)->get_statistics_percent() < LOW_PROD ) {
                        found = true;
                        break;
                     }
                  }
               }
            if(!found) { // Now look at the old
               Building* b = ((Building*)m_parent->get_game()->get_map()->get_field(vec[m_last_building_index].pos)->get_immovable());
               if( b->get_building_type() == Building::PRODUCTIONSITE)
                     if(((ProductionSite*)b)->get_statistics_percent() < LOW_PROD )
                        found = true;
               }
         }
         break;



   }

   validate_pointer(&m_last_building_index, vec.size());

	if (found) m_parent->move_view_to(vec[m_last_building_index].pos);
}

/*
 * The table has been selected
 */
void Building_Statistics_Menu::table_changed( int ) {
   update();
}

/*
 * Update table
 */
void Building_Statistics_Menu::update( void ) {
   m_owned->set_text("");
   m_build->set_text("");
   m_progbar->set_state(0);
   m_selected = -1;

   // List all buildings
	const Tribe_Descr & tribe = *m_parent->get_player()->get_tribe();
	for (long i = 0; i < tribe.get_nrbuildings(); ++i) {
		const char * const name = tribe.get_building_descr(i)->get_name();
      if (strcmp(name, "constructionsite") == 0) continue;
      if (strcmp(name, "headquarters")     == 0) continue;

      const std::vector< Interactive_Player::Building_Stats >& vec = m_parent->get_building_statistics(i);

      // walk all entries, add new ones if needed
      UI::Table_Entry* te = 0;
      for( int l = 0; l< m_table->get_nr_entries(); l++) {
         UI::Table_Entry* entr = m_table->get_entry(l);
         if( (long)entr->get_user_data() == i) {
            te = entr;
            break;
         }
      }

      // If not in list, add new one, as long as this building is
      // enabled
      if(!te) {
         if(! m_parent->get_player()->is_building_allowed(i) ) continue;
			te = new UI::Table_Entry
				(m_table,
				 reinterpret_cast<void * const>(i),
				 tribe.get_building_descr(i)->get_buildicon());
      }

       int nr_owned=0;
          int nr_build=0;
          int total_prod=0;
          bool is_productionsite = 0;
          for(uint l = 0; l < vec.size(); l++) {
             if( vec[l].is_constructionsite ) nr_build++;
             else {
                nr_owned++;
                Building* b = ((Building*)m_parent->get_game()->get_map()->get_field(vec[l].pos)->get_immovable());
                if( b->get_building_type() == Building::PRODUCTIONSITE) {
                   total_prod += ((ProductionSite*)b)->get_statistics_percent();
                   is_productionsite = true;
                }

             }
          }

          // Is this entry selected?
          bool is_selected = (m_table->get_selection_index() != -1 &&  (long)(m_table->get_selection()) == i);

          if(is_selected) {
			m_anim = tribe.get_building_descr(i)->get_ui_anim();
             m_selected = i;
             if(nr_owned)
                for(uint j = 0; j < 2; j++)
                   m_btn[j]->set_enabled(true);
             else
                for(uint j = 0; j < 2; j++)
                   m_btn[j]->set_enabled(false);
             if(nr_build)
                for(uint j = 2; j < 4; j++)
                   m_btn[j]->set_enabled(true);
             else
                for(uint j = 2; j < 4; j++)
                   m_btn[j]->set_enabled(false);
          }

          // Add new Table Entry
          char buffer[100];
		te->set_string(0, tribe.get_building_descr(i)->get_descname());

          // Product
          if(is_productionsite && nr_owned) {
             int percent = (int)((float)total_prod / (float) nr_owned);
             snprintf(buffer, 100, "%i", percent );
             if( is_selected ) {
                m_progbar->set_state ( percent );
                for(uint j = 4; j < 6; j++)
                   m_btn[j]->set_enabled(true);
             }
          } else {
             snprintf(buffer, 100, "-");
             if(is_selected)
                for(uint j = 4; j < 6; j++)
                   m_btn[j]->set_enabled(false);
          }
          te->set_string(1, buffer);

          // Number of this buildings
          snprintf(buffer, 100, "%i", nr_owned);
          te->set_string(2, buffer);
          if(is_selected)
            m_owned->set_text(buffer);

          // Number of currently builds
          snprintf(buffer, 100, "%i", nr_build);
          te->set_string(3, buffer);
          if(is_selected)
            m_build->set_text(buffer);
   }

   // Disable all buttons, if nothing to select
   if(m_selected == -1)
      for(uint j = 0; j < 6; j++)
      m_btn[j]->set_enabled(false);
}
