/*	========================
 *	FSTree_sys_window_REF.cc
 *	========================
 */

#include "Genie/FileSystem/FSTree_sys_window_REF.hh"

// Nucleus
#include "Nucleus/Saved.h"

// ClassicToolbox
#include "ClassicToolbox/MacWindows.h"

// POSeven
#include "POSeven/Errno.hh"

// Nitrogen
#include "Nitrogen/MacWindows.h"

// Pedestal
#include "Pedestal/EmptyView.hh"
#include "Pedestal/Window.hh"

// Genie
#include "Genie/FileSystem/FSTree_Property.hh"
#include "Genie/FileSystem/ResolvePathname.hh"
#include "Genie/FileSystem/Views.hh"
#include "Genie/IO/Terminal.hh"
#include "Genie/Process.hh"


namespace Genie
{
	
	namespace N = Nitrogen;
	namespace NN = Nucleus;
	namespace p7 = poseven;
	namespace Ped = Pedestal;
	
	
	static const Point gZeroPoint = { 0 };
	
	struct WindowParameters
	{
		N::Str255  itsTitle;
		Point      itsOrigin;
		Point      itsSize;
		bool       itIsVisible;
		
		boost::shared_ptr< Ped::Window >  itsWindow;
		boost::shared_ptr< Ped::View >  itsSubview;
		
		FSTreePtr                    itsTTYDelegate;
		boost::weak_ptr< IOHandle >  itsTerminal;
		
		WindowParameters() : itsOrigin( gZeroPoint ),
		                     itsSize  ( gZeroPoint ),
		                     itIsVisible( true ),
		                     itsSubview( Ped::EmptyView::Get() )
		{
		}
	};
	
	typedef std::map< const FSTree*, WindowParameters > WindowParametersMap;
	
	static WindowParametersMap gWindowParametersMap;
	
	
	static void CenterWindowRect( Rect& bounds )
	{
		// Pre-conditions:  bounds is set to { 0, 0, v, h }
		
		const short topMargin = ::GetMBarHeight() + 18;  // FIXME:  Calculate title bar height
		
		BitMap screenBits = N::GetQDGlobalsScreenBits();
		
		short spareWidth = screenBits.bounds.right - bounds.right;
		
		short spareHeight = screenBits.bounds.bottom - bounds.bottom - topMargin;
		
		::OffsetRect( &bounds,
		              spareWidth / 2,
		              topMargin + spareHeight / 3 );
	}
	
	
	class Window : public Ped::Window
	{
		private:
			const FSTree* itsKey;
		
		public:
			Window( const FSTree*                 key,
			        const Ped::NewWindowContext&  context,
			        N::WindowDefProcID            defProcID = Nitrogen::documentProc )
			:
				Ped::Window( context, defProcID ),
				itsKey( key )
			{
			}
			
			boost::shared_ptr< Ped::View >& GetView();
	};
	
	boost::shared_ptr< Ped::View >& Window::GetView()
	{
		return gWindowParametersMap[ itsKey ].itsSubview;
	}
	
	
	static void CloseUserWindow( const FSTree* key )
	{
		WindowParametersMap::iterator it = gWindowParametersMap.find( key );
		
		if ( it != gWindowParametersMap.end() )
		{
			WindowParameters& params = it->second;
			
			if ( params.itsTerminal.expired() )
			{
				// tty file is not open for this window, just close the window
				
				if ( params.itsWindow.get() )
				{
					params.itsWindow->GetView()->Uninstall();
					
					params.itsWindow.reset();
				}
			}
			else
			{
				const boost::shared_ptr< IOHandle >& handle = params.itsTerminal.lock();
				
				TerminalHandle& terminal( IOHandle_Cast< TerminalHandle >( *handle ) );
				
				terminal.Disconnect();
			}
		}
	}
	
	class UserWindowCloseHandler : public Ped::WindowCloseHandler
	{
		private:
			const FSTree* itsKey;
		
		public:
			UserWindowCloseHandler( const FSTree* key ) : itsKey( key )
			{
			}
			
			void operator()( N::WindowRef ) const  { CloseUserWindow( itsKey ); }
	};
	
	class WindowResizeHandler : public Ped::WindowResizeHandler
	{
		private:
			const FSTree* itsKey;
		
		public:
			WindowResizeHandler( const FSTree* key ) : itsKey( key )
			{
			}
			
			void operator()( N::WindowRef window, short h, short v ) const;
	};
	
