// MacWindows.h

#ifndef NITROGEN_MACWINDOWS_H
#define NITROGEN_MACWINDOWS_H

#ifndef __MACWINDOWS__
#include <MacWindows.h>
#endif
#ifndef NITROGEN_OWNED_H
#include "Nitrogen/Owned.h"
#endif
#ifndef NITROGEN_MACTYPES_H
#include "Nitrogen/MacTypes.h"
#endif
#ifndef NITROGEN_ALIASES_H
#include "Nitrogen/Aliases.h"
#endif
#ifndef NITROGEN_APPLEEVENTS_H
#include "Nitrogen/AppleEvents.h"
#endif
#ifndef NITROGEN_COLLECTIONS_H
#include "Nitrogen/Collections.h"
#endif
#ifndef NITROGEN_DRAG_H
#include "Nitrogen/Drag.h"
#endif
#ifndef NITROGEN_EVENTS_H
#include "Nitrogen/Events.h"
#endif
#ifndef NITROGEN_MENUS_H
#include "Nitrogen/Menus.h"
#endif
#ifndef NITROGEN_MIXEDMODE_H
#include "Nitrogen/MixedMode.h"
#endif
#ifndef NITROGEN_QDOFFSCREEN_H
#include "Nitrogen/QDOffscreen.h"
#endif
#ifndef NITROGEN_QUICKDRAW_H
#include "Nitrogen/Quickdraw.h"
#endif
#ifndef NITROGEN_TEXTCOMMON_H
#include "Nitrogen/TextCommon.h"
#endif
#ifndef NITROGEN_ICONS_H
#include "Nitrogen/Icons.h"
#endif
#ifndef NITROGEN_MACERRORS_H
#include "Nitrogen/MacErrors.h"
#endif
#ifndef NITROGEN_CFSTRING_H
#include "Nitrogen/CFString.h"
#endif

namespace Nitrogen
  {
   class PropertyCreator_Tag {};
   typedef SelectorType< PropertyCreator_Tag, ::PropertyCreator, '\?\?\?\?' > PropertyCreator;

   class PropertyTag_Tag {};      // Seems redundant, doesn't it?
   typedef SelectorType< PropertyTag_Tag, ::PropertyTag, '\?\?\?\?' > PropertyTag;

   class WindowClass_Tag {};
   typedef SelectorType< WindowClass_Tag, ::WindowClass, 0 > WindowClass;

   class WindowAttributes_Tag {};
   typedef SelectorType< WindowAttributes_Tag, ::WindowAttributes, kWindowNoAttributes > WindowAttributes;

   class WindowPositionMethod_Tag {};
   typedef SelectorType< WindowPositionMethod_Tag, ::WindowPositionMethod, 0 > WindowPositionMethod;

   class WindowRegionCode_Tag {};
   typedef SelectorType< WindowRegionCode_Tag, ::WindowRegionCode, 0 > WindowRegionCode;
  
   using ::GetWindowRegionRec;
   using ::GetWindowRegionPtr;
   using ::GetWindowRegionRecPtr;
   
   /* ... */
   
   using ::WindowRef;
   
   template <> struct Disposer< WindowRef >: public std::unary_function< WindowRef, void >
     {
      void operator()( WindowRef w ) const
        {
         ::DisposeWindow( w );
        }
     };

   /* ... */

   WindowAttributes GetWindowAttributes( WindowRef window );

   void ChangeWindowAttributes ( WindowRef        window,
                                 WindowAttributes setTheseAttributes,
                                 WindowAttributes clearTheseAttributes );
   
   /* ... */
   
   using ::ShowWindow;

   /* ... */
   
   void RegisterWindowManagerErrors();
  }

#endif
