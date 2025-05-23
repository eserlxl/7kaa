/*
 * Seven Kingdoms: Ancient Adversaries
 *
 * Copyright 1997,1998 Enlight Software Ltd.
 * Copyright 2010,2015 Jesse Allen
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

//Filename    : OMOUSE.H
//Description : Input event class

#ifndef __OMOUSE_H
#define __OMOUSE_H

//-------- Define macro constant --------//
//
// Button id, for Mouse internal use only,
// not appeal to mouse driver parameters
//
//---------------------------------------//

// event_type
enum MouseEventType
{
	LEFT_BUTTON = 0,
	RIGHT_BUTTON = LEFT_BUTTON+1,
	KEY_PRESS = 2,
	LEFT_BUTTON_RELEASE = 4,
	RIGHT_BUTTON_RELEASE = LEFT_BUTTON_RELEASE+1,
	KEY_RELEASE = 6,
	KEY_TYPING = 7,
};

enum KeyEventType : int
{
	KEYEVENT_UNSET = 0,

	KEYEVENT_FIRM_BUILD,
	KEYEVENT_FIRM_PATROL,

	KEYEVENT_TOWN_RECRUIT,
	KEYEVENT_TOWN_TRAIN,

	KEYEVENT_UNIT_BUILD,
	KEYEVENT_UNIT_RETURN,
	KEYEVENT_UNIT_SETTLE,
	KEYEVENT_UNIT_UNLOAD,
	KEYEVENT_UNIT_CANCEL,

	KEYEVENT_BUILD_BASE,
	KEYEVENT_BUILD_CAMP,
	KEYEVENT_BUILD_FACTORY,
	KEYEVENT_BUILD_HARBOR,
	KEYEVENT_BUILD_INN,
	KEYEVENT_BUILD_MARKET,
	KEYEVENT_BUILD_MINE,
	KEYEVENT_BUILD_MONSTER,
	KEYEVENT_BUILD_RESEARCH,
	KEYEVENT_BUILD_WAR_FACTORY,

	KEYEVENT_MAP_MODE_CYCLE,
	KEYEVENT_MAP_MODE0,
	KEYEVENT_MAP_MODE1,
	KEYEVENT_MAP_MODE2,
	KEYEVENT_REPORT_OPAQUE_TOGGLE,
	KEYEVENT_CLEAR_NEWS,
	KEYEVENT_OPEN_DIPLOMATIC_MSG,
	KEYEVENT_OPEN_OPTION_MENU,

	KEYEVENT_TUTOR_PREV,
	KEYEVENT_TUTOR_NEXT,

	KEYEVENT_SAVE_GAME,
	KEYEVENT_LOAD_GAME,

	KEYEVENT_OBJECT_PREV,
	KEYEVENT_OBJECT_NEXT,
	KEYEVENT_NATION_OBJECT_PREV,
	KEYEVENT_NATION_OBJECT_NEXT,

	KEYEVENT_GOTO_RAW,
	KEYEVENT_GOTO_KING,
	KEYEVENT_GOTO_GENERAL,
	KEYEVENT_GOTO_SPY,
	KEYEVENT_GOTO_SHIP,
	KEYEVENT_GOTO_CAMP,
	KEYEVENT_GOTO_SELECTED,

	KEYEVENT_CHEAT_ENABLE1,
	KEYEVENT_CHEAT_ENABLE2,
	KEYEVENT_CHEAT_ENABLE3,

	KEYEVENT_MANUF_QUEUE_UP,
	KEYEVENT_MANUF_QUEUE_DOWN,
	KEYEVENT_MANUF_QUEUE_ADD,
	KEYEVENT_MANUF_QUEUE_ADD_BATCH,
	KEYEVENT_MANUF_QUEUE_REMOVE,
	KEYEVENT_MANUF_QUEUE_REMOVE_BATCH,

	KEYEVENT_SEL_GROUP_1,
	KEYEVENT_SEL_GROUP_2,
	KEYEVENT_SEL_GROUP_3,
	KEYEVENT_SEL_GROUP_4,
	KEYEVENT_SEL_GROUP_5,
	KEYEVENT_SEL_GROUP_6,
	KEYEVENT_SEL_GROUP_7,
	KEYEVENT_SEL_GROUP_8,
	KEYEVENT_SEL_GROUP_9,

	KEYEVENT_SET_GROUP_1,
	KEYEVENT_SET_GROUP_2,
	KEYEVENT_SET_GROUP_3,
	KEYEVENT_SET_GROUP_4,
	KEYEVENT_SET_GROUP_5,
	KEYEVENT_SET_GROUP_6,
	KEYEVENT_SET_GROUP_7,
	KEYEVENT_SET_GROUP_8,
	KEYEVENT_SET_GROUP_9,

	KEYEVENT_MAX
};
#define GETKEY(n) mouse.get_key_code(n)
#define ISKEY(n) mouse.is_key_event(n)

//------- Define struct MouseEvent --------//

struct MouseEvent               // event buffer structure
{
	MouseEventType	event_type;   // mouse state

	unsigned long  time;               // time event occurred
	unsigned short skey_state;   // speical key state, such as LEFT_SHIFT_KEY_MASK ...
	// int	state;              // state mask of mouse button

	int      x, y;               // mousecursor coordinates
	unsigned scan_code;          // if scan_code>0 then it's a key press event
	char typing;
};

#define LEFT_BUTTON_MASK	1
#define RIGHT_BUTTON_MASK	2


// bit flag of any skey_state, event_skey_state
#define LEFT_SHIFT_KEY_MASK 0x001
#define RIGHT_SHIFT_KEY_MASK 0x002
#define SHIFT_KEY_MASK 0x003
#define LEFT_CONTROL_KEY_MASK 0x004
#define RIGHT_CONTROL_KEY_MASK 0x008
#define CONTROL_KEY_MASK 0x00c
#define LEFT_ALT_KEY_MASK 0x010
#define RIGHT_ALT_KEY_MASK 0x020
#define ALT_KEY_MASK 0x030
#define NUM_LOCK_STATE_MASK 0x040
#define CAP_LOCK_STATE_MASK 0x080
#define SCROLL_LOCK_STATE_MASK 0x100
#define INSERT_STATE_MASK 0x200
#define GRAPH_KEY_MASK 0x400

//----- Define the upper limit for mouse coordination ------//

#define MOUSE_X_UPPER_LIMIT   (VGA_WIDTH-5)
#define MOUSE_Y_UPPER_LIMIT	(VGA_HEIGHT-5)

//------ Default settting ---------//

#define DEFAULT_DOUBLE_SPEED_THRESHOLD 8
#define DEFAULT_TRIPLE_SPEED_THRESHOLD 16

//-------- Define struct MouseClick -------//

struct MouseClick               // MultiClick buffer structure
{
	int x, y;
	int release_x, release_y;	// where mouse is release
	int count;          // number of clicks
	unsigned long time;           // time of last click
	unsigned long release_time;	 // time of last release
};

//--------- Define class Mouse ------------//

class Mouse
{
private:
	char  init_flag;
	char* vga_update_buf;

	// ------ mouse setting ---------- //
	int	double_speed_threshold;				// default DEFAULT_DOUBLE_SPEED_THRESHOLD
	int	triple_speed_threshold;				// default DEFAULT_TRIPLE_SPEED_THRESHOLD

	//-------- click buffer ---------//
	MouseClick click_buffer[2];    // left button & right button only

	//-------- event buffer ---------//

	enum { EVENT_BUFFER_SIZE = 20 };  // No. of events can be stored in buffer

	MouseEvent event_buffer[EVENT_BUFFER_SIZE];

	int head_ptr;        // head pointer to the event buffer
	int tail_ptr;        // tail pointer to the event buffer

public:
	char handle_flicking;

	//------- real-time mouse state -------//

	int cur_x, cur_y;
	int left_press, right_press;

	//------- real-time keyboard state ---------//
	unsigned short skey_state;		// such as LEFT_SHIFT_KEY_MASK

	//------- boundary of mouse cursor hot spot --------//
	int	bound_x1;
	int	bound_y1;
	int	bound_x2;
	int	bound_y2;

	//-------- click & key buffer ---------//

	unsigned short event_skey_state;
	char has_mouse_event;		 // if has_mouse_event, mouse_event_type is valid
	MouseEventType mouse_event_type;
											 // use : LEFT_BUTTON=0, RIGHT_BUTTON=1
	unsigned scan_code;             // key pressed, keyboard event
	unsigned key_code;				// converted from scan_code and event_skey_state
	unsigned unique_key_code;	// any key pressed unfiltered by modifiers
	char typing_char;

	//-------- wheel/touch scrolling ---------//

	bool scrolling = false;
	int scroll_sensitivity = 10;
	int scroll_x = 0, scroll_y = 0;
	double scroll_prev_y = 0.0, scroll_prev_x = 0.0;

public:
	Mouse();
	~Mouse();

	void	init();
	void	init_key();
	void	deinit();

	void 	add_event(MouseEvent *);
	void 	add_event(MouseEventType type);
	void 	add_key_event(unsigned, unsigned long);
	int  	get_event();
	void	poll_event();
	void    process_mouse_motion(int x, int y);
	// #### begin Gilbert 31/10 #########//
	void	update_skey_state();
	// #### end Gilbert 31/10 #########//

	void 	show();
	void 	hide();
	void 	hide_area(int,int,int,int);
	void 	show_area();

	//--- functions for real-time mouse state accessing ---//

	int  	in_area(int,int,int,int);
	int  	press_area(int,int,int,int,int=0);
	int 	wait_press(int timeOutSecond=0);
	bool    get_scroll(int * x, int * y);
	void    process_scroll(int x, int y);
	void    process_scroll(double x, double y);
	void    end_scroll();

	//---- functions of mouse cursor boundary ------//
	void	set_boundary(int, int, int, int);
	void	reset_boundary();

	//----- functions for queue event accessing ----//

	int  	single_click(int,int,int,int,int=0);
	int  	double_click(int,int,int,int,int=0);
	int  	any_click   (int,int,int,int,int=0);
	int  	any_click(int=0);
	int	release_click(int,int,int,int,int=0);

	int  	click_x(int buttonId=0)     { return click_buffer[buttonId].x; }
	int  	click_y(int buttonId=0)     { return click_buffer[buttonId].y; }
	int  	release_x(int buttonId=0)   { return click_buffer[buttonId].release_x; }
	int  	release_y(int buttonId=0)   { return click_buffer[buttonId].release_y; }
	int  	click_count(int buttonId=0) { return click_buffer[buttonId].count; }

	int	is_mouse_event()            { return has_mouse_event; }
	int	is_key_event()              { return scan_code; }
	int	is_any_event()              { return has_mouse_event || scan_code; }
	int	is_press_button_event()     { return has_mouse_event && (mouse_event_type == LEFT_BUTTON || mouse_event_type == RIGHT_BUTTON); }
	int	is_release_button_event()   { return has_mouse_event && (mouse_event_type == LEFT_BUTTON_RELEASE || mouse_event_type == RIGHT_BUTTON_RELEASE); }

	void	reset_click();

	static int is_key(unsigned keyCode, unsigned short skeyState, unsigned short charValue, unsigned flags = 0 );
	static int is_key(unsigned keyCode, unsigned short skeyState, char *keyStr, unsigned flags = 0 );
	// see omouse2.h for flags

	void disp_count_start();
	void disp_count_end();

	int bind_key(KeyEventType key_event, const char *key);
	int is_key_event(KeyEventType key_event);
	unsigned get_key_code(KeyEventType key_event);
	void add_typing_event(char *text, unsigned long timeStamp);

private:
	int micky_to_displacement(int d);
};
//---------- End of define class ---------------//

extern Mouse mouse;

#endif
