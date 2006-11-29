// Controls.h

#ifndef NITROGEN_CONTROLS_H
#define NITROGEN_CONTROLS_H

#ifndef NITROGEN_FRAMEWORKHEADER_H
#include "Nitrogen/FrameworkHeader.h"
#endif
#ifndef __CONTROLS__
#include FRAMEWORK_HEADER(HIToolbox,Controls.h)
#endif
#ifndef NITROGEN_MACWINDOWS_H
#include "Nitrogen/MacWindows.h"
#endif
#ifndef NITROGEN_AEDATAMODEL_H
#include "Nitrogen/AEDataModel.h"
#endif
#ifndef NITROGEN_MACTYPES_H
#include "Nitrogen/MacTypes.h"
#endif
#ifndef NUCLEUS_MAKE_H
#include "Nucleus/Make.h"
#endif

#ifdef DrawOneControl
   #undef DrawOneControl
   inline void DrawOneControl( ControlRef theControl )
     {
      Draw1Control( theControl );
     }
#endif

namespace Nitrogen
  {
   using ::ControlRef;
  }

namespace Nucleus
  {
   // Not actually used, since controls are owned by the parent window
   
   template <> struct Disposer< Nitrogen::ControlRef >: public std::unary_function< Nitrogen::ControlRef, void >
     {
      void operator()( Nitrogen::ControlRef c ) const
        {
         ::DisposeControl( c );
        }
     };
  }

namespace Nitrogen
  {
	class ControlPartCode_Tag {};
	typedef Nucleus::SelectorType< ControlPartCode_Tag, ::ControlPartCode, kControlNoPart > ControlPartCode;
	
	struct ControlActionUPP_Details : Basic_UPP_Details< ::ControlActionUPP,
	                                                     ::ControlActionProcPtr,
	                                                     ::NewControlActionUPP,
	                                                     ::DisposeControlActionUPP,
	                                                     ::InvokeControlActionUPP >
	{};
	
	typedef UPP< ControlActionUPP_Details > ControlActionUPP;
	
	typedef void ( *ControlActionProcPtr )( ControlRef control, ControlPartCode partCode );
	
	template < ControlActionProcPtr actionProc >
	struct Adapt_ControlAction
	{
		static pascal void ToCallback( ::ControlRef control, ::ControlPartCode partCode )
		{
			try
			{
				actionProc( control, ControlPartCode( partCode ) );
			}
			catch ( OSStatus err )
			{
			}
		}
	};
	
	inline Nucleus::Owned< ControlActionUPP > NewControlActionUPP( ::ControlActionProcPtr p )
	{
		return NewUPP< ControlActionUPP >( p );
	}

	inline void DisposeControlActionUPP( Nucleus::Owned< ControlActionUPP > )
	{
	}
	
	inline void InvokeControlActionUPP( ControlRef theControl, 
	                                    ControlPartCode partCode,
	                                    ControlActionUPP userUPP )
	{
		userUPP( theControl, partCode );
	}
	
	inline ControlPartCode ControlNoPart()         { return ControlPartCode::Make( kControlNoPart        ); }
	inline ControlPartCode ControlIndicatorPart()  { return ControlPartCode::Make( kControlIndicatorPart ); }
	inline ControlPartCode ControlDisabledPart()   { return ControlPartCode::Make( kControlDisabledPart  ); }
	inline ControlPartCode ControlInactivePart()   { return ControlPartCode::Make( kControlInactivePart  ); }
	
	inline ControlPartCode ControlEntireControl()  { return ControlPartCode::Make( kControlEntireControl ); }
	
	class ControlProcID_Tag {};
	typedef Nucleus::SelectorType< ControlProcID_Tag, ::SInt16, 0 > ControlProcID;
	
	// 972
	ControlRef NewControl(
		WindowRef owningWindow, 
		const Rect& boundsRect, 
		ConstStr255Param controlTitle, 
		bool initiallyVisible, 
		short initialValue, 
		short minimumValue, 
		short maximumValue, 
		ControlProcID procID, 
		RefCon refCon
	);
	
	// 1007
	using ::DisposeControl;
	inline void DisposeControl( Nucleus::Owned< ControlRef > )  {}
	
	// 1169
	void HiliteControl( ControlRef control, ControlPartCode hiliteState );
	
	// 1183, 1195
	using ::ShowControl;
	using ::HideControl;
	
	// 1336
   using ::DrawOneControl;

	// 1339
	inline void UpdateControls( WindowRef window, RgnHandle region )
	{
		::UpdateControls( window, region );
	}
	
	inline void UpdateControls( WindowRef window )
	{
		::UpdateControls( window, GetPortVisibleRegion( GetWindowPort( window ) ) );
	}
	
	// 1711
	ControlPartCode TrackControl( ControlRef theControl, Point startPoint, ControlActionUPP actionProc = NULL );
	
	template < typename ControlActionUPP::ProcPtr actionProc >
	ControlPartCode TrackControl( ControlRef theControl, Point startPoint )
	{
		return TrackControl( theControl, startPoint, StaticUPP< ControlActionUPP, actionProc >() );
	}
	
	template < ControlActionProcPtr actionProc >
	ControlPartCode TrackControl( ControlRef theControl, Point startPoint )
	{
		return TrackControl< Adapt_ControlAction< actionProc >::ToCallback >( theControl, startPoint );
	}
	
	struct FindControl_Result
	{
		ControlRef control;
		ControlPartCode part;
	};
	
	// 1757
	FindControl_Result FindControl( Point testPoint, WindowRef theWindow );
	
	// 1914, 1929
	using ::MoveControl;
	using ::SizeControl;
	
	// 1947
	using ::SetControlTitle;
	void SetControlTitle( ControlRef control, std::string title );
	
	// 2006, 2018, 2032, 2044, 2058, 2070
	using ::GetControlValue;
	using ::SetControlValue;
	using ::GetControlMinimum;
	using ::SetControlMinimum;
	using ::GetControlMaximum;
	using ::SetControlMaximum;
	
	// 2211
   using ::ControlID;
   
  }