	void WindowResizeHandler::operator()( N::WindowRef window, short h, short v ) const
	{
		N::SizeWindow( window, h, v, true );
		
		WindowParametersMap::iterator it = gWindowParametersMap.find( itsKey );
		
		if ( it != gWindowParametersMap.end() )
		{
			WindowParameters& params = it->second;
			
			if ( !params.itsTerminal.expired() )
			{
				const boost::shared_ptr< IOHandle >& handle = params.itsTerminal.lock();
				
				TerminalHandle& terminal( IOHandle_Cast< TerminalHandle >( *handle ) );
				
				if ( !terminal.GetProcessGroup().expired() )
				{
					SendSignalToProcessGroup( SIGWINCH, *terminal.GetProcessGroup().lock() );
				}
			}
		}
	}
	
	
	static bool HasWindow( const FSTree* that )
	{
		const FSTree* key = that->ParentRef().get();
		
		return gWindowParametersMap[ key ].itsWindow != NULL;
	}
	
	static void CreateUserWindow( const FSTree* key )
	{
		WindowParametersMap::iterator it = gWindowParametersMap.find( key );
		
		if ( it == gWindowParametersMap.end() )
		{
			p7::throw_errno( EPERM );
		}
		
		WindowParameters& params = it->second;
		
		ConstStr255Param title = params.itsTitle;
		
		Rect bounds = { 0, 0, 90, 120 };
		
		if ( params.itsSize.h || params.itsSize.v )
		{
			reinterpret_cast< Point* >( &bounds )[1] = params.itsSize;
		}
		
		if ( params.itsOrigin.h || params.itsOrigin.v )
		{
			::OffsetRect( &bounds, params.itsOrigin.h, params.itsOrigin.v );
		}
		else
		{
			CenterWindowRect( bounds );
		}
		
		Ped::NewWindowContext context( bounds, title, params.itIsVisible );
		
		boost::shared_ptr< Ped::Window > window( new Window( key, context, N::documentProc ) );
		
		boost::shared_ptr< Ped::WindowCloseHandler > closeHandler( new UserWindowCloseHandler( key ) );
		
		window->SetCloseHandler( closeHandler );
		
		boost::shared_ptr< Ped::WindowResizeHandler > resizeHandler( new WindowResizeHandler( key ) );
		
		window->SetResizeHandler( resizeHandler );
		
		params.itsWindow = window;
		
		params.itsSubview->Install();
	}
	
	void RemoveUserWindow( const FSTree* key )
	{
		CloseUserWindow( key );
		
		gWindowParametersMap.erase( key );
		
		RemoveAllViewParameters( key );
	}
	
	
	static inline bool is_integer( const char* s )
	{
		return std::isdigit( s[ s[0] == '-' ] );
	}
	
	static Point ReadPoint( const char* string )
	{
		const char* p = string;
		
		long x = std::strtol( p, (char**) &p, 10 );
		
		if ( p != string  &&  *p != '\0' )
		{
			while ( *++p )
			{
				if ( is_integer( p ) )
				{
					long y = std::strtol( p, NULL, 10 );
					
					Point result = { y, x };
					
					return result;
				}
			}
		}
		
		throw p7::errno_t( EINVAL );
	}
	
	static std::string WritePoint( Point point, const char* separator )
	{
		std::string result = NN::Convert< std::string >( point.h );
		
		result += separator;
		
		result += NN::Convert< std::string >( point.v );
		
		return result;
	}
	
	
	struct Access_Title
	{
		typedef N::Str255 Value;
		
		static Value const& GetRef( WindowParameters const& params )  { return params.itsTitle; }
		static Value      & GetRef( WindowParameters      & params )  { return params.itsTitle; }
		
		static std::string StringFromValue( const Value& v )
		{
			return NN::Convert< std::string >( v );
		}
		
		static Value ValueFromString( const char* begin, const char* end )
		{
			return N::Str255( begin, end - begin );
		}
	};
	
	struct Access_Origin
	{
		typedef Point Value;
		
		static Value const& GetRef( WindowParameters const& params )  { return params.itsOrigin; }
		static Value      & GetRef( WindowParameters      & params )  { return params.itsOrigin; }
		
		static std::string StringFromValue( const Value& origin )
		{
			return WritePoint( origin, "," );
		}
		
		static Value ValueFromString( const char* begin, const char* end )
		{
			return ReadPoint( begin );
		}
	};
	
	struct Access_Size
	{
		typedef Point Value;
		
		static Value const& GetRef( WindowParameters const& params )  { return params.itsSize; }
		static Value      & GetRef( WindowParameters      & params )  { return params.itsSize; }
		
		static std::string StringFromValue( const Value& size )
		{
			return WritePoint( size, "x" );
		}
		
