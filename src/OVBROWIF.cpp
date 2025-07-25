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

//Filename    : OVBROWIF.CPP
//Description : Object Visual Browse for the Interface

#include <OSYS.h>
#include <OINFO.h>
#include <OVGA.h>
#include <OVBROWIF.h>
#include <vga_util.h>
#include <OMOUSE.h>


//---------- Begin of function VBrowseIF::VBrowseIF -------//
//
VBrowseIF::VBrowseIF()
{
	vga_front_only = 0;  // if 1, then it does all the bitmap processing on the front buffer only
}
//----------- End of function VBrowseIF::VBrowseIF ----------//


//---------- Begin of function VBrowseIF::init_var -------//
//
void VBrowseIF::init_var(int totalRec, int recNo)
{
	VBrowse::init_var(totalRec, recNo);

	scroll_bar.init( 1, x2-SCROLL_BAR_WIDTH, y1, x2, y2, disp_max_rec,
			 (disp_frame && x_max_rec==1 ? 1 : disp_max_rec),    // only use record as scroller unit when it's a vertical browser with frame
			 total_rec_num, 1, 1 );		// last 1 - Interface mode if_flag is 1
}
//----------- End of function VBrowseIF::init_var ----------//


//--------- Begin of function VBrowseIF::paint ---------//
//
void VBrowseIF::paint()
{
	vga_util.d3_panel_down( x1, y1, x2-SCROLL_BAR_WIDTH-3, y2, vga_front_only );  // the list content box

	scroll_bar.paint();
}
//--------- End of function VBrowseIF::paint ---------//


//--------- Begin of function VBrowseIF::refresh ---------//
//
// Reset the no. of record and current record pointer and cause displaying
//
// [int] newRecNo      = new current record no.
//                       (default : -1, no change)
// [int] newTotalRec   = new total no. of records
//                       (default : -1, no change)
//
void VBrowseIF::refresh(int newRecNo, int newTotalRec)
{
	if( !vga_front_only && !vga.use_back_buf )
		vga_util.blt_buf( x1, y1, x2-SCROLL_BAR_WIDTH-3, y2, 0 );  // the list content box

	VBrowse::refresh(newRecNo, newTotalRec);
}
//--------- End of function VBrowseIF::refresh ---------//


//-------- Begin of function VBrowseIF::disp_all ----------//
//
// Display all records on screen, highlight cur_rec_no
//
void VBrowseIF::disp_all()
{
	int recNo;

	if( !vga_front_only && !vga.use_back_buf )
		vga_util.blt_buf( ix1, iy1, ix2, iy2, 0 ); // clear background

	int scrollRecno = (disp_frame && x_max_rec==1) ? rec_no : top_rec_no;

	scroll_bar.refresh( scrollRecno, 1, disp_max_rec,
			 (disp_frame && x_max_rec==1 ? 1 : disp_max_rec),
			 total_rec_num );

	int y = iy1;
	for( recNo=top_rec_no ; recNo<=total_rec_num && recNo<top_rec_no+disp_max_rec ; recNo++ )
	{
		int is_selected = (recNo == rec_no);
		int highlight_extra = is_selected ? 4 : 0;
		int y_offset = is_selected ? 2 : 0;
		int row_height = rec_height + highlight_extra;
		int x = ix1;

		if( disp_frame && is_selected ) {
			Vga::active_buf->draw_d3_up_border(x-2, y-2, x+rec_width+1, y+rec_height+1+highlight_extra);
			disp_rec( recNo, x, y + y_offset, INFO_REPAINT );
		} else if( !is_selected ) {
			disp_rec( recNo, x, y, INFO_REPAINT );
		}
		y += row_height + rec_y_space;
	}
}
//--------- End of function VBrowseIF::disp_all ----------//


//-------- Begin of function VBrowseIF::disp_one ----------//
//
// <int> recNo     = the no. of the record to display
// <int> putType   = DISP_REC  - display record
//                   DISP_HIGH - highlight rect
//                   CLEAR_HIGH- clear highlight rect
//
void VBrowseIF::disp_one(int recNo, int dispType)
{
	if( none_record )
		return;

	int x, y;

	// Increase the height and y-position of the selected row to prevent overlap
	int highlight_extra = (dispType == DISP_HIGH) ? 4 : 0;
	int y_offset = (dispType == DISP_HIGH) ? 2 : 0;

	y = iy1 + (recNo-top_rec_no)/x_max_rec * (rec_height+rec_y_space);
	x = ix1 + (recNo-top_rec_no)%x_max_rec * (rec_width+rec_x_space);

	if( disp_frame && dispType == CLEAR_HIGH && !vga.use_back_buf && !vga_front_only )
	{
		vga_util.blt_buf( x-2, y-2, x+rec_width+1, y-2, 0 );
		vga_util.blt_buf( x-2, y+rec_height+1+highlight_extra, x+rec_width+1, y+rec_height+1+highlight_extra, 0 );
		vga_util.blt_buf( x-2, y-2, x-2, y+rec_height+1+highlight_extra, 0 );
		vga_util.blt_buf( x+rec_width+1, y-2, x+rec_width+1, y+rec_height+1+highlight_extra, 0 );
	}

	if( dispType == DISP_REC ) {
		disp_rec( recNo, x, y, INFO_REPAINT );  // call user defined function at normal y
	}
	else if( dispType == DISP_HIGH ) {
		// Draw highlight border and content at shifted y
		int content_y = y + y_offset;
		Vga::active_buf->draw_d3_up_border(x-2, y-2, x+rec_width+1, y+rec_height+1+highlight_extra);
		disp_rec( recNo, x, content_y, INFO_REPAINT );
		int scrollRecno = (disp_frame && x_max_rec==1) ? recNo : top_rec_no;
		scroll_bar.refresh( scrollRecno, 0, disp_max_rec,
			 (disp_frame && x_max_rec==1 ? 1 : disp_max_rec),
					total_rec_num );
	}
}
//--------- End of function VBrowseIF::disp_one ----------//


//-------- Begin of function VBrowseIF::disp_rec --------//
//
void VBrowseIF::disp_rec(int recNo, int x, int y, int refreshFlag)
{
	(*disp_rec_func)( recNo, x, y, refreshFlag );  // call user defined function

	sys.yield();
}
//---------- End of function VBrowseIF::disp_rec -----------//


//-------- Begin of function VBrowseIF::mouse_over ----------//
//
// return the number of the record the mouse is over
// optionally passes the geometry of the box the mouse is hovering over
// if not over a record, the return is zero, and geometry is not initialized
int VBrowseIF::mouse_over(int *x1, int *y1, int *x2, int *y2)
{
	int recNo, recX, recY;
	for( recNo=top_rec_no ; recNo<=total_rec_num && recNo<top_rec_no+disp_max_rec ; recNo++ )
	{
		recY = iy1 + (recNo-top_rec_no)/x_max_rec * (rec_height+rec_y_space);
		recX = ix1 + (recNo-top_rec_no)%x_max_rec * (rec_width+rec_x_space);
		if( mouse.in_area(recX, recY, recX+rec_width, recY+rec_height) )
		{
			if( x1 )
				*x1 = recX;
			if( y1 )
				*y1 = recY;
			if( x2 )
				*x2 = recX+rec_width;
			if( y2 )
				*y2 = recY+rec_height;
			return recNo;
		}
	}
	return 0;
}
