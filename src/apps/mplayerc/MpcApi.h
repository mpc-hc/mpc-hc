/*
 * $Id: MpcApi.h 193 2007-09-09 09:12:21Z Casimir666 $
 *
 * (C) 2006-2007 see AUTHORS
 *
 * This file is part of mplayerc.
 *
 * Mplayerc is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * Mplayerc is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */



// This file define commands used for "Media Player Classic - Homecinema" API. To send commands
// to mpc-hc, and receive playback notifications, first launch process with the /slave command line
// argument follow by an HWnd handle use to receive notification :
//
// ..\bin\mplayerc /slave 125421
//
// After startup, mpc-hc send a WM_COPYDATA message to host with COPYDATASTRUCT struct filled with :
//		- dwData	: CMD_CONNECT
//		- lpData	: Unicode string containing mpc-hc main window Handle
//
// To pilot mpc-hc, send WM_COPYDATA messages to Hwnd provided on connection. All messages should be
// formatted as Unicode strings. For commands or notifications with multiple parameters, values are
// separated by |
//
// Ex : When a file is openned, mpc-hc send to host the "now playing" notification :
//		- dwData	: CMD_NOWPLAYING
//		- lpData	: title|author|description|filename|duration

#pragma once


typedef enum MPC_LOADSTATE
{
	MLS_CLOSED, 
	MLS_LOADING, 
	MLS_LOADED, 
	MLS_CLOSING
};


typedef enum MPC_PLAYSTATE
{
	PS_PLAY   = 0,
	PS_PAUSE  = 1,
	PS_STOP   = 2,
	PS_UNUSED = 3
};


typedef enum MPCAPI_COMMAND
{
	// ==== Commands from MPC to host
	
	// Send after connection
	// Par 1 : MPC window handle (command should be send to this HWnd)
	CMD_CONNECT				= 0x50000000,		

	// Send when opening or closing file
	// Par 1 : current state (see MPC_LOADSTATE enum)
	CMD_STATE				= 0x50000001,

	// Send when playing, pausing or closing file
	// Par 1 : current play mode (see MPC_PLAYSTATE enum)
	CMD_PLAYMODE			= 0x50000002,

	// Send after opening a new file
	// Par 1 : title
	// Par 2 : author
	// Par 3 : description
	// Par 4 : complete filename (path included)
	// Par 5 : duration in seconds
	CMD_NOWPLAYING			= 0x50000003,

	// List of subtitles
	// Par 1 : Subtitle name 1
	// Par 2 : Subtitle name 2
	// ...
	CMD_SUBTITLES			= 0x50000004,


	// ==== Commands from host to MPC
	
	// Open new file 
	// Par 1 : file path
	CMD_OPENFILE			= 0xA0000000,

	// Stop playback, but keep file / playlist
	CMD_STOP				= 0xA0000001,

	// Stop playback and close file / playlist
	CMD_CLOSEFILE			= 0xA0000002,

	// Pause or restart playback
	CMD_PLAYPAUSE			= 0xA0000003,

	// Add a new file to playlist (did not start playing)
	// Par 1 : file path
	CMD_ADDTOPLAYLIST		= 0xA0001000,

	// Remove all files from playlist
	CMD_CLEARPLAYLIST		= 0xA0001001,

	// Start playing playlist
	CMD_STARTPLAYLIST		= 0xA0001002,

	CMD_REMOVEFROMPLAYLIST	= 0xA0001003,	// TODO

	// Cue current file to specific position
	// Par 1 : new position in seconds
	CMD_SETPOSITION			= 0xA0002000,

	// Cue current file to specific position
	// Par 1 : new audio delay in ms
	CMD_SETAUDIODELAY		= 0xA0002001,
};
