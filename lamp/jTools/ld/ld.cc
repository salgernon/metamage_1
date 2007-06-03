/*	=====
 *	ld.cc
 *	=====
 */

// Standard C++
#include <vector>

// Standard C
#include <stdlib.h>

// POSIX
#include <sys/wait.h>

// POSeven
#include "POSeven/Pathnames.hh"
#include "POSeven/Stat.hh"

// Nitrogen
#include "Nitrogen/MacErrors.h"
#include "Nitrogen/Str.h"

// GetPathname
#include "GetPathname.hh"

// Divergence
#include "Divergence/Utilities.hh"

// Orion
#include "Orion/GetOptions.hh"
#include "Orion/Main.hh"
#include "Orion/StandardIO.hh"


namespace N = Nitrogen;
namespace NN = Nucleus;
namespace O = Orion;


static int exit_from_wait( int stat )
{
	int result = WIFEXITED( stat )   ? WEXITSTATUS( stat )
	           : WIFSIGNALED( stat ) ? WTERMSIG( stat ) + 128
	           :                       -1;
	
	return result;
}


namespace jTools
{
	
	namespace Div = Divergence;
	
	using namespace io::path_descent_operators;
	
	
	static bool m68k = TARGET_CPU_68K;
	static bool ppc  = TARGET_CPU_PPC;
	
	static bool debug = true;
	
	static bool carbon = false;
	
	static std::string outputFilename;
	
	
	static std::string CommandFromArch( const std::string& arch )
	{
		if ( arch == "m68k" )
		{
			m68k = true;
			
			return "MWLink68K -model far";
		}
		else if ( arch == "ppc" )
		{
			ppc = true;
			
			return "MWLinkPPC";
		}
		
		return "MWLinkUnsupportedArchitecture";
	}
	
	static const char* TranslateCodeGenFlag( const std::string& flag )
	{
		if ( flag == "-fpascal-strings" )
		{
			return "";
		}
		else if ( flag == "-fno-rtti" )
		{
			return "-RTTI off";
		}
		else if ( flag == "-mCFM"  &&  m68k )
		{
			return "-model CFMflatdf";
		}
		else if ( flag == "-mA4-globals" )
		{
			return "-a4";
		}
		else if ( flag == "-ftraceback" )
		{
			return "-tb on";
		}
		
		return "";
	}
	
	static std::string MacPathFromPOSIXPath( const char* pathname )
	{
		FSSpec item = Div::ResolvePathToFSSpec( pathname );
		
		return GetMacPathname( item );
	}
	
	static std::string QuotedMacPathFromPOSIXPath( const char* pathname )
	{
		return "'" + MacPathFromPOSIXPath( pathname ) + "'";
	}
	
	static std::string OutputFile( const char* pathname )
	{
		return "-o " + QuotedMacPathFromPOSIXPath( pathname );
	}
	
	std::vector< const char* > gIncludeDirs;
	
	static void RememberIncludeDir( const char* pathname )
	{
		gIncludeDirs.push_back( pathname );
	}
	
	enum ProductType
	{
		kProductUnknown,
		kProductStaticLib,
		kProductSharedLib,
		kProductTool,
		kProductApp
	};
	
	static ProductType gProductType = kProductUnknown;
	
	static std::string ProductOptionsForTool( bool cfm )
	{
		std::string build = cfm ? "-xm s -init InitializeFragment -term TerminateFragment -export pragma -sizemin 4096 -sizemax 8192"
		                        : "-xm c -rsrcfar -rsrcflags system -rt Wish=0";
		
		return build + " -t Wish -c Poof";
	}
	
	static int Main( int argc, const char *const argv[] )
	{
		NN::RegisterExceptionConversion< NN::Exception, N::OSStatus >();
		
		std::string command = TARGET_CPU_68K ? "MWLink68K -model far" : "MWLinkPPC";
		std::string product = "-xm l";
		std::string ldArgs;
		std::string translatedPath;
		
		bool verbose = false;
		
		while ( const char* arg = *++argv )
		{
			if ( arg[0] == '-' )
			{
				switch ( arg[1] )
				{
					case 'v':
						verbose = true;
						continue;
					
					case 'a':
						if ( std::strcmp( arg + 1, "arch" ) == 0 )
						{
							command = CommandFromArch( *++argv );
							continue;
						}
						break;
					
					case 'd':
						if ( std::strcmp( arg + 1, "dynamic" ) == 0 )
						{
							gProductType = kProductSharedLib;
							product = "-xm s -init __initialize -term __terminate -export pragma";
							continue;
						}
						break;
					
					case 'e':
						if ( std::strcmp( arg + 1, "execute" ) == 0 )
						{
							gProductType = kProductTool;
							product = ProductOptionsForTool( ppc );
							continue;
						}
						break;
					
					case 'b':
						if ( std::strcmp( arg + 1, "bundle" ) == 0 )
						{
							gProductType = kProductApp;
							product = "-xm a -sizemin 4096 -sizemax 8192";
							continue;
						}
						break;
					
					case 'm':
						if ( std::strcmp( arg + 1, "model" ) == 0 )
						{
							break;
						}
						// fall through
					case 'f':
						arg = TranslateCodeGenFlag( arg );
						break;
					
					case 's':
						if ( arg[2] == '\0' )
						{
							debug = false;
							continue;
						}
						
						if ( std::strcmp( arg + 1, "static" ) == 0 )
						{
							gProductType = kProductStaticLib;
							continue;
						}
						
						break;
					
					case 'o':
						if ( arg[2] == '\0' )
						{
							translatedPath = OutputFile( *++argv );
							
							outputFilename = io::get_filename( *argv );
							
							arg = translatedPath.c_str();
							break;
						}
						
						if ( std::strcmp( arg + 1, "object" ) == 0 )
						{
							product = "-xm c -rsrcfar -rsrcflags system";
							continue;
						}
						
						break;
					
					default:
						break;
				}
			}
			else if ( std::strchr( arg, '/' ) )
			{
				if ( io::get_filename( arg ) == "CarbonLib" )
				{
					carbon = true;
				}
				
				// translate path
				translatedPath = QuotedMacPathFromPOSIXPath( arg );
				
				arg = translatedPath.c_str();
			}
			
			if ( arg[0] != '\0' )
			{
				ldArgs += ' ';
				ldArgs += arg;
			}
		}
		
		std::string debugging;
		
		if ( debug )
		{
			debugging = " -sym full";
			
			if ( ppc )
			{
				debugging += " -tb on";
			}
		}
		
		std::string deadstripping;
		
		if ( gProductType == kProductApp )
		{
			deadstripping = m68k ? " -dead code" : " -dead off";
		}
		
		std::string fragmentName;
		
		if ( gProductType == kProductTool )
		{
			fragmentName = " -name '" + outputFilename + " " + ( carbon ? "Carbon" : "(Blue)" ) + "'";
		}
		
		ldArgs = " " + product + debugging + deadstripping + fragmentName + " " + ldArgs;
		
		std::string output = "tlsrvr --escape -- " + command + ldArgs + '\n';
		
		if ( verbose )
		{
			write( STDOUT_FILENO, output.data(), output.size() );
		}
		
		int wait_status = system( output.c_str() );
		
		return exit_from_wait( wait_status );
	}
	
}

int O::Main( int argc, const char *const argv[] )
{
	return jTools::Main( argc, argv );
}