namespace Nucleus
  {
   template <>
   struct Maker< Nitrogen::ControlID >
     {
      Nitrogen::ControlID operator()( Nitrogen::OSType signature, SInt32 id ) const
        {
         Nitrogen::ControlID result;
         result.signature = signature;
         result.id = id;
         return result;
        }
     };   
  }

namespace Nitrogen
  {
   
   // 2245
   ControlRef GetControlByID( WindowRef inWindow, const ControlID& id );
   ControlRef GetControlByID( WindowRef inWindow, OSType signature, SInt32 id );  // To be removed; use Make.

	// 2491
	void SetControlAction( ControlRef control, ControlActionUPP actionProc );
	
	template < typename ControlActionUPP::ProcPtr actionProc >
	void SetControlAction( ControlRef control )
	{
		SetControlAction( control, StaticUPP< ControlActionUPP, actionProc >() );
	}
	
	template < ControlActionProcPtr actionProc >
	void SetControlAction( ControlRef control )
	{
		SetControlAction< Adapt_ControlAction< actionProc >::ToCallback >( control );
	}
	
	// 2505
	inline ControlActionUPP GetControlAction( ControlRef control )  { return ::GetControlAction( control ); }
	
	// 2520
	void SetControlReference( ControlRef control, RefCon data );
	
	// 2534
	RefCon GetControlReference( ControlRef control );
	
   template < ::ResType inTagName > struct ControlData_Traits;

   struct ControlKeyFilterUPP_Details: Basic_UPP_Details< ::ControlKeyFilterUPP,
                                                          ::ControlKeyFilterProcPtr,
                                                          ::NewControlKeyFilterUPP,
                                                          ::DisposeControlKeyFilterUPP,
                                                          ::InvokeControlKeyFilterUPP >
     {};
   
   typedef UPP< ControlKeyFilterUPP_Details > ControlKeyFilterUPP;
   
   template<> struct ControlData_Traits< kControlFontStyleTag >: POD_DescType_Traits< ControlFontStyleRec > {};
   template<> struct ControlData_Traits< kControlKeyFilterTag >: POD_DescType_Traits< ControlKeyFilterUPP > {};
   template<> struct ControlData_Traits< kControlKindTag      >: POD_DescType_Traits< ControlKind         > {};
   template<> struct ControlData_Traits< kControlSizeTag      >: POD_DescType_Traits< ControlSize         > {};


   template < ::ResType inTagName >
   struct SetControlData_Traits: public ControlData_Traits<inTagName>
     {
      typedef typename ControlData_Traits<inTagName>::Parameter InData_Type;
     };

   template < ::ResType inTagName >
   struct GetControlData_Traits: public ControlData_Traits<inTagName>
     {
     };
   
   // 2835
   void SetControlData( ControlRef        inControl,
                        ControlPartCode   inPart,
                        ResType           inTagName,
                        Size              inSize,
                        const void *      inData );

	class SetControlData_Putter
	{
		private:
			ControlRef       myControl;
			ControlPartCode  myPart;
			ResType          myTagName;
		
		public:
			SetControlData_Putter( ControlRef       control,
			                       ControlPartCode  part,
			                       ResType          tagName ) : myControl( control ),
			                                                    myPart   ( part    ),
			                                                    myTagName( tagName )  {}
			
			void operator()( const void *begin, const void *end ) const
			{
				Nitrogen::SetControlData( myControl,
				                          myPart,
				                          myTagName,
				                          Detail::Distance( begin, end ),
				                          begin );
			}
	};
	
   template < ::ResType inTagName >
   void SetControlData( ControlRef                                               inControl,
                        ControlPartCode                                          inPart,
                        typename SetControlData_Traits< inTagName >::InData_Type inData )
     {
      typedef SetControlData_Traits< inTagName > Traits;
      
      Traits().Put( inData,
		            SetControlData_Putter( inControl,
		                                   inPart,
		                                   inTagName ) );
     }

   template < ::ResType inTagName >
   void SetControlData( ControlRef                                               inControl,
                        typename SetControlData_Traits< inTagName >::InData_Type inData )
     {
      return SetControlData< inTagName >( inControl, ControlEntireControl(), inData );
     }

	// 2852
   Size GetControlData( ControlRef        inControl,
                        ControlPartCode   inPart,
                        ResType           inTagName,
                        Size              inBufferSize,
                        void *            inBuffer );

   Size GetControlData( ControlRef        inControl,
                        ControlPartCode   inPart,
                        ResType           inTagName );
   
	template < ::ResType tagName >
	class GetControlData_Getter
	{
		private:
			ControlRef myControl;
			ControlPartCode myPart;
		
		public:
			GetControlData_Getter( ControlRef       control,
			                       ControlPartCode  part ) : myControl( control ),
			                                                 myPart   ( part    )  {}
			
			std::size_t size() const
			{
				if ( GetControlData_Traits< tagName >::hasStaticSize )
				{
					return sizeof (typename GetControlData_Traits< tagName >::Buffer);
				}
				
				return GetControlData( myControl, myPart, tagName );
			}
			
			void operator()( void *begin, void *end ) const
			{
				GetControlData( myControl,
				                myPart,
				                tagName,
				                Detail::Distance( begin, end ),
				                begin );
			}
	};
	
   template < ::ResType inTagName >
   typename GetControlData_Traits<inTagName>::Result
   GetControlData( ControlRef        inControl,
                   ControlPartCode   inPart = ControlEntireControl() )
     {
      typedef GetControlData_Traits< inTagName > Traits;
      
      return Traits().Get( GetControlData_Getter< inTagName >( inControl, inPart ) );
     }
   
   /* ... */
   
   // 2918
   bool HandleControlDragTracking( ControlRef          inControl,
                                   DragTrackingMessage inMessage,
                                   DragReference       inDrag );

   void HandleControlDragReceive( ControlRef    inControl,
                                  DragReference inDrag );

   void SetControlDragTrackingEnabled( ControlRef theControl,
                                       bool       tracks );

   bool IsControlDragTrackingEnabled( ControlRef theControl );

   void SetAutomaticControlDragTrackingEnabledForWindow( WindowRef theWindow,
                                                         bool      tracks );

	// 3140
   bool IsAutomaticControlDragTrackingEnabledForWindow( WindowRef theWindow );

	// 3395
	Rect GetControlBounds( ControlRef control );
	
	// 3497
	void SetControlBounds( ControlRef control, const Rect& bounds );
	
   /* ... */
   
   void RegisterControlManagerErrors();
  }

#endif