		static Value ValueFromString( const char* begin, const char* end )
		{
			return ReadPoint( begin );
		}
	};
	
	struct Access_Visible
	{
		typedef bool Value;
		
		static Value const& GetRef( WindowParameters const& params )  { return params.itIsVisible; }
		static Value      & GetRef( WindowParameters      & params )  { return params.itIsVisible; }
		
		static std::string StringFromValue( const Value& vis )
		{
			return vis ? "1" : "0";
		}
		
		static Value ValueFromString( const char* begin, const char* end )
		{
			return begin[ 0 ] != '0';
		}
	};
	
	
	template < class Accessor >
	struct sys_window_REF_Property
	{
		static std::string Read( const FSTree* that, bool binary )
		{
			const FSTree* key = GetViewKey( that );
			
			WindowParametersMap::const_iterator it = gWindowParametersMap.find( key );
			
			if ( it == gWindowParametersMap.end() )
			{
				throw FSTree_Property::Undefined();
			}
			
			return Accessor::StringFromValue( Accessor::GetRef( it->second ) );
		}
		
		static void Write( const FSTree* that, const char* begin, const char* end )
		{
			const FSTree* key = GetViewKey( that );
			
			Accessor::GetRef( gWindowParametersMap[ key ] ) = Accessor::ValueFromString( begin, end );
		}
	};
	
	
	class FSTree_sys_window_REF_Property : public FSTree_Property
	{
		public:
			FSTree_sys_window_REF_Property( const FSTreePtr&    parent,
			                                const std::string&  name,
			                                ReadHook            readHook,
			                                WriteHook           writeHook )
			:
				FSTree_Property( parent,
				                 name,
				                 readHook,
				                 writeHook )
			{
			}
			
			bool IsLink() const  { return HasWindow( this ); }
			
			mode_t FilePermMode() const;
			
			std::string ReadLink() const;
			
			FSTreePtr ResolveLink() const;
	};
	
	mode_t FSTree_sys_window_REF_Property::FilePermMode() const
	{
		return IsLink() ? S_IRUSR | S_IWUSR | S_IXUSR : FSTree_Property::FilePermMode();
	}
	
	std::string FSTree_sys_window_REF_Property::ReadLink() const
	{
		if ( !IsLink() )
		{
			p7::throw_errno( EINVAL );
		}
		
		return "ref/" + Name();
	}
	
	FSTreePtr FSTree_sys_window_REF_Property::ResolveLink() const
	{
		return ResolvePathname( ReadLink(), ParentRef() );
	}
	
	
	class SetWindowPort_Scope
	{
		private:
			NN::Saved< N::Port_Value > savePort;
			
			SetWindowPort_Scope           ( const SetWindowPort_Scope& );
			SetWindowPort_Scope& operator=( const SetWindowPort_Scope& );
		
		public:
			SetWindowPort_Scope()
			{
			}
			
			SetWindowPort_Scope( N::WindowRef window )
			{
				N::SetPortWindowPort( window );
			}
	};
	
	static void InvalidateWindowRef( N::WindowRef window )
	{
		ASSERT( window != NULL );
		
		if ( ::IsPortVisibleRegionEmpty( N::GetWindowPort( window ) ) )
		{
			return;
		}
		
		SetWindowPort_Scope scope( window );
		
		N::InvalRect( N::GetPortBounds( N::GetWindowPort( window ) ) );
	}
	
	static N::WindowRef GetWindowRef( const FSTree* key )
	{
		WindowParametersMap::const_iterator it = gWindowParametersMap.find( key );
		
		if ( it != gWindowParametersMap.end() )
		{
			const boost::shared_ptr< Ped::Window >& window = it->second.itsWindow;
			
			if ( window.get() != NULL )
			{
				return window->Get();
			}
		}
		
		return NULL;
	}
	
	bool InvalidateWindow( const FSTree* key )
	{
		if ( N::WindowRef window = GetWindowRef( key ) )
		{
			InvalidateWindowRef( window );
			
			return true;
		}
		
		return false;
	}
	
	void InstallViewInWindow( const boost::shared_ptr< Ped::View >& view, const FSTree* key )
	{
		if ( N::WindowRef window = GetWindowRef( key ) )
		{
			SetWindowPort_Scope scope( window );
			
			InvalidateWindowRef( window );
			
			view->Install();
		}
	}
	
