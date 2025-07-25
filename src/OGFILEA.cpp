/*
 * Seven Kingdoms: Ancient Adversaries
 *
 * Copyright 1997,1998 Enlight Software Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

//Filename    : OGFILEA.CPP
//Description : Game File Array / UI.

#ifdef USE_WINDOWS
#include <io.h>
#elif defined USE_POSIX
#include <unistd.h>
#include <time.h>
#endif
#include <OGFILE.h>
#include <KEY.h>
#include <ODIR.h>
#include <OSYS.h>
#include <ODATE.h>
#include <OMOUSE.h>
#include <OMOUSECR.h>
#include <OPOWER.h>
#include <ORACERES.h>
#include <OUNITRES.h>
#include <OIMGRES.h>
#include <OVGA.h>
#include <vga_util.h>
#include <OBOX.h>
#include <OFONT.h>
#include <OINFO.h>
#include <OGAME.h>
#include <OGAMESET.h>
#include <OGFILE.h>
//#include <OSaveGameProvider.h>
#include <OGAMHALL.h>
#include <OBUTT3D.h>
#include <OSLIDCUS.h>
#include <OBLOB.h>
#include <dbglog.h>
#include <ONATIONA.h>
#include "gettext.h"

#include <string.h> //strncpy
#include <posix_string_compat.h> //strnicmp

#ifdef USE_WINDOWS
#include <windows.h>
#endif

extern int input_box(const char*, char*, int, char);

//--------- Define constant ---------//

enum { FILE_MENU_WIDTH = 638,
		 FILE_MENU_HEIGHT = 398 };

enum { FILE_MAIN_MENU_X1 = 80,
		 FILE_MAIN_MENU_Y1 = 175 };

enum { FILE_IN_GAME_MENU_X1 = 80,
		 FILE_IN_GAME_MENU_Y1 = 115 };

// Revert row height:
enum { BROWSE_X1 = 34,
		 BROWSE_Y1 = 31,
		 BROWSE_REC_WIDTH  = 538,
		 BROWSE_REC_HEIGHT = 62,
		 BROWSE_X2 = BROWSE_X1 + BROWSE_REC_WIDTH - 1 };

enum { SCROLL_X1 = 595,
		 SCROLL_Y1 = 47,
		 SCROLL_X2 = 609,
		 SCROLL_Y2 = 324,
		 SCROLL_WIDTH = SCROLL_X2 - SCROLL_X1 + 1,
		 SCROLL_HEIGHT = SCROLL_Y2 - SCROLL_Y1 + 1 };

//----- File name of the game file array --------//

#define MAX_BROWSE_DISP_REC	   5		// MAX. no. of records can be displayed in the saved game browser


//------- Declare static vars & functions ----------//

static char    action_mode;
static short	browse_recno;
static short	browse_top_recno;
static short	menu_x1, menu_y1;

static int     sort_game_file_function( const void *a, const void *b );
static void    disp_scroll_bar_func(SlideVBar *scroll, int);


//------ Begin of function GameFileArray constuctor ------//

GameFileArray::GameFileArray() : DynArray( sizeof(GameFile), 10 )
{
	demo_format = 0;

	#ifdef DEMO
		demo_format = 1;
	#endif

	#ifdef DEMO_DESIGN
		demo_format = 1;
	#endif

	has_read_hall_of_fame = 0;
	last_file_name[0] = '\0';
}
//-------- End of function GameFileArray constuctor ------//


//------ Begin of function GameFileArray::init ------//

void GameFileArray::init(const char *extStr)
{
	if( !has_read_hall_of_fame )		// only read once, GameFileArray::init() is called every time the load/save game menu is brought up.
	{
		strncpy(last_file_name, hall_of_fame.get_last_savegame_file_name(), FilePath::MAX_FILE_PATH);
		last_file_name[FilePath::MAX_FILE_PATH] = '\0';
		has_read_hall_of_fame = 1;
	}

	//-- Load all headers of all saved game files in current directory --//

	load_all_game_header(extStr);
}
//-------- End of function GameFileArray::init ------//


//------ Begin of function GameFileArray::deinit ------//

void GameFileArray::deinit()
{
	//-- Need to transfer last savegame filename to hall of fame, because it contains last loaded/saved game --//
	hall_of_fame.set_last_savegame_file_name(last_file_name);
}
//-------- End of function GameFileArray::deinit ------//


#define LSOPTION_SLOT(n) (1 << (n))
#define LSOPTION_ALL_SLOTS       0x0000ffff
#define LSOPTION_PAGE            0x00010000
#define LSOPTION_SCROLL          0x00020000
#define LSOPTION_ALL             0xffffffff


//-------- Begin of function GameFileArray::menu --------//
//
// <int> actionMode = -2 - save screen to back buffer
//                    -1 - restore screen to back buffer
//                    1 - save game
//                    2 - load game
//
// <int *> recno    = if overwritting save game or load game action
//                    is succcessful, return the recno of GameFile
//
// return : <int> 1 - game loaded/saved
//                0 - user cancel loading/saving
//               -1 - loading/saving error
//
int GameFileArray::menu(int actionMode, int *recno)
{
	if( actionMode == -2 || actionMode == -1)
	{
		// copy or restore screen to back buffer
		int scrnX1, scrnY1, scrnX2, scrnY2;
		if( game.game_mode==GAME_PREGAME )	  // called from the main menu, not in the game
		{
			scrnX1 = FILE_MAIN_MENU_X1;
			scrnY1 = FILE_MAIN_MENU_Y1;
		}
		else
		{
			scrnX1 = FILE_IN_GAME_MENU_X1;
			scrnY1 = FILE_IN_GAME_MENU_Y1;
		}
		scrnX2 = scrnX1 + FILE_MENU_WIDTH-1;
		scrnY2 = scrnY1 + FILE_MENU_HEIGHT-1;

		mouse.hide_area( scrnX1, scrnY1, scrnX2, scrnY2);

		if( actionMode == -2 )
			// save to back buffer
			IMGcopy(vga_back.buf_ptr(), vga_back.buf_pitch(),
				vga_front.buf_ptr(), vga_front.buf_pitch(),
				scrnX1, scrnY1, scrnX2, scrnY2);
		else
			// restore from back buffer
			IMGcopy(vga_front.buf_ptr(), vga_front.buf_pitch(),
				vga_back.buf_ptr(), vga_back.buf_pitch(),
				scrnX1, scrnY1, scrnX2, scrnY2);

		mouse.show_area();

		return 1;
	}

	action_mode = actionMode;

	if( action_mode==2 && size()==0 )
	{
		box.msg( _("You haven't saved any games yet.") );
		return 0;
	}

	// load race icon
	char deinitGameSet=0;
	char deinitUnitRes=0;
	char deinitRaceRes=0;

	if( !game_set.set_opened_flag )
	{
		game_set.open_set(1);
		deinitGameSet = 1;
	}
	if( !unit_res.init_flag )
	{
		unit_res.init();
		deinitUnitRes = 1;
	}
	if( !race_res.init_flag )
	{
		race_res.init();
		deinitRaceRes = 1;
	}

	//-------------------------------------//

	if( game.game_mode==GAME_PREGAME )	  // called from the main menu, not in the game
	{
		menu_x1 = FILE_MAIN_MENU_X1;
		menu_y1 = FILE_MAIN_MENU_Y1;
	}
	else
	{
		menu_x1 = FILE_IN_GAME_MENU_X1;
		menu_y1 = FILE_IN_GAME_MENU_Y1;
	}

	int x=menu_x1, y=menu_y1+17;

	// vga_back.adjust_brightness( x, y, x+menu_x1-1, y+menu_y1-1, -6 );
	vga_util.blt_buf( x, y, x+menu_x1-1, y+menu_y1-1, 0 );

	mouse_cursor.set_icon(CURSOR_NORMAL);

	power.win_opened = 1;

	int minRecno = action_mode == 1 ? 0 : 1;

	//------ set current record no. -------//

	for( int i=1 ; i<=size() ; i++ )
	{
		if( strcmp(last_file_name, (*this)[i]->file_name)==0 )
		{
			browse_recno = i;
			break;
		}
	}

	//---------------------------------//

	browse_top_recno = minRecno;
	// in save game mode, browse_recno = 0 means selecting empty slot
	// in load game mode, browse_recno = 0 means nonthing is selected
	// browse_top_recno = browse_recno ? browse_recno : minRecno;

	//--------------------------------------//
	Button3D scrollUp, scrollDown, saveButton, saveNewButton, delButton, cancelButton;
	int retFlag = 0;
	int refreshFlag = LSOPTION_ALL;
	//int scrollButtonY1 = menu_y1+SCROLL_Y1, scrollButtonY2 = menu_y1+SCROLL_Y2;
	//int dragingScrollBar = 0;
	//int dragScrollYDiff;	// when draging begins, mouse.cur_y - scrollButtonY1

	SlideVBar scrollBar;
	scrollBar.init_scroll(menu_x1+SCROLL_X1, menu_y1+SCROLL_Y1, menu_x1+SCROLL_X2, menu_y1+SCROLL_Y2,
		MAX_BROWSE_DISP_REC, disp_scroll_bar_func);
	scrollBar.set(minRecno, size(), browse_top_recno);

	// try to centre the selected record on the browser
	//browse_top_recno = browse_recno - MAX_BROWSE_DISP_REC /2;
	//if( browse_top_recno > size()-MAX_BROWSE_DISP_REC+1)
	//	browse_top_recno = size()-MAX_BROWSE_DISP_REC+1;
	//if( browse_top_recno < minRecno )
	//	browse_top_recno = minRecno;
	browse_top_recno = scrollBar.set_view_recno(browse_recno - MAX_BROWSE_DISP_REC /2);

	Blob browseArea[MAX_BROWSE_DISP_REC];
	Blob scrollArea;

	while(1)
	{
		//---------- yield --------//

		sys.yield();
		vga.flip();

		mouse.get_event();

		// When called ingame sys.signal_exit_flag is set to 2 by Sys::load_game
		if( sys.signal_exit_flag == 1 )
		{
			retFlag = 0;
			break;
		}

		// --------- display ----------//

		if( refreshFlag )
		{
			if( refreshFlag & LSOPTION_PAGE )
			{
				mouse.hide_area(menu_x1, menu_y1, menu_x1+FILE_MENU_WIDTH, menu_y1+FILE_MENU_HEIGHT);

				image_interface.put_front( menu_x1, menu_y1, actionMode==1 ? (char*)"SAVEGAME" : (char*)"LOADGAME" );

				scrollUp.paint(menu_x1+SCROLL_X1+1,menu_y1+SCROLL_Y1-17, "SV-UP-U", "SV-UP-D");
				scrollDown.paint(menu_x1+SCROLL_X1+1,menu_y1+SCROLL_Y2+1, "SV-DW-U", "SV-DW-D");
				if( action_mode == 1)
				{
					saveButton.paint(menu_x1+34, menu_y1+354, "SAVE", "CANCEL1D");
					saveNewButton.paint(menu_x1+147, menu_y1+354, "SAVE-NEW", "CANCEL1D");
					delButton.paint(menu_x1+260, menu_y1+354, "DELETE", "CANCEL1D");
				}
				else if( action_mode == 2)
				{
					saveButton.paint(menu_x1+34, menu_y1+354, "LOAD", "CANCEL1D");
				}
				cancelButton.paint(menu_x1+473, menu_y1+354, "CANCEL1", "CANCEL1D");

				// capture browseArea, scrollArea
				for( int j = 0; j < MAX_BROWSE_DISP_REC; ++j)
				{
					browseArea[j].resize(2*sizeof(short)+BROWSE_REC_WIDTH*BROWSE_REC_HEIGHT);
					vga_front.read_bitmap(
						menu_x1+BROWSE_X1, menu_y1+BROWSE_Y1+j*BROWSE_REC_HEIGHT,
						menu_x1+BROWSE_X2, menu_y1+BROWSE_Y1+j*BROWSE_REC_HEIGHT+BROWSE_REC_HEIGHT-1,
						browseArea[j].ptr);
				}

				scrollArea.resize(2*sizeof(short)+SCROLL_WIDTH*SCROLL_HEIGHT);
				vga_front.read_bitmap( menu_x1+SCROLL_X1, menu_y1+SCROLL_Y1,
					menu_x1+SCROLL_X2, menu_y1+SCROLL_Y2, scrollArea.ptr);

				mouse.show_area();
			}

			if( scrollBar.max_recno != size() )
			{
				scrollBar.set_max_recno(size());
				if( scrollBar.view_recno > scrollBar.max_view_recno() )
				{
					scrollBar.view_recno = scrollBar.max_view_recno();
				}
				refreshFlag |= LSOPTION_SCROLL;
			}

			if( refreshFlag & LSOPTION_SCROLL )
			{
				vga_front.put_bitmap( menu_x1+SCROLL_X1, menu_y1+SCROLL_Y1,
					scrollArea.ptr);
				scrollBar.paint();
			}

			if( refreshFlag & LSOPTION_ALL_SLOTS )
			{
				for(int slot = 0; slot < MAX_BROWSE_DISP_REC; ++slot)
				{
					int rec = slot + scrollBar.view_recno;
					if( refreshFlag & LSOPTION_SLOT(slot) )
					{
						int browseSlotX1 = menu_x1+BROWSE_X1;
						int browseSlotY1 = menu_y1+BROWSE_Y1+slot*BROWSE_REC_HEIGHT;
						int browseSlotX2 = menu_x1+BROWSE_X2;
						int browseSlotY2 = menu_y1+BROWSE_Y1+(slot+1)*BROWSE_REC_HEIGHT-1;

						mouse.hide_area(browseSlotX1, browseSlotY1,
							browseSlotX2, browseSlotY2);
						vga_front.put_bitmap( browseSlotX1, browseSlotY1,
							browseArea[rec%MAX_BROWSE_DISP_REC].ptr);

						if( rec == 0 )
						{
							err_when( action_mode!=1 );
							font_bible.center_put( browseSlotX1, browseSlotY1,
								browseSlotX2, browseSlotY2, _("Empty Game Slot") );
						}
						else if( rec <= size() )
						{
							disp_entry_info((*this)[rec], browseSlotX1, browseSlotY1);
						}
						if( rec == browse_recno )
						{
							vga_front.adjust_brightness( browseSlotX1, browseSlotY1,
								browseSlotX2, browseSlotY2, -2);
							vga_front.put_bitmap_trans_decompress(browseSlotX1, browseSlotY1,
								image_button.read("LS-DWN"));
						}
						mouse.show_area();
					}
				}
				// disp_browse();
			}

			refreshFlag = 0;
		}

		sys.blt_virtual_buf();

		if( scrollBar.detect() == 1 )
		{
			browse_top_recno = scrollBar.view_recno;
			refreshFlag |= LSOPTION_SCROLL | LSOPTION_ALL_SLOTS;
		}
		else if( scrollUp.detect() )
		{
			// click on scroll up
			int oldValue = scrollBar.view_recno;
			if( oldValue != scrollBar.set_view_recno(oldValue-1) )
				refreshFlag |= LSOPTION_SCROLL | LSOPTION_ALL_SLOTS;
			browse_top_recno = scrollBar.view_recno;
		}
		else if( scrollDown.detect() )
		{
			// click on scroll down
			// click on scroll up
			int oldValue = scrollBar.view_recno;
			if( oldValue != scrollBar.set_view_recno(oldValue+1) )
				refreshFlag |= LSOPTION_SCROLL | LSOPTION_ALL_SLOTS;
			browse_top_recno = scrollBar.view_recno;
		}
		else if( mouse.double_click( menu_x1+BROWSE_X1, menu_y1+BROWSE_Y1,
			menu_x1+BROWSE_X1+BROWSE_REC_WIDTH-1,
			menu_y1+BROWSE_Y1+ BROWSE_REC_HEIGHT*MAX_BROWSE_DISP_REC -1) )
		{
			// double click on game slot
			int oldValue = browse_recno;
			int newValue = scrollBar.view_recno + (mouse.click_y(0) - BROWSE_Y1 - menu_y1) / BROWSE_REC_HEIGHT;
			if( newValue <= size())
			{
				// ######## begin Gilbert 31/10 ########//
				if( newValue == oldValue )
				{
					browse_recno = newValue;
					refreshFlag |= LSOPTION_SLOT(newValue-scrollBar.view_recno);
					if( oldValue-scrollBar.view_recno >= 0 && oldValue-scrollBar.view_recno < MAX_BROWSE_DISP_REC )
						refreshFlag |= LSOPTION_SLOT(oldValue-scrollBar.view_recno);
					if( recno )
						*recno = browse_recno;
					retFlag = process_action(0);
//					if( retFlag < 0 )
//						box.msg("Error");
					break;
				}
				// ######## end Gilbert 31/10 ########//
			}
		}
		else if( mouse.single_click( menu_x1+BROWSE_X1, menu_y1+BROWSE_Y1,
			menu_x1+BROWSE_X1+BROWSE_REC_WIDTH-1,
			menu_y1+BROWSE_Y1+ BROWSE_REC_HEIGHT*MAX_BROWSE_DISP_REC -1) )
		{
			// click on game slot
			int oldValue = browse_recno;
			int newValue = scrollBar.view_recno + (mouse.click_y(0) - BROWSE_Y1 - menu_y1) / BROWSE_REC_HEIGHT;
			if( newValue <= size())
			{
				// ##### begin Gilbert 31/10 #######//
				//if( oldValue == browse_recno )
				//{
				//	browse_recno = newValue;
				//	refreshFlag |= LSOPTION_SLOT(oldValue-scrollBar.view_recno)
				//		| LSOPTION_SLOT(newValue-scrollBar.view_recno);
				//}
				if( newValue != oldValue )
				{
					browse_recno = newValue;
					refreshFlag |= LSOPTION_SLOT(newValue-scrollBar.view_recno);
					if( oldValue-scrollBar.view_recno >= 0 && oldValue-scrollBar.view_recno < MAX_BROWSE_DISP_REC )
						refreshFlag |= LSOPTION_SLOT(oldValue-scrollBar.view_recno);
				}
				// ##### end Gilbert 31/10 #######//
			}
		}
		else if( cancelButton.detect(KEY_ESC) || mouse.any_click(RIGHT_BUTTON) > 0)		// also when ESC key is pressed or right button
		{
			// cancel button or escape key
			refreshFlag = LSOPTION_ALL;
			retFlag = 0;
			break;		// break while(1)
		}
		else if( (action_mode == 1 || (action_mode == 2 && browse_recno))
			&& saveButton.detect() )
		{
			// save / load button
			refreshFlag = LSOPTION_ALL;
			if( recno )
				*recno = browse_recno;
			retFlag = process_action(0);
			// ##### begin Gilbert 15/10 #####//
			if( retFlag != 0 )
			{
//				if( retFlag < 0 )
//					box.msg("Error");
				break;
			}
			// ##### end Gilbert 15/10 #####//
		}
		else if( action_mode == 1 && saveNewButton.detect() )
		{
			// save new button
			refreshFlag = LSOPTION_ALL;
			retFlag = process_action(1);
//			if( retFlag < 0 )
//				box.msg("Error");
			break;
		}
		else if( action_mode == 1 && browse_recno && delButton.detect() )
		{
			// delete save game button
			if( browse_recno != 0 )			// cannot del save game slot
			{
				del_game();
				if( browse_recno > size() )
				{
					browse_recno = size();
				}
				if( browse_top_recno > size()-MAX_BROWSE_DISP_REC+1)
					browse_top_recno = size()-MAX_BROWSE_DISP_REC+1;
				if( browse_top_recno < minRecno )
					browse_top_recno = minRecno;
				scrollBar.set_view_recno(browse_top_recno);
				refreshFlag |= LSOPTION_ALL_SLOTS | LSOPTION_SCROLL;
			}
			else
			{
				box.msg(_("Cannot delete this slot"));
			}
			refreshFlag = LSOPTION_ALL;
		}
	}

	power.win_opened = 0;
	if( retFlag <= 0 )
	{
		// if load game is successful, no need to deinit resource
		if( deinitGameSet )
			game_set.close_set();
		if( deinitUnitRes )
			unit_res.deinit();
		if( deinitRaceRes )
			race_res.deinit();
	}

	mouse.reset_click();
	return retFlag;
}
//---------- End of function GameFileArray::menu --------//


//-------- Begin of function GameFileArray::disp_browse --------//
//
// Display saved game info on the browser.
//
void GameFileArray::disp_browse()
{
	int lastRec = MIN(browse_top_recno+MAX_BROWSE_DISP_REC-1, size());

	int x = menu_x1 + BROWSE_X1;
	int y = menu_y1 + BROWSE_Y1;

	for( int i=browse_top_recno ; i<=lastRec ; i++, y+=BROWSE_REC_HEIGHT )
	{
		if( i==0 )
		{
			err_when( action_mode!=1 );
			font_bible.center_put( x, y, x+BROWSE_REC_WIDTH-1, y+BROWSE_REC_HEIGHT-1, "Empty Game Slot" );
		}
		else
		{
			disp_entry_info((*this)[i], x, y);
		}
		if( i == browse_recno )
		{
			vga_front.adjust_brightness(x,y,x+BROWSE_REC_WIDTH-1,y+BROWSE_REC_HEIGHT-1,-2);
			vga_front.put_bitmap_trans_decompress( x, y, image_button.read("LS-DWN"));
		}
	}
}
//--------- End of function GameFileArray::disp_browse --------//


static time_t filetime_to_posix(const GameFileDate* gameFileDate)
{
	time_t t;

	t = (uint64_t)gameFileDate->dwHighDateTime << 32;
	t |= gameFileDate->dwLowDateTime;

	// 100-nanoseconds = milliseconds * 10000
	// removes the diff between 1970 and 1601
	t -= 11644473600000 * 10000;

	// converts back from 100-nanoseconds to seconds
	t /= 10000000;

	return t;
}


//-------- Begin of function GameFileArray::disp_entry_info --------//
//
void GameFileArray::disp_entry_info(const GameFile* entry, int x, int y)
{
	vga_front.put_bitmap(x+10, y+10,	unit_res[ race_res[entry->race_id]->basic_unit_id ]->king_icon_ptr);

	x+=60;

	//------ display player color -----//

	nation_array.disp_nation_color( x+1, y+13, entry->nation_color );

	//-------- display king name ------//

	String str;

	str  = _("King");
	str += " ";
	str += entry->player_name;

	font_bible.put( x+18, y+8, str );

	//------- display game date --------//

	str  = _("Game Date");
	str += ": ";
	str += date.date_str(entry->game_date);

	font_bible.put( x, y+30, str );

	//---------------------------------//

	str  = _("File Name");
	str += ": ";
	str += entry->file_name;

	// Move file info, file date, and note text up by 20 pixels in disp_entry_info:
	// File name
#if(defined(FRENCH))
	font_small.put( x+320, y+6, str );
#elif(defined(GERMAN))
	font_small.put( x+320, y+6, str );
#else
	font_small.put( x+335, y+6, str );
#endif
// File date
str  = _( "File Date" );
str += ": ";
#ifdef USE_WINDOWS
	SYSTEMTIME sysTime;
	FILETIME localFileTime;
	FileTimeToLocalFileTime( (FILETIME*)&entry->file_date, &localFileTime );
	FileTimeToSystemTime( &localFileTime, &sysTime );

	str += date.date_str(date.julian(sysTime.wYear, sysTime.wMonth, sysTime.wDay), 1);
	str += " ";
	str += date.time_str(sysTime.wHour*100 + sysTime.wMinute);
#elif defined USE_POSIX
	time_t gameFileDate = filetime_to_posix(&entry->file_date);
	struct tm* lt = localtime(&gameFileDate);
	str += date.date_str(date.julian(lt->tm_year+1900, lt->tm_mon+1, lt->tm_mday), 1);
	str += " ";
	str += date.time_str(lt->tm_hour*100 + lt->tm_min);
#endif
#if(defined(FRENCH))
	font_small.put( x+318, y+20, str );
#elif(defined(GERMAN))
	font_small.put( x+320, y+20, str );
#else
	font_small.put( x+335, y+20, str );
#endif
// Note
if (entry->note[0]) {
#if(defined(FRENCH))
	font_small.put( x+318, y+34, entry->note );
#elif(defined(GERMAN))
	font_small.put( x+320, y+34, entry->note );
#else
	font_small.put( x+335, y+34, entry->note );
#endif
}
}
//--------- End of function GameFileArray::disp_entry_info --------//


//------- Begin of function GameFileArray::process_action ------//
//
// [int] saveNew - save on a new game file
//                 (default : 0)
//
// return : <int> 1 - process ok
//                0 - process cancelled
//               -1 - save/load failed
//
int GameFileArray::process_action(int saveNew)
{
	//------------ save game --------------//

	if( action_mode == 1 )
	{
		if( saveNew || browse_recno==0 )   // save on empty slot
		{
			if ( !save_new_game())
				return -1;
		}
		else           // save on existing slot
		{
			if( !box.ask( _("Overwrite the existing saved game?") ) )
				return 0;

			GameFile* gameFile = (*this)[browse_recno];
			char note_buf2[128] = "";
			if (game.game_mode != GAME_PREGAME) {
				if (!input_box("Enter a note for this save (optional):", note_buf2, sizeof(note_buf2)-1, 0)) {
					note_buf2[0] = '\0';
				}
			}
			note_buf2[sizeof(note_buf2)-1] = '\0';
			strncpy(gameFile->note, note_buf2, sizeof(gameFile->note));
			gameFile->note[sizeof(gameFile->note)-1] = '\0';

			if( !gameFile->save_game() )
			{
				box.msg(gameFile->status_str());
				return -1;
			}

			strcpy( last_file_name, gameFile->file_name );
		}

		return 1;
	}

	//----------- load game -------------//

	else
	{
		GameFile* gameFile = (*this)[browse_recno];

		int rc = gameFile->load_game((const char*)sys.dir_config, NULL);
		if( rc > 0 )
		{
			strcpy(last_file_name, (*this)[browse_recno]->file_name);
		}
		else
		{
			box.msg(gameFile->status_str());
		}
		return rc;
	}

	return 0;
}
//--------- End of function GameFileArray::process_action ------//


//-------- Begin of function GameFileArray::save_new_game -----//
//
// Save current game to a new saved game file immediately without
// prompting menu.
//
// Called by GameFileArray::process_action() and error handler.
//
// [char*] fileName - file name of the saved game
//
// return : <int> 1 - saved successfully.
//                0 - not saved.
//
int GameFileArray::save_new_game(const char* fileName)
{
	GameFile gameFile;
	int addFlag=1;
	int gameFileRecno;

	if( fileName )
	{
		if( strlen(fileName) >= FilePath::MAX_FILE_PATH )
		{
			box.msg(_("Cannot save the game because the path is too long"));
			return 0;
		}

		//----- check for overwriting an existing file ----//

		for( gameFileRecno=1 ; gameFileRecno<=this->size() ; gameFileRecno++ )
		{
			GameFile* gameFilePtr = (*this)[gameFileRecno];

			if( strcmp(gameFilePtr->file_name, fileName)==0 )      // if this file name already exist
			{
				addFlag=0;
				break;
			}
		}

		strcpy( gameFile.file_name, fileName );
	}
	else
	{
		gameFile.set_file_name();        // give it a new game_file_name based on current group name
	}

	//----------- save game now ------------//

	char note_buf[128] = "";
	if (game.game_mode != GAME_PREGAME) { // Only prompt if not in pregame (adjust as needed)
		if (!input_box("Enter a note for this save (optional):", note_buf, sizeof(note_buf)-1, 0)) {
			note_buf[0] = '\0'; // User cancelled or empty
		}
	}
	note_buf[sizeof(note_buf)-1] = '\0';
	strncpy(gameFile.note, note_buf, sizeof(gameFile.note));
	gameFile.note[sizeof(gameFile.note)-1] = '\0';

	if( gameFile.save_game(fileName) )
	{
		strcpy( last_file_name, gameFile.file_name );

		FilePath full_path(sys.dir_config);
		full_path += gameFile.file_name;
		Directory saveGameDirectory;
		saveGameDirectory.read(full_path, 0);  // 0-Don't sort file names

		if( saveGameDirectory.size() == 1 )
		{
			if( addFlag )
			{
				linkin(&gameFile);

				quick_sort( sort_game_file_function );
			}
			else
			{
				this->update(&gameFile, gameFileRecno);
			}
		}

		return 1;
	}
	else
	{
		box.msg(gameFile.status_str());
		return 0;
	}
}
//-------- End of function GameFileArray::save_new_game -------//



//------- Begin of function GameFileArray::set_file_name -------//
//
// Set the game file name of the given save game
//
// e.g. ENLI_001.SAV - the first saved game of the group "Enlight Enterprise"
//
void GameFileArray::set_file_name(char* /*out*/ fileName, int size)
{
	enum { NAME_PREFIX_LEN = 4,    // Maximum 4 characters in name prefix, e.g. "ENLI" for "Enlight Enterprise"
		NAME_NUMBER_LEN = 3  };

	String str, str2;
	int    i;
	char   nameChar;
	const char*  baseName;             // the long name which the file name is based on
	char   addStr[] = "0";       // as a small string for adding to the large string

	baseName = (~nation_array)->king_name();

	//--------- add the group name prfix ----------//

	for( i=0 ; i<(int) strlen(baseName) && (int) str.len() < NAME_PREFIX_LEN ; i++ )
	{
		nameChar = misc.upper(baseName[i]);

		if( ( nameChar >= 'A' && nameChar <= 'Z' ) ||
			( nameChar >= '0' && nameChar <= '9' ) )
		{
			addStr[0] = nameChar;

			str += addStr;
		}
	}

	//----- add tailing characters if prefix len < NAME_PREFIX_LEN+1 ---//

	while( str.len() < NAME_PREFIX_LEN+1 )       // +1 is the "_" between the name and the number
		str += "_";

	//---- find the saved game number for this saved game ----//

	int       curNumber, lastNumber=0;

	for( i=1 ; i<=this->size() ; i++ )
	{
		GameFile* findSaveGame = (*this)[i];

		// ##### begin Gilbert 3/10 ########//
		if( strnicmp(findSaveGame->file_name, str, NAME_PREFIX_LEN)==0 )
			// ##### end Gilbert 3/10 ########//
		{
			//------------------------------------------------//
			//
			// if there is a free number in the middle of the list
			// (left by being deleted game), use this number.
			//
			//------------------------------------------------//

			curNumber = atoi( findSaveGame->file_name+NAME_PREFIX_LEN+1 );      // +1 is to pass the "_" between the name and the number

			if( curNumber > lastNumber+1 )   // normally, curNumber should be lastNumber+1
				break;

			lastNumber = curNumber;
		}
	}

	//------- add saved game number after the prefix --------//

	str2 = lastNumber+1;    // use the next number after the last number

	for( i=NAME_NUMBER_LEN-str2.len() ; i>0 ; i-- )   // add "0" before the number if the len of the number < NAME_NUMBER_LEN
		str += "0";

	str += str2;
	str += ".SAV";

	//----- copy the string to fileName ------//

	strncpy( fileName, str, size-1 );
	fileName[size-1] = '\0';
}
//--------- End of function GameFileArray::set_file_name -------//


