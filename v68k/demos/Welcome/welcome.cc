/*
	welcome.cc
	----------
*/

// Mac OS X
#ifdef __APPLE__
#include <Carbon/Carbon.h>
#endif

// Mac OS
#ifndef __GESTALT__
#include <Gestalt.h>
#endif
#ifndef __MACWINDOWS__
#include <MacWindows.h>
#endif

// missing-macos
#ifdef MAC_OS_X_VERSION_10_7
#ifndef MISSING_QUICKDRAW_H
#include "missing/Quickdraw.h"
#endif
#ifndef MISSING_QUICKDRAWTEXT_H
#include "missing/QuickdrawText.h"
#endif
#endif

// mac-qd-utils
#include "mac_qd/get_portRect.hh"
#include "mac_qd/main_display_bounds.hh"


using mac::qd::get_portRect;
using mac::qd::main_display_bounds;


static inline
bool in_v68k()
{
	SInt32 result;
	OSErr err = Gestalt( 'v68k', &result );
	
	return err == noErr;
}

static
bool has_color_quickdraw()
{
	SInt32 result;
	OSErr err = Gestalt( 'qd  ', &result );
	
	return err == noErr  &&  result > 0;
}

static
WindowRef new_window( const Rect*           bounds,
                      const unsigned char*  title,
                      Boolean               vis,
                      short                 proc,
                      WindowRef             behind,
                      Boolean               closable,
                      SInt32                ref )
{
	static bool has_color = has_color_quickdraw();
	
	if ( has_color )
	{
		return NewCWindow( 0, bounds, title, vis, proc, behind, closable, ref );
	}
	else
	{
		return NewWindow( 0, bounds, title, vis, proc, behind, closable, ref );
	}
}

static
WindowRef create_window()
{
	Rect bounds = main_display_bounds();
	
	const short width  = 428;
	const short height = 112;
	
	bounds.left  = (bounds.right - width) / 2;
	bounds.right = bounds.left + width;
	
	bounds.top    = (bounds.bottom - height) / 3 - 4;
	bounds.bottom = bounds.top + height;
	
	return new_window( &bounds, "\p", true, dBoxProc, (WindowRef) -1, false, 0 );
}

#define WELCOME( name ) "\p" "Welcome to " name "."

static
void draw_window( WindowRef window )
{
	OSErr err;
	
	const Rect& portRect = get_portRect( window );
	
	SetPortWindowPort( window );
	
	EraseRect( &portRect );
	
	SInt32 micn = 0;
	err = Gestalt( 'micn', &micn );
	
	const short icon_h = 16;
	const short icon_v = 16;
	
	const Rect icon_rect = { icon_v, icon_h, icon_v + 32, icon_h + 32 };
	
	PlotIconID( &icon_rect, 0, 0, micn );
	
	const short text_h = 100;
	const short text_v = 36;
	
	MoveTo( text_h, text_v );
	
	DrawString( "\p" "Welcome to Advanced Mac Substitute." );
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
	
	if ( TARGET_CPU_68K  &&  in_v68k() )
	{
		HideCursor();
		
		FillRect( &qd.screenBits.bounds, &qd.gray );
	}
	
#endif
	
	WindowRef main_window = create_window();
	
	while ( ! quitting )
	{
		EventRecord event;
		
		if ( WaitNextEvent( everyEvent, &event, 0x7FFFFFFF, NULL ) )
		{
			WindowRef window;
			
			switch ( event.what )
			{
				case mouseDown:
				case keyDown:
					quitting = true;
					break;
				
				case updateEvt:
					window = (WindowRef) event.message;
					BeginUpdate( window );
					
					draw_window( window );
					
					EndUpdate  ( window );
					break;
				
				default:
					break;
			}
		}
	}
	
	return 0;
}
