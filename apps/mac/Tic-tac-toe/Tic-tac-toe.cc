/*
	Tic-tac-toe.cc
	--------------
*/

// Mac OS X
#ifdef __APPLE__
#include <Carbon/Carbon.h>
#endif

// Mac OS
#ifndef __MACWINDOWS__
#include <MacWindows.h>
#endif

// missing-macos
#ifdef MAC_OS_X_VERSION_10_7
#ifndef MISSING_QUICKDRAW_H
#include "missing/Quickdraw.h"
#endif
#endif

// mac-qd-utils
#include "mac_qd/get_portRect.hh"
#include "mac_qd/main_display_bounds.hh"
#include "mac_qd/wide_drag_area.hh"
#include "mac_qd/globals/arrow.hh"

// Tic-tac-toe
#include "cursors.hh"
#include "regions.hh"
#include "state.hh"


using mac::qd::get_portRect;
using mac::qd::main_display_bounds;
using mac::qd::wide_drag_area;

using tictactoe::move_t;
using tictactoe::player_t;

static player_t current_player = tictactoe::Player_X;

static inline
short min( short a, short b )
{
	return b < a ? b : a;
}

static
void InvalRect( WindowRef window, const Rect& rect )
{
#if ! TARGET_API_MAC_CARBON
	
	InvalRect( &rect );
	return;
	
#endif
	
	InvalWindowRect( window, &rect );
}

static const Rect grow_size =
{
	50, 50,
	32767, 32767,
};

static RgnHandle mouseRgns[ 10 ];
static RgnHandle otherRgn;
static RgnHandle gMouseRgn;

static
void alloc_mouseRgns()
{
	for ( short i = 0;  i < 10;  ++i )
	{
		mouseRgns[ i ] = NewRgn();
	}
	
	gMouseRgn = otherRgn = mouseRgns[ 0 ];
}

/*
	      *     *
	      *     *
	  * * * * * * * *
	      *     *
	      *     *
	  * * * * * * * *
	      *     *
	      *     *
	
	| |<-     3u
	 |<-->|   8u
	    -> <- 1u
	
	3 + 8 + 1 + 8 + 1 + 8 + 3 == 32
*/

enum
{
	kMargin = 3,
};

static WindowRef main_window;

static
void make_main_window()
{
	const short alignment = 16;
	
	Rect bounds = main_display_bounds();
	
	bounds.top    += 40;  // approximate menu bar height + title bar height
	bounds.bottom -=  8;  // approximate window frame thickness
	
	const short usableHeight = bounds.bottom - bounds.top;
	
	short height = usableHeight;
	
	height -= height /  4;  // leave some space
	height -= height % 16;  // make it an exact multiple
	
	const short margin = usableHeight - height;
	
	bounds.top   += margin / 2;
	bounds.bottom = bounds.top + height;
	
	bounds.left  = (bounds.right - bounds.left - height) / 2;
	bounds.right = bounds.left + height;
	
	main_window = NewWindow( NULL,
	                         &bounds,
	                         "\p" "Tic-tac-toe",
	                         true,
	                         documentProc,
	                         (WindowRef) -1,
	                         true,
	                         0 );
	
	SetPortWindowPort( main_window );
}

static
void draw_window( WindowRef window )
{
	SetPortWindowPort( window );
	
	const Rect& portRect = get_portRect( window );
	
	EraseRect( &portRect );
	
	const short portWidth  = portRect.right - portRect.left;
	const short portHeight = portRect.bottom - portRect.top;
	
	const short portLength = min( portWidth, portHeight );
	
	const short unitLength = portLength / 32;
	
	Rect line;
	
	line.left  = unitLength * 3;
	line.right = unitLength * 29;
	
	line.top    = unitLength * 11;
	line.bottom = line.top + unitLength;
	
	PaintRect( &line );
	
	line.top    = unitLength * 20;
	line.bottom = line.top + unitLength;
	
	PaintRect( &line );
	
	line.top    = unitLength * 3;
	line.bottom = unitLength * 29;
	
	line.left  = unitLength * 11;
	line.right = line.left + unitLength;
	
	PaintRect( &line );
	
	line.left  = unitLength * 20;
	line.right = line.left + unitLength;
	
	PaintRect( &line );
	
	const short width  = unitLength * 6;
	const short height = unitLength * 6;
	
	Rect rect = { 0, 0, height, width };
	
	for ( short i = 0, i3 = 0;  i < 3;  ++i, i3 += 3 )
	{
		const short top = unitLength * (4 + 9 * i);
		
		for ( short j = 0;  j < 3;  ++j )
		{
			const short left = unitLength * (4 + 9 * j);
			
			if ( player_t token = tictactoe::get( i3 + j ) )
			{
				SetOrigin( -left, -top );
				
				RgnHandle rgn = token > 0 ? get_X_token( unitLength )
				                          : get_O_token( unitLength );
				
				PaintRgn( rgn );
			}
		}
	}
	
	SetOrigin( 0, 0 );
}

static
void draw_token( WindowRef window, player_t token, short index )
{
	const Rect& portRect = get_portRect( window );
	
	const short portWidth  = portRect.right - portRect.left;
	const short portHeight = portRect.bottom - portRect.top;
	
	const short portLength = min( portWidth, portHeight );
	
	const short unitLength = portLength / 32;
	
	const short i = index / 3;
	const short j = index % 3;
	
	const short top = unitLength * (4 + 9 * i);
	const short left = unitLength * (4 + 9 * j);
	
	SetOrigin( -left, -top );
	
	RgnHandle rgn = token > 0 ? get_X_token( unitLength )
	                          : get_O_token( unitLength );
	
	PaintRgn( rgn );
	
	SetOrigin( 0, 0 );
}