//------- Begin of function GameFileArray::del_game ------//

void GameFileArray::del_game()
{
	int recNo = browse_recno;

	if( !box.ask( _("This saved game will be deleted. Proceed?") ) )
		return;

	FilePath fullPath(sys.dir_config);
	fullPath += (*this)[recNo]->file_name;
	if( fullPath.error_flag )
		return;
	unlink(fullPath);

	go(recNo);
	linkout();

	go(recNo-1);    // skip back one record
}
//--------- End of function GameFileArray::del_game ------//


//-------- Begin of function GameFileArray::load_all_game_header --------//
//
// Load all headers of all saved game files in current directory.
//
void GameFileArray::load_all_game_header(const char *extStr)
{
	zap();

	FilePath full_path(sys.dir_config);

	full_path += extStr;
	if( full_path.error_flag )
		return;

	Directory gameDir;
	gameDir.read(full_path, 0);  // 0-Don't sort file names

	//-------- read in the headers of all savegames -------//

	for( int i=1 ; i<=gameDir.size() ; i++ )
	{
		const char* const gameFileName = gameDir[i]->name;
		FilePath save_game_path(sys.dir_config);

		save_game_path += gameFileName;
		if( save_game_path.error_flag )
			continue;

		GameFile gameFile;
		if( gameFile.read_game_header(save_game_path) )
		{
			strcpy( gameFile.file_name, gameFileName );  // in case that the name may be different
			linkin(&gameFile);
		}
	}

	quick_sort( sort_game_file_function );
}
//------ End of function GameFileArray::load_all_game_header --------//


