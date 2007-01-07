#ifndef INTERFACE_H
#define INTERFACE_H

#ifdef __cplusplus
extern "C" {
#endif

#pragma pack(push)
#pragma pack(1)

#ifndef BYTE
#define BYTE		unsigned char
#endif

#define PN31SNOOP_SERVICE "pn31snoop"

#define PN31SNOOP_NTNAME         L"\\Device\\PN31Snoop"
#define PN31SNOOP_SYMNAME        L"\\DosDevices\\PN31Snoop"
#define PN31SNOOP_W32NAME_2K     "\\\\.\\PN31Snoop"

// Name pipe for driver <-> application communications
#define APP_PIPENAME			"\\\\.\\pipe\\4C095AF9-A454-48b5-8F07-D241421DB2DA"
#define DRV_PIPENAME			"\\Device\\NamedPipe\\4C095AF9-A454-48b5-8F07-D241421DB2DA"

typedef enum
{
	PC_LOG			= 0,
	PC_BULK_DOWN	= 1
} PIPE_COMMAND;

typedef struct
{
	PIPE_COMMAND	nCode;
	long			lTime;
	long			lSize;
} PIPE_MSG;

typedef enum
{
	K_POWER,
	K_MEDIAPLAYER,	
	K_IEXPLORE,		
	K_EMAIL,		
	K_EXPLORER,		
	K_FAVORITE,		
	K_PREVIOUS,		
	K_NEXT,			
	K_REWIND,		
	K_PLAYPAUSE,	
	K_FORWARD,		
	K_MAXWIN,		
	K_STOP,			
	K_MUTE,			
	K_VOLUP,		
	K_VOLDOWN,		
	K_WINDOWS,		
	K_PGUP,			
	K_PGDOWN,		
	K_ESC,			
	K_TAB,			
	K_DEL,			
	K_1,			
	K_2,			
	K_3,			
	K_4,			
	K_5,			
	K_6,			
	K_7,			
	K_8,			
	K_9,			
	K_0,			
	K_DOT,			
	K_RETURN
} PN31_KEY;

// Keyboard modifiers
#define KM_NUMLOCK			0x01
#define KM_CAPSLOCK			0x02
#define KM_SCROLLLOCK		0x04
#define KM_COMPOSE			0x08
#define KM_KANA				0x10

typedef struct
{
	PN31_KEY	nKey;
	char*		strName;
	/*
	When Code[0] is equal to 1 :
		- Code[1]		: Keys modifiers (num lock, caps lock, ...)
		- Code[3]		: Scancode
	*/
	BYTE		Code[8];
} PN31_KEYCODE;

PN31_KEYCODE	g_PN31Keyboard[];
PN31_KEYCODE* FindPN31Key (BYTE* pBuff);


#pragma pack(pop)

#ifdef __cplusplus
};
#endif

#endif