static
short hit_test( WindowRef window, Point where )
{
	const Rect& portRect = get_portRect( window );
	
	const short portWidth  = portRect.right - portRect.left;
	const short portHeight = portRect.bottom - portRect.top;
	
	const short portLength = min( portWidth, portHeight );
	
	const short unitLength = portLength / 32;
	
	short x = where.h / unitLength;
	short y = where.v / unitLength;
	
	x -= 3;
	y -= 3;
	
	if ( (unsigned short) x > 25  ||  (unsigned short) y > 25 )
	{
		// hit in margin
		return -1;
	}
	
	if ( ++x % 9 == 0  ||  ++y % 9 == 0 )
	{
		// hit on a grid line
		return -1;
	}
	
	return y / 9 * 3 + x / 9;
}

static
void click( WindowRef window, Point where )
{
	if ( ! current_player )
	{
		return;
	}
	
	using namespace tictactoe;
	
	short i = hit_test( window, where );
	
	move_t result = move( current_player, i );
	
	if ( result < Move_ok )
	{
		return;
	}
	
	SetCursor( &mac::qd::arrow() );
	
	RgnHandle rgn = mouseRgns[ 1 + i ];
	
	gMouseRgn = mouseRgns[ 1 + i ] = otherRgn;
	
	XorRgn( otherRgn, rgn, otherRgn );
	
	DisposeRgn( rgn );
	
	draw_token( window, current_player, i );
	
	if ( result == Move_ok )
	{
		current_player = opponent( current_player );
	}
	else if ( result > Move_ok )
	{
		current_player = Player_none;
	}
}

static
void calibrate_mouseRgns( WindowRef window )
{
	Point globalOffset = { 0, 0 };
	
	LocalToGlobal( &globalOffset );
	
	const Rect& portRect = get_portRect( window );
	
	const short portWidth  = portRect.right - portRect.left;
	const short portHeight = portRect.bottom - portRect.top;
	
	const short portLength = min( portWidth, portHeight );
	
	const short unitLength = portLength / 32;
	
	Rect rect = { -32000, -32000, 32000, 32000 };
	
	RectRgn( otherRgn, &rect );
	
	for ( short i = 0, i3 = 0;  i < 3;  ++i, i3 += 3 )
	{
		rect.top    = globalOffset.v + unitLength * (3 + 9 * i);
		rect.bottom = globalOffset.v + unitLength * (3 + 9 * i + 8);
		
		for ( short j = 0;  j < 3;  ++j )
		{
			rect.left  = globalOffset.h + unitLength * (3 + 9 * j);
			rect.right = globalOffset.h + unitLength * (3 + 9 * j + 8);
			
			RgnHandle rgn = mouseRgns[ 1 + i3 + j ];
			
			if ( rgn != otherRgn )
			{
				RectRgn( rgn, &rect );
				
				XorRgn( otherRgn, rgn, otherRgn );
			}
		}
	}
}

static
RgnHandle mouse_moved( Point where )
{
	GlobalToLocal( &where );
	
	const short i = hit_test( main_window, where );
	
	const Cursor* cursor;
	
	if ( current_player  &&  tictactoe::can_move( i ) )
	{
		cursor = current_player > 0 ? &X_cursor : &O_cursor;
	}
	else
	{
		cursor = &mac::qd::arrow();
	}
	
	SetCursor( cursor );
	
	return mouseRgns[ 1 + i ];
}

int main()
{
	Boolean quitting = false;
	
#if ! TARGET_API_MAC_CARBON
	
	InitGraf( &qd.thePort );
	
	InitFonts();
	InitWindows();
	InitMenus();
	
	InitCursor();
	
#endif
	
	alloc_mouseRgns();
	
	make_main_window();
	
	calibrate_mouseRgns( main_window );
	
	while ( ! quitting )
	{
		EventRecord event;
		
		if ( WaitNextEvent( everyEvent, &event, 0x7FFFFFFF, gMouseRgn ) )
		{
			WindowRef window;
			
			switch ( event.what )
			{
				case mouseDown:
					switch ( FindWindow( event.where, &window ) )
					{
						case inMenuBar:
							break;
						
						case inDrag:
							DragWindow( window, event.where, wide_drag_area() );
							calibrate_mouseRgns( window );
							break;
						
						case inGoAway:
							if ( TrackGoAway( window, event.where ) )
							{
								DisposeWindow( main_window );
								
								main_window = NULL;
								quitting = true;
							}
							break;
						
						case inContent:
							GlobalToLocal( &event.where );
							click( window, event.where );
							break;
						
						case inGrow:
							if ( long grew = GrowWindow( window, event.where, &grow_size ) )
							{
								SizeWindow( window, grew, grew >> 16, true );
								
								InvalRect( window, get_portRect( window ) );
								
								calibrate_mouseRgns( window );
							}
							break;
						
						default:
							break;
					}
					break;
				
				case keyDown:
					switch( (char) event.message )
					{
						case 'q':
							quitting = true;
							break;
						
						default:
							break;
					}
					break;
				
				case updateEvt:
					window = (WindowRef) event.message;
					
					BeginUpdate( window );
					draw_window( window );
					EndUpdate  ( window );
					break;
				
				case osEvt:
					switch ( (event.message & osEvtMessageMask) >> 24 )
					{
						case mouseMovedMessage:
							gMouseRgn = mouse_moved( event.where );
							break;
						
						default:
							break;
					}
					break;
				
				default:
					break;
			}
		}
	}
	
	return 0;
}
