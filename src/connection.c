#include <stdlib.h>
#include <lua.h>
#include <lauxlib.h>

#include <dbus/dbus.h>

#include "ldbus.h"
#include "message.h"
#include "pending_call.h"

#include "connection.h"

static const char *const DispatchStatus_lst [] = {
	"data_remains" , 	//DBUS_DISPATCH_DATA_REMAINS	== 0
	"complete" ,		//DBUS_DISPATCH_COMPLETE	== 1
	"need_memory" ,		//DBUS_DISPATCH_NEED_MEMORY	== 2
	NULL
};

static int ldbus_connection_open ( lua_State *L ) {
	const char * address = luaL_checkstring ( L , 1 );
	
	DBusConnection * connection;
	DBusError error;
	dbus_error_init ( &error );
	
	connection = dbus_connection_open ( address , &error );
	
	if ( dbus_error_is_set ( &error ) ) {
		lua_pushboolean ( L , FALSE );
		lua_pushstring ( L , error.message );
		dbus_error_free ( &error );
		return 2;
	} else {
		push_DBusConnection ( L , connection );
		return 1;
	}
}

static int ldbus_connection_unref ( lua_State *L ) {
	DBusConnection * connection = *(void **)luaL_checkudata ( L , 1 , "ldbus_DBusConnection" );
	
	dbus_connection_unref ( connection );
	
	return 0;
}

static int ldbus_connection_get_is_connected ( lua_State *L ) {
	DBusConnection * connection = *(void **)luaL_checkudata ( L , 1 , "ldbus_DBusConnection" );
	
	lua_pushboolean ( L , dbus_connection_get_is_connected ( connection ) );
	
	return 1;
}

static int ldbus_connection_get_is_authenticated ( lua_State *L ) {
	DBusConnection * connection = *(void **)luaL_checkudata ( L , 1 , "ldbus_DBusConnection" );
	
	lua_pushboolean ( L , dbus_connection_get_is_authenticated ( connection ) );
	
	return 1;
}

static int ldbus_connection_get_is_anonymous ( lua_State *L ) {
	DBusConnection * connection = *(void **)luaL_checkudata ( L , 1 , "ldbus_DBusConnection" );
	
	lua_pushboolean ( L , dbus_connection_get_is_anonymous ( connection ) );
	
	return 1;
}

static int ldbus_connection_get_server_id ( lua_State *L ) {
	DBusConnection * connection = *(void **)luaL_checkudata ( L , 1 , "ldbus_DBusConnection" );
	
	char * result = dbus_connection_get_server_id ( connection );
	if ( result == NULL ) {
		lua_pushboolean ( L , FALSE );
	} else {
		lua_pushstring ( L , result );
	}
	return 1;
}

static int ldbus_connection_send ( lua_State *L ) {
	DBusConnection * connection = *(void **)luaL_checkudata ( L , 1 , "ldbus_DBusConnection" );
	DBusMessage * message = *(void **)luaL_checkudata ( L , 2 , "ldbus_DBusMessage" );
	
	unsigned int serial;
	
	lua_pushboolean ( L , dbus_connection_send ( connection , message , &serial ) );
	lua_pushinteger ( L , serial );
	
	return 2;
}

static int ldbus_connection_send_with_reply(lua_State *L) {
    DBusConnection * connection = *(void **)luaL_checkudata(L, 1, "ldbus_DBusConnection");
    DBusMessage * message = *(void **)luaL_checkudata(L, 2, "ldbus_DBusMessage");
    int timeout_milliseconds = luaL_optint(L, 3, -1);

	DBusPendingCall* pending;
	if (!dbus_connection_send_with_reply(connection, message, &pending, timeout_milliseconds)) {
		return luaL_error(L, LDBUS_NO_MEMORY);
	}

	push_DBusPendingCall(L, pending);
	return 1;
}

static int ldbus_connection_send_with_reply_and_block ( lua_State *L ) {
	DBusConnection * connection = *(void **)luaL_checkudata ( L , 1 , "ldbus_DBusConnection" );
	DBusMessage * message = *(void **)luaL_checkudata ( L , 2 , "ldbus_DBusMessage" );
	int timeout_milliseconds = luaL_optint ( L , 3 , -1 );
	
	DBusError error;
	dbus_error_init ( &error );
	
	DBusMessage * reply = dbus_connection_send_with_reply_and_block ( connection , message , timeout_milliseconds , &error );
	
	if ( dbus_error_is_set ( &error ) ) {
		lua_pushboolean ( L , FALSE );
		lua_pushstring ( L , error.message );
		dbus_error_free ( &error );
		return 2;
	} else {
		push_DBusMessage ( L , reply );
		return 1;
	}
}

static int ldbus_connection_flush ( lua_State *L ) {
	DBusConnection * connection = *(void **)luaL_checkudata ( L , 1 , "ldbus_DBusConnection" );
	
	dbus_connection_flush ( connection );
	
	return 0;
}

static int ldbus_connection_read_write_dispatch ( lua_State *L ) {
	DBusConnection * connection = *(void **)luaL_checkudata ( L , 1 , "ldbus_DBusConnection" );
	int timeout_milliseconds = luaL_optint ( L , 2 , -1 );
	
	lua_pushboolean ( L , dbus_connection_read_write_dispatch ( connection , timeout_milliseconds ) );
	
	return 1;
}

static int ldbus_connection_read_write ( lua_State *L ) {
	DBusConnection * connection = *(void **)luaL_checkudata ( L , 1 , "ldbus_DBusConnection" );
	int timeout_milliseconds = luaL_optint ( L , 2 , -1 );
	
	lua_pushboolean ( L , dbus_connection_read_write ( connection , timeout_milliseconds ) );
	
	return 1;
}

