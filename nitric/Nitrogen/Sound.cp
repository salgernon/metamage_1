// Sound.cp

// Nitrogen Carbon support
#ifndef NITROGEN_SOUND_H
#include "Nitrogen/Sound.h"
#endif
#ifndef NITROGEN_OSSTATUS_H
#include "Nitrogen/OSStatus.h"
#endif

namespace Nitrogen
{
	
	SoundManagerErrorsRegistrationDependency::SoundManagerErrorsRegistrationDependency()
	{
		// does nothing, but guarantees construction of theRegistration
	}
	
	
	static void RegisterSoundManagerErrors();
	
	
	class SoundManagerErrorsRegistration
	{
		public:
			SoundManagerErrorsRegistration()  { RegisterSoundManagerErrors(); }
	};
	
	static SoundManagerErrorsRegistration theRegistration;
	
	
	SPBGetIndexedDevice_Result SPBGetIndexedDevice( std::size_t count )
	{
		SPBGetIndexedDevice_Result  result;
		::Handle                    deviceIconHandle;
		
		ThrowOSStatus( ::SPBGetIndexedDevice( count,
		                                      result.deviceName,
		                                      &deviceIconHandle ) );
		
		result.deviceIconHandle = Nucleus::Owned< Handle >::Seize( deviceIconHandle );
		
		return result;
	}
	
	Nucleus::Owned< SoundInputRefNum > SPBOpenDevice( ConstStr255Param       deviceName,
	                                         SoundInputPermissions  permission )
	{
		long refNum;
		ThrowOSStatus( ::SPBOpenDevice( deviceName, permission, &refNum ) );
		
		return Nucleus::Owned< SoundInputRefNum >::Seize( SoundInputRefNum( refNum ) );
	}
	
	void SPBCloseDevice( Nucleus::Owned< SoundInputRefNum > refNum )
	{
		ThrowOSStatus( ::SPBCloseDevice( refNum.Release() ) );
	}
	
	void SPBGetDeviceInfo( SoundInputRefNum          refNum,
	                       SoundInputDeviceInfoType  infoType,
	                       void*                     infoData )
	{
		ThrowOSStatus( ::SPBGetDeviceInfo( refNum, infoType, infoData ) );
	}
	
	void SPBSetDeviceInfo( SoundInputRefNum          refNum,
	                       SoundInputDeviceInfoType  infoType,
	                       void*                     infoData )
	{
		ThrowOSStatus( ::SPBSetDeviceInfo( refNum, infoType, infoData ) );
	}
	
	void RegisterSoundManagerErrors()
	{
		RegisterOSStatus< noHardware                   >();
		RegisterOSStatus< notEnoughHardware            >();
		RegisterOSStatus< queueFull                    >();
		RegisterOSStatus< resProblem                   >();
		RegisterOSStatus< badChannel                   >();
		RegisterOSStatus< badFormat                    >();
		RegisterOSStatus< notEnoughBufferSpace         >();
		RegisterOSStatus< badFileFormat                >();
		RegisterOSStatus< channelBusy                  >();
		RegisterOSStatus< buffersTooSmall              >();
		RegisterOSStatus< channelNotBusy               >();
		RegisterOSStatus< noMoreRealTime               >();
		RegisterOSStatus< siVBRCompressionNotSupported >();
		RegisterOSStatus< siNoSoundInHardware          >();
		RegisterOSStatus< siBadSoundInDevice           >();
		RegisterOSStatus< siNoBufferSpecified          >();
		RegisterOSStatus< siInvalidCompression         >();
		RegisterOSStatus< siHardDriveTooSlow           >();
		RegisterOSStatus< siInvalidSampleRate          >();
		RegisterOSStatus< siInvalidSampleSize          >();
		RegisterOSStatus< siDeviceBusyErr              >();
		RegisterOSStatus< siBadDeviceName              >();
		RegisterOSStatus< siBadRefNum                  >();
		RegisterOSStatus< siInputDeviceErr             >();
		RegisterOSStatus< siUnknownInfoType            >();
		RegisterOSStatus< siUnknownQuality             >();
	}
	
}