	void UninstallViewFromWindow( const boost::shared_ptr< Pedestal::View >& view, const FSTree* key )
	{
		if ( N::WindowRef window = GetWindowRef( key ) )
		{
			SetWindowPort_Scope scope( window );
			
			InvalidateWindowRef( window );
			
			view->Uninstall();
		}
	}
	
	
	class FSTree_sys_window_REF_ref : public FSTree
	{
		public:
			FSTree_sys_window_REF_ref( const FSTreePtr&    parent,
			                           const std::string&  name ) : FSTree( parent, name )
			{
			}
			
			const FSTree* WindowKey() const  { return ParentRef().get(); }
			
			bool Exists() const  { return HasWindow( this ); }
			
			bool IsLink() const  { return Exists(); }
			
			void SetTimes() const;
			
			void Delete() const;
			
			std::string ReadLink() const;
			
			FSTreePtr ResolveLink() const;
	};
	
	
	void FSTree_sys_window_REF_ref::SetTimes() const
	{
		const FSTree* key = WindowKey();
		
		if ( !InvalidateWindow( key ) )
		{
			CreateUserWindow( key );
		}
	}
	
	void FSTree_sys_window_REF_ref::Delete() const
	{
		const FSTree* key = WindowKey();
		
		CloseUserWindow( key );
	}
	
	std::string FSTree_sys_window_REF_ref::ReadLink() const
	{
		N::WindowRef windowPtr = GetWindowRef( WindowKey() );
		
		if ( windowPtr == NULL )
		{
			p7::throw_errno( EINVAL );
		}
		
		std::string result = "/sys/app/window/";
		
		result += Pointer_KeyName_Traits< ::WindowRef >::NameFromKey( windowPtr );
		
		return result;
	}
	
	FSTreePtr FSTree_sys_window_REF_ref::ResolveLink() const
	{
		return ResolvePathname( ReadLink() );
	}
	
	
	class FSTree_sys_window_REF_tty : public FSTree
	{
		public:
			FSTree_sys_window_REF_tty( const FSTreePtr&    parent,
			                           const std::string&  name ) : FSTree( parent, name )
			{
			}
			
			const FSTree* WindowKey() const  { return ParentRef().get(); }
			
			mode_t FileTypeMode() const  { return S_IFCHR; }
			mode_t FilePermMode() const  { return S_IRUSR | S_IWUSR; }
			
			void Attach( const FSTreePtr& target ) const;
			
			boost::shared_ptr< IOHandle > Open( OpenFlags flags ) const;
	};
	
	void FSTree_sys_window_REF_tty::Attach( const FSTreePtr& target ) const
	{
		gWindowParametersMap[ WindowKey() ].itsTTYDelegate = target;
	}
	
	boost::shared_ptr< IOHandle >
	//
	FSTree_sys_window_REF_tty::Open( OpenFlags flags ) const
	{
		WindowParameters& params = gWindowParametersMap[ WindowKey() ];
		
		boost::shared_ptr< IOHandle > tty;
		
		const bool has_tty = params.itsTTYDelegate.get() != NULL;
		
		if ( has_tty )
		{
			tty = params.itsTTYDelegate->Open( flags );
		}
		
		std::string pathname = ( has_tty ? tty->GetFile().get() : this )->Pathname();
		
		boost::shared_ptr< IOHandle > terminal( new TerminalHandle( pathname ) );
		
		if ( has_tty )
		{
			tty->Attach( terminal );
			
			terminal->Attach( tty );
		}
		
		params.itsTerminal = terminal;
		
		return terminal;
	}
	
	
	namespace
	{
		
		boost::shared_ptr< Ped::View >& GetView( const FSTree* key )
		{
			return gWindowParametersMap[ key ].itsSubview;
		}
		
	}
	
	
	template < class Accessor >
	static FSTreePtr PropertyFactory( const FSTreePtr&    parent,
	                                  const std::string&  name )
	{
		typedef sys_window_REF_Property< Accessor > Property;
		
		return FSTreePtr( new FSTree_sys_window_REF_Property( parent,
		                                                      name,
		                                                      &Property::Read,
		                                                      &Property::Write ) );
	}
	
	const FSTree_Premapped::Mapping sys_window_REF_Mappings[] =
	{
		{ "ref",   &Basic_Factory< FSTree_sys_window_REF_ref >, true },
		
		{ "view",  &Basic_Factory< FSTree_X_view< GetView > >, true },
		
		{ "tty",   &Basic_Factory< FSTree_sys_window_REF_tty > },
		
		{ "title", &PropertyFactory< Access_Title   > },
		{ "pos",   &PropertyFactory< Access_Origin  > },
		{ "size",  &PropertyFactory< Access_Size    > },
		{ "vis",   &PropertyFactory< Access_Visible > },
		
		{ NULL, NULL }
	};
	
}