static int ldbus_connection_pop_message ( lua_State *L ) {
	DBusConnection * connection = *(void **)luaL_checkudata ( L , 1 , "ldbus_DBusConnection" );
	
	DBusMessage * message = dbus_connection_pop_message ( connection );
	
	if ( message == NULL ) {
		lua_pushboolean ( L , FALSE );
	} else {
		push_DBusMessage ( L , message );
	}
	return 1;
}

static int ldbus_connection_get_dispatch_status ( lua_State *L ) {
	DBusConnection * connection = *(void **)luaL_checkudata ( L , 1 , "ldbus_DBusConnection" );
	
	lua_pushstring ( L , DispatchStatus_lst [ dbus_connection_get_dispatch_status ( connection ) ] );
	return 1;
}

static int ldbus_connection_dispatch ( lua_State *L ) {
	DBusConnection * connection = *(void **)luaL_checkudata ( L , 1 , "ldbus_DBusConnection" );
	
	lua_pushstring ( L , DispatchStatus_lst [ dbus_connection_dispatch ( connection ) ] );
	return 1;
}

static int ldbus_connection_set_max_message_size ( lua_State *L ) {
	DBusConnection * connection = *(void **)luaL_checkudata ( L , 1 , "ldbus_DBusConnection" );
	long size = luaL_checklong ( L , 2 );
	
	dbus_connection_set_max_message_size ( connection , size );
	
	return 0;
}

static int ldbus_connection_get_max_message_size ( lua_State *L ) {
	DBusConnection * connection = *(void **)luaL_checkudata ( L , 1 , "ldbus_DBusConnection" );
	
	lua_pushinteger ( L , dbus_connection_get_max_message_size ( connection ) );
	
	return 1;
}

static int ldbus_connection_set_max_received_size ( lua_State *L ) {
	DBusConnection * connection = *(void **)luaL_checkudata ( L , 1 , "ldbus_DBusConnection" );
	long size = luaL_checklong ( L , 2 );
	
	dbus_connection_set_max_received_size ( connection , size );
	
	return 0;
}

static int ldbus_connection_get_max_received_size ( lua_State *L ) {
	DBusConnection * connection = *(void **)luaL_checkudata ( L , 1 , "ldbus_DBusConnection" );
	
	lua_pushinteger ( L , dbus_connection_get_max_received_size ( connection ) );
	
	return 1;
}

static int ldbus_connection_get_outgoing_size ( lua_State *L ) {
	DBusConnection * connection = *(void **)luaL_checkudata ( L , 1 , "ldbus_DBusConnection" );
	
	lua_pushinteger ( L , dbus_connection_get_outgoing_size ( connection ) );
	
	return 1;
}

static int ldbus_connection_has_messages_to_send ( lua_State *L ) {
	DBusConnection * connection = *(void **)luaL_checkudata ( L , 1 , "ldbus_DBusConnection" );
	
	lua_pushboolean ( L , dbus_connection_has_messages_to_send ( connection ) );
	
	return 1;
}

void push_DBusConnection ( lua_State *L , DBusConnection * connection ) {
	DBusConnection ** udata = lua_newuserdata ( L , sizeof ( DBusConnection * ) );
	*udata = connection;
	
	if ( luaL_newmetatable ( L , "ldbus_DBusConnection" ) ) {
		lua_newtable ( L );
		static luaL_Reg const methods [] = {
			{ "get_is_connected" , 			ldbus_connection_get_is_connected },
			{ "get_is_authenticated" , 		ldbus_connection_get_is_authenticated },
			{ "get_is_anonymous" , 			ldbus_connection_get_is_anonymous },
			{ "get_server_id" , 			ldbus_connection_get_server_id },
			{ "send" , 				ldbus_connection_send },
			{ "send_with_reply" , 			ldbus_connection_send_with_reply },
			{ "send_with_reply_and_block" , 	ldbus_connection_send_with_reply_and_block },
			{ "flush" ,				ldbus_connection_flush },
			{ "read_write_dispatch" ,		ldbus_connection_read_write_dispatch},
			{ "read_write" , 			ldbus_connection_read_write },
			{ "pop_message" ,			ldbus_connection_pop_message },
			{ "get_dispatch_status" , 		ldbus_connection_get_dispatch_status },
			{ "dispatch" , 				ldbus_connection_dispatch },
			{ "set_max_message_size" , 		ldbus_connection_set_max_message_size },
			{ "get_max_message_size" , 		ldbus_connection_get_max_message_size },
			{ "set_max_received_size" , 		ldbus_connection_set_max_received_size },
			{ "get_max_received_size" , 		ldbus_connection_get_max_received_size },
			{ "get_outgoing_size" , 		ldbus_connection_get_outgoing_size },
			{ "has_messages_to_send" , 		ldbus_connection_has_messages_to_send },
			{ NULL , NULL }
		};
		luaL_register ( L , NULL , methods );
                lua_setfield ( L , -2 , "__index" );
		
		lua_pushcfunction ( L , ldbus_connection_unref ) ;
                lua_setfield ( L , -2 , "__gc" );
		
		lua_pushcfunction ( L , tostring );
                lua_setfield ( L , -2 , "__tostring" );
		
		lua_pushstring ( L , "DBusConnection" );
                lua_setfield ( L , -2 , "__udtype" );
	}
	lua_setmetatable ( L , -2 );
}

void load_dbus_connection ( lua_State *L ) {
	lua_newtable ( L );
	static const struct luaL_Reg ldbus_connection [] = {
		{ "open" , ldbus_connection_open },
		{ NULL , NULL }
	};
	luaL_register ( L , NULL , ldbus_connection );
}
