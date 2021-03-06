// Nitrogen/Controls.cc
// --------------------
//
// Maintained by Joshua Juran

// Part of the Nitrogen project.
//
// Written 2002-2007 by Lisa Lippincott, Marshall Clow, and Joshua Juran.
//
// This code was written entirely by the above contributors, who place it
// in the public domain.


#include "Nitrogen/Controls.hh"

// Nitrogen
#include "Nitrogen/MacWindows.hh"
#include "Nitrogen/OSStatus.hh"
#include "Nitrogen/Quickdraw.hh"


namespace Nitrogen
{
	
	using Mac::ThrowOSStatus;
	
	
	// does nothing, but guarantees construction of theRegistration
	NUCLEUS_DEFINE_ERRORS_DEPENDENCY( ControlManager )
	
	
	static void RegisterControlManagerErrors();
	
	
#if NUCLEUS_RICH_ERRORCODES
#pragma force_active on
	
	class ControlManagerErrorsRegistration
	{
		public:
			ControlManagerErrorsRegistration()  { RegisterControlManagerErrors(); }
	};
	
	static ControlManagerErrorsRegistration theRegistration;
	
#pragma force_active reset
#endif
	
	
	ControlRef NewControl( WindowRef         owningWindow,
	                       const Rect&       boundsRect, 
	                       ConstStr255Param  controlTitle,
	                       bool              initiallyVisible,
	                       short             initialValue,
	                       short             minimumValue,
	                       short             maximumValue,
	                       ControlProcID     procID,
	                       long              refCon )
	{
		ControlRef control = ::NewControl( owningWindow,
		                                   &boundsRect,
		                                   controlTitle,
		                                   initiallyVisible,
		                                   initialValue,
		                                   minimumValue,
		                                   maximumValue,
		                                   procID,
		                                   refCon );
		
		if ( control == NULL )
		{
			// Assume out of memory
			
			ThrowOSStatus( memFullErr );
		}
		
		return control;
	}
	
#if TARGET_API_MAC_OSX
// Works in 10.1 and later
#define GetPortVisibleRegion_or_NULL( port )  (NULL)
#else
#define GetPortVisibleRegion_or_NULL( port )  GetPortVisibleRegion( port )
#endif
	
	void UpdateControls( WindowRef window )
	{
		::UpdateControls( window,
		                  GetPortVisibleRegion_or_NULL( GetWindowPort( window ) ) );
	}
	
	FindControl_Result FindControl( Point testPoint, WindowRef theWindow )
	{
		FindControl_Result result;
		result.part = ControlPartCode( ::FindControl( testPoint,
		                                              theWindow,
		                                              &result.control ) );
		
		return result;
	}
	
   ControlRef GetControlByID( WindowRef inWindow, const ControlID& id )
     {
      ControlRef result;
      ThrowOSStatus( ::GetControlByID( inWindow, &id, &result ) );
      return result;
     }

   ControlRef GetControlByID( WindowRef inWindow, Mac::OSType signature, SInt32 id )
     {
      ControlID controlID;
      controlID.signature = signature;
      controlID.id = id;
      return GetControlByID( inWindow, controlID );
     }

   void SetControlData( ControlRef        inControl,
                        ControlPartCode   inPart,
                        ControlDataTag    inTagName,
                        std::size_t       inSize,
                        const void *      inData )
     {
      ThrowOSStatus( ::SetControlData( inControl,
                                       inPart,
                                       inTagName,
                                       static_cast< ::Size >( inSize ),
                                       inData ) );
     }

   std::size_t GetControlData( ControlRef        inControl,
                        ControlPartCode   inPart,
                        ControlDataTag    inTagName,
                        std::size_t       inBufferSize,
                        void *            inBuffer )
     {
      ::Size outActualSize;
      ThrowOSStatus( ::GetControlData( inControl,
                                       inPart,
                                       inTagName,
                                       static_cast< ::Size >( inBufferSize ),
                                       inBuffer,
                                       &outActualSize ) );
      return static_cast< std::size_t >( outActualSize );
     }

   bool HandleControlDragTracking( ControlRef          inControl,
                                   DragTrackingMessage inMessage,
                                   DragReference       inDrag )
     {
      ::Boolean outLikesDrag;
      ThrowOSStatus( ::HandleControlDragTracking( inControl, inMessage, inDrag, &outLikesDrag ) );
      return outLikesDrag;
     }

   void HandleControlDragReceive( ControlRef    inControl,
                                  DragReference inDrag )
     {
      ThrowOSStatus( ::HandleControlDragReceive( inControl, inDrag ) );
     }

   void SetControlDragTrackingEnabled( ControlRef theControl,
                                       bool       tracks )
     {
      ThrowOSStatus( ::SetControlDragTrackingEnabled( theControl, tracks ) );
     }

   bool IsControlDragTrackingEnabled( ControlRef theControl )
     {
      ::Boolean tracks;
      ThrowOSStatus( ::IsControlDragTrackingEnabled( theControl, &tracks ) );
      return tracks;
     }
   
   void SetAutomaticControlDragTrackingEnabledForWindow( WindowRef theWindow,
                                                         bool      tracks )
     {
      ThrowOSStatus( ::SetAutomaticControlDragTrackingEnabledForWindow( theWindow, tracks ) );
     }
  
   bool IsAutomaticControlDragTrackingEnabledForWindow( WindowRef theWindow )
     {
      ::Boolean tracks;
      ThrowOSStatus( ::IsAutomaticControlDragTrackingEnabledForWindow( theWindow, &tracks ) );
      return tracks;
     }

   void RegisterControlManagerErrors()
     {
      RegisterOSStatus< memFullErr                   >();
      RegisterOSStatus< errMessageNotSupported       >();
      RegisterOSStatus< errDataNotSupported          >();
      RegisterOSStatus< errControlDoesntSupportFocus >();
      RegisterOSStatus< errUnknownControl            >();
      RegisterOSStatus< errCouldntSetFocus           >();
      RegisterOSStatus< errNoRootControl             >();
      RegisterOSStatus< errRootAlreadyExists         >();
      RegisterOSStatus< errInvalidPartCode           >();
      RegisterOSStatus< errControlsAlreadyExist      >();
      RegisterOSStatus< errControlIsNotEmbedder      >();
      RegisterOSStatus< errDataSizeMismatch          >();
      RegisterOSStatus< errControlHiddenOrDisabled   >();
      RegisterOSStatus< errCantEmbedIntoSelf         >();
      RegisterOSStatus< errCantEmbedRoot             >();
      RegisterOSStatus< errItemNotControl            >();
      RegisterOSStatus< controlInvalidDataVersionErr >();
      RegisterOSStatus< controlPropertyInvalid       >();
      RegisterOSStatus< controlPropertyNotFoundErr   >();
      RegisterOSStatus< controlHandleInvalidErr      >();
     }
  }
