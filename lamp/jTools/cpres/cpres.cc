/*	========
 *	cpres.cc
 *	========
 */

// Standard C
#include <stdlib.h>

// Standard C/C++
#include <cstdio>

// Standard C++
#include <algorithm>

// command
#include "command/get_option.hh"

// plus
#include "plus/string.hh"

// nucleus
#include "nucleus/saved.hh"

// Nitrogen
#include "Mac/Toolbox/Types/OSStatus.hh"
#include "Mac/Toolbox/Utilities/ThrowOSStatus.hh"

#include "Nitrogen/Resources.hh"

// Io: MacFiles
#include "MacFiles/Classic.hh"

// Divergence
#include "Divergence/Utilities.hh"

// Orion
#include "Orion/Main.hh"


namespace n = nucleus;
namespace N = Nitrogen;
namespace Div = Divergence;


enum
{
	Option_last_byte = 255,
	
	Option_data_fork,
};

static command::option options[] =
{
	{ "data",      Option_data_fork },
	{ "data-fork", Option_data_fork },
	{ NULL }
};

static bool globally_using_data_fork = false;


static char* const* get_options( char* const* argv )
{
	++argv;  // skip arg 0
	
	short opt;
	
	while ( (opt = command::get_option( &argv, options )) )
	{
		switch ( opt )
		{
			case Option_data_fork:
				globally_using_data_fork = true;
				break;
			
			default:
				abort();
		}
	}
	
	return argv;
}


namespace tool
{
	
	static n::owned< N::ResFileRefNum > open_res_file_from_data_fork( const FSSpec&   filespec,
	                                                                  N::FSIOPermssn  perm )
	{
		FSRef fileref = N::FSpMakeFSRef( filespec );
		
		return N::FSOpenResourceFile( fileref, perm );
	}
	
	static n::owned< N::ResFileRefNum > open_res_file( const FSSpec&   filespec,
	                                                   N::FSIOPermssn  perm )
	{
		try
		{
			if ( TARGET_API_MAC_CARBON && globally_using_data_fork )
			{
				return open_res_file_from_data_fork( filespec, perm );
			}
		}
		catch ( ... )
		{
		}
		
		return N::FSpOpenResFile( filespec, perm );
	}
	
	
	static int TryResCopy( const char*       source_path,
	                       const FSSpec&     source,
	                       N::ResFileRefNum  destRes )
	{
		if ( io::directory_exists( source ) )
		{
			// Source item is a directory.
			std::fprintf( stderr, "cpres: %s: omitting directory\n", source_path );
			
			return 1;
		}
		
		n::owned< N::ResFileRefNum > sourceRes( open_res_file( source, N::fsRdPerm ) );
		
		int types = N::Count1Types();
		
		for ( int iType = 1;  iType <= types;  ++iType )
		{
			N::ResType type = N::Get1IndType( iType );
			int rsrcs = N::Count1Resources( type );
			
			for ( int iRsrc = 1;  iRsrc <= rsrcs;  ++iRsrc )
			{
				N::Handle h = N::Get1IndResource( type, iRsrc );
				
				::HNoPurge( h );
				
				n::saved< N::ResFile > savedResFile( destRes );
				
				N::GetResInfo_Result resInfo = N::GetResInfo( h );
				
				Handle existing = ::Get1Resource( resInfo.type, resInfo.id );
				
				if ( existing )
				{
					::RemoveResource( existing );
				}
				else
				{
					OSErr err = ResError();
					
					if ( err != resNotFound )
					{
						Mac::ThrowOSStatus( err );
					}
					
					// Okay, resource didn't exist in dest file
				}
				
				N::AddResource( N::DetachResource( h ), resInfo );
			}
		}
		
		return 0;
	}
	
	
	int Main( int argc, char** argv )
	{
		char *const *args = get_options( argv );
		
		const int argn = argc - (args - argv);
		
		if ( globally_using_data_fork  &&  ( !TARGET_API_MAC_CARBON || ::FSOpenResourceFile == NULL ) )
		{
			std::fprintf( stderr, "cpres: FSOpenResourceFile() unavailable for data fork\n" );
			
			return 2;
		}
		
		int fail = 0;
		
		// Check for sufficient number of args
		if ( argn < 2 )
		{
			std::fprintf( stderr, "cpres: missing %s\n", (argn == 0) ? "file arguments"
			                                                         : "destination file" );
			
			return 1;
		}
		
		// Last arg should be the destination file.
		const char* dest_path = args[ argn - 1 ];
		
		FSSpec dest;
		
		try
		{
			dest = Div::ResolvePathToFSSpec( dest_path );
		}
		catch ( ... )
		{
			std::fprintf( stderr, "cpres: last argument (%s) is not a file.\n", dest_path );
			
			return 1;
		}
		
		if ( TARGET_API_MAC_CARBON && globally_using_data_fork )
		{
			FSSpec parent_spec = N::FSMakeFSSpec( io::get_preceding_directory( dest ) );
			
			FSRef parent_ref = N::FSpMakeFSRef( parent_spec );
			
			
			HFSUniStr255 name = { dest.name[ 0 ] };
			
			std::copy( dest.name + 1,
			           dest.name + 1 + dest.name[ 0 ],
			           name.unicode );
			
			Mac::ThrowOSStatus( ::FSCreateResourceFile( &parent_ref,
			                                            name.length,
			                                            name.unicode,
			                                            FSCatalogInfoBitmap(),
			                                            NULL,
			                                            0,
			                                            NULL,
			                                            NULL,
			                                            NULL ) );
		}
		else
		{
			::FSpCreateResFile( &dest, 'RSED', 'rsrc', smRoman );
		}
		
		n::owned< N::ResFileRefNum > resFileH( open_res_file( dest, N::fsRdWrPerm ) );
		
		// Try to copy each file.  Return whether any errors occurred.
		for ( int index = 0;  index < argn - 1;  ++index )
		{
			const char* source_path = args[ index ];
			
			try
			{
				FSSpec source = Div::ResolvePathToFSSpec( source_path );
				
				fail += TryResCopy( source_path, source, resFileH );
			}
			catch ( const Mac::OSStatus& err )
			{
				++fail;
				
				const char* slash = strrchr( dest_path, '/' );
				const char* dest_name = slash ? slash + 1 : dest_path;
				
				std::fprintf( stderr, "OSStatus %ld copying from %s to %s.\n",
				                                err.Get(),       source_path,
				                                                       dest_name );
			}
		}
		
		return fail;
	}
	
}