//------ Begin of function sort_game_file_function ------//
//
// Sort files by name, whilst moving AUTO.SAV and AUTO2.SAV to the top
//
static int sort_game_file_function( const void *a, const void *b )
{
	int firstAuto = 0, secondAuto = 0;

	firstAuto = strcmpi( ((GameFile*)a)->file_name, "AUTO.SAV" ) == 0 ? 1 :
		(strcmpi( ((GameFile*)a)->file_name, "AUTO2.SAV" ) == 0 ? 2 : 0);
	if (firstAuto != 1) // only check second if first is not AUTO.SAV
		secondAuto = strcmpi( ((GameFile*)b)->file_name, "AUTO.SAV" ) == 0 ? 1 :
			(strcmpi( ((GameFile*)b)->file_name, "AUTO2.SAV" ) == 0 ? 2 : 0);

	if (firstAuto == 1 || (firstAuto == 2 && secondAuto != 1))
		return -1;
	else if (secondAuto > 0)
		return 1;
	else
		return strcmpi( ((GameFile*)a)->file_name, ((GameFile*)b)->file_name );
}
//------- End of function sort_game_file_function ------//


//------- Begin of function GameFileArray::operator[] -----//

GameFile* GameFileArray::operator[](int recNo)
{
	GameFile* gameFile = (GameFile*) get(recNo);

	return gameFile;
}
//--------- End of function GameFileArray::operator[] ----//


//------- Begin of static function disp_scroll_bar_func --------//
static void disp_scroll_bar_func(SlideVBar *scroll, int)
{
    short rectTop = scroll->rect_top();
    short rectBottom = scroll->rect_bottom();
    vga_front.bar( scroll->scrn_x1, rectTop, scroll->scrn_x2, rectBottom, VGA_YELLOW+1);
    if( rectBottom - rectTop > 6 )
    {
        vga_front.d3_panel_up(scroll->scrn_x1, rectTop, scroll->scrn_x2, rectBottom,2,0);
    }
}
//------- End of static function disp_scroll_bar_func --------//
