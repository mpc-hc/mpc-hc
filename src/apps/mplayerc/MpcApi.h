/*
 * $Id$
 *
 * (C) 2006-2012 see Authors.txt
 *
 * This file is part of MPC-HC.
 *
 * MPC-HC is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * MPC-HC is distributed in the hope that it will be useful,
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
// If a string contains a |, it will be escaped with a \ so a \| is not a separator
//
// Ex : When a file is opened, mpc-hc send to host the "now playing" notification :
//		- dwData	: CMD_NOWPLAYING
//		- lpData	: title|author|description|filename|duration
//
// Ex : When a DVD is playing, use CMD_GETNOWPLAYING to get:
//		- dwData	: CMD_NOWPLAYING
//		- lpData	: dvddomain|titlenumber|numberofchapters|currentchapter|titleduration
//								dvddomains : DVD - Stopped, DVD - FirstPlay, DVD - RootMenu, DVD - TitleMenu, DVD - Title

#pragma once


typedef enum MPC_LOADSTATE {
	MLS_CLOSED,
	MLS_LOADING,
	MLS_LOADED,
	MLS_CLOSING
};


typedef enum MPC_PLAYSTATE {
	PS_PLAY   = 0,
	PS_PAUSE  = 1,
	PS_STOP   = 2,
	PS_UNUSED = 3
};


struct MPC_OSDDATA {
	int nMsgPos;       // screen position constant (see OSD_MESSAGEPOS constants)
	int nDurationMS;   // duration in milliseconds
	TCHAR strMsg[128]; // message to display thought OSD
};
//// MPC_OSDDATA.nMsgPos constants (for host side programming):
//typedef enum
//{
//    OSD_NOMESSAGE,
//    OSD_TOPLEFT,
//    OSD_TOPRIGHT,
//} OSD_MESSAGEPOS;


typedef enum MPCAPI_COMMAND {
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

	// List of subtitle tracks
	// Par 1 : Subtitle track name 0
	// Par 2 : Subtitle track name 1
	// ...
	// Par n : Active subtitle track, -1 if subtitles disabled
	//
	// if no subtitle track present, returns -1
	// if no file loaded, returns -2
	CMD_LISTSUBTITLETRACKS		= 0x50000004,

	// List of audio tracks
	// Par 1 : Audio track name 0
	// Par 2 : Audio track name 1
	// ...
	// Par n : Active audio track
	//
	// if no audio track present, returns -1
	// if no file loaded, returns -2
	CMD_LISTAUDIOTRACKS			= 0x50000005,

	// Send current playback position in responce
	// of CMD_GETCURRENTPOSITION.
	// Par 1 : current position in seconds
	CMD_CURRENTPOSITION			= 0x50000007,

	// Send the current playback position after a jump.
	// (Automatically sent after a seek event).
	// Par 1 : new playback position (in seconds).
	CMD_NOTIFYSEEK				= 0x50000008,

	// Notify the end of current playback
	// (Automatically sent).
	// Par 1 : none.
	CMD_NOTIFYENDOFSTREAM		= 0x50000009,

	// List of files in the playlist
	// Par 1 : file path 0
	// Par 2 : file path 1
	// ...
	// Par n : active file, -1 if no active file
	CMD_PLAYLIST				= 0x50000006,


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

	// Set the audio delay
	// Par 1 : new audio delay in ms
	CMD_SETAUDIODELAY		= 0xA0002001,

	// Set the subtitle delay
	// Par 1 : new subtitle delay in ms
	CMD_SETSUBTITLEDELAY	= 0xA0002002,

	// Set the active file in the playlist
	// Par 1 : index of the active file, -1 for no file selected
	// DOESN'T WORK
	CMD_SETINDEXPLAYLIST	= 0xA0002003,

	// Set the audio track
	// Par 1 : index of the audio track
	CMD_SETAUDIOTRACK		= 0xA0002004,

	// Set the subtitle track
	// Par 1 : index of the subtitle track, -1 for disabling subtitles
	CMD_SETSUBTITLETRACK	= 0xA0002005,

	// Ask for a list of the subtitles tracks of the file
	// return a CMD_LISTSUBTITLETRACKS
	CMD_GETSUBTITLETRACKS		= 0xA0003000,

	// Ask for the current playback position,
	// see CMD_CURRENTPOSITION.
	// Par 1 : current position in seconds
	CMD_GETCURRENTPOSITION		= 0xA0003004,

	// Jump forward/backward of N seconds,
	// Par 1 : seconds (negative values for backward)
	CMD_JUMPOFNSECONDS			= 0xA0003005,

	// Ask for a list of the audio tracks of the file
	// return a CMD_LISTAUDIOTRACKS
	CMD_GETAUDIOTRACKS			= 0xA0003001,

	// Ask for the properties of the current loaded file
	// return a CMD_NOWPLAYING
	CMD_GETNOWPLAYING			= 0xA0003002,

	// Ask for the current playlist
	// return a CMD_PLAYLIST
	CMD_GETPLAYLIST				= 0xA0003003,

	// Toggle FullScreen
	CMD_TOGGLEFULLSCREEN		= 0xA0004000,

	// Jump forward(medium)
	CMD_JUMPFORWARDMED			= 0xA0004001,

	// Jump backward(medium)
	CMD_JUMPBACKWARDMED			= 0xA0004002,

	// Increase Volume
	CMD_INCREASEVOLUME			= 0xA0004003,

	// Decrease volume
	CMD_DECREASEVOLUME			= 0xA0004004,

	// Shader toggle
	CMD_SHADER_TOGGLE			= 0xA0004005,

	// Close App
	CMD_CLOSEAPP				= 0xA0004006,

	// show host defined OSD message string
	CMD_OSDSHOWMESSAGE			= 0xA0005000,

};
