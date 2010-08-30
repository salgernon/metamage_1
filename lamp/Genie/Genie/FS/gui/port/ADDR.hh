/*
	Genie/FS/gui/port/ADDR.hh
	-------------------------
*/

#ifndef GENIE_FS_GUI_PORT_ADDR_HH
#define GENIE_FS_GUI_PORT_ADDR_HH

// Genie
#include "Genie/FS/FSTree_Directory.hh"


namespace Pedestal
{
	
	class View;
	
}

namespace Genie
{
	
	extern const FSTree_Premapped::Mapping sys_port_ADDR_Mappings[];
	
	void notify_port_of_view_loss( const FSTree* port_key, const FSTree* view );
	
	void RemoveUserWindow( const FSTree* key );
	
	bool invalidate_port_WindowRef( const FSTree* key );
	
	void install_view_in_port    ( const boost::intrusive_ptr< Pedestal::View >& view, const FSTree* key );
	void uninstall_view_from_port( const boost::intrusive_ptr< Pedestal::View >& view, const FSTree* key );
	
	const FSTree* get_port_focus( const FSTree* port );
	
	void set_port_focus( const FSTree* port, const FSTree* focus );
	
}

#endif

