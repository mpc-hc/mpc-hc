/*

  lglcd.h

  library definition for lglcd.a
  part of lglcd for Microsoft(R) Windows(R)

  The Logitech LCD SDK, including all acompanying documentation,
  is protected by intellectual property laws.  All use of the Logitech
  LCD SDK is subject to the License Agreement found in the
  "ReadMe License Agreement" file and in the Reference Manual.  All rights
  not expressly granted by Logitech are reserved.


  01/14/2005    1.00    initial draft
  02/23/2005    1.01    added callbacks, implemented changes as discussed
  02/08/2006    1.02    added call to set foreground, sync update with confirmation
  05/29/2006    1.03    Added device family feature
  12/03/2007    3.00    Added support for color devices and partial event notifications (arrival and removal)
  10/03/2008    3.01    Added more event notifications
  10/27/2008    3.01    Deprecated enumeration and index-based open; Applets should use lgLcdOpenByType() (and
                        possibly notifications)

*/

#ifndef _LGLCD_H_INCLUDED_
#define _LGLCD_H_INCLUDED_

#ifdef __cplusplus
extern "C" {
#endif

#pragma pack(push, 8)

//// Definitions

// Invalid handle definitions
#define LGLCD_INVALID_CONNECTION            (-1)
#define LGLCD_INVALID_DEVICE                (-1)


// Common Soft-Buttons available through the SDK
#define LGLCDBUTTON_LEFT                    (0x00000100)
#define LGLCDBUTTON_RIGHT                   (0x00000200)
#define LGLCDBUTTON_OK                      (0x00000400)
#define LGLCDBUTTON_CANCEL                  (0x00000800)
#define LGLCDBUTTON_UP                      (0x00001000)
#define LGLCDBUTTON_DOWN                    (0x00002000)
#define LGLCDBUTTON_MENU                    (0x00004000)

// Soft-Button masks. Kept for backwards compatibility
#define LGLCDBUTTON_BUTTON0                 (0x00000001)
#define LGLCDBUTTON_BUTTON1                 (0x00000002)
#define LGLCDBUTTON_BUTTON2                 (0x00000004)
#define LGLCDBUTTON_BUTTON3                 (0x00000008)
#define LGLCDBUTTON_BUTTON4                 (0x00000010)
#define LGLCDBUTTON_BUTTON5                 (0x00000020)
#define LGLCDBUTTON_BUTTON6                 (0x00000040)
#define LGLCDBUTTON_BUTTON7                 (0x00000080)

//************************************************************************
// lgLcdDeviceDesc
//************************************************************************
typedef struct
{
    DWORD Width;
    DWORD Height;
    DWORD Bpp;
    DWORD NumSoftButtons;
} lgLcdDeviceDesc;


//************************************************************************
// lgLcdDeviceDescEx
//************************************************************************
typedef struct
{
    DWORD deviceFamilyId;
    WCHAR deviceDisplayName[MAX_PATH];
    DWORD Width;            // # of pixels (horizontally) on the LCD
    DWORD Height;           // # of pixels (lines) on the LCD
    DWORD Bpp;              // # of bits per pixel (1,8,16,24,...)
    DWORD NumSoftButtons;
    DWORD Reserved1;
    DWORD Reserved2;
} lgLcdDeviceDescExW;

typedef struct
{
    DWORD deviceFamilyId;
    CHAR  deviceDisplayName[MAX_PATH];
    DWORD Width;
    DWORD Height;
    DWORD Bpp;
    DWORD NumSoftButtons;
    DWORD Reserved1;
    DWORD Reserved2;
} lgLcdDeviceDescExA;

#ifdef UNICODE
typedef lgLcdDeviceDescExW lgLcdDeviceDescEx;
#else
typedef lgLcdDeviceDescExA lgLcdDeviceDescEx;
#endif

//************************************************************************
// lgLcdBitmap
//************************************************************************

#define LGLCD_BMP_FORMAT_160x43x1           (0x00000001)
#define LGLCD_BMP_FORMAT_QVGAx32            (0x00000003)
#define LGLCD_BMP_WIDTH                     (160)
#define LGLCD_BMP_HEIGHT                    (43)
#define LGLCD_BMP_BPP                       (1)
#define LGLCD_BW_BMP_WIDTH                  (160)
#define LGLCD_BW_BMP_HEIGHT                 (43)
#define LGLCD_BW_BMP_BPP                    (1)
#define LGLCD_QVGA_BMP_WIDTH                (320)
#define LGLCD_QVGA_BMP_HEIGHT               (240)
#define LGLCD_QVGA_BMP_BPP                  (4)

typedef struct
{
    DWORD Format;
} lgLcdBitmapHeader;

typedef struct
{
    lgLcdBitmapHeader hdr;
    BYTE pixels[LGLCD_BMP_WIDTH * LGLCD_BMP_HEIGHT * LGLCD_BMP_BPP];
} lgLcdBitmap160x43x1;

typedef struct
{
    lgLcdBitmapHeader hdr;  // Format = LGLCD_BMP_FORMAT_QVGAx32
    BYTE pixels[LGLCD_QVGA_BMP_WIDTH * LGLCD_QVGA_BMP_HEIGHT * LGLCD_QVGA_BMP_BPP];
} lgLcdBitmapQVGAx32;
//
// Generic bitmap for use by both color and BW applets
//
typedef union
{
    lgLcdBitmapHeader hdr;          // provides easy access to the header
    lgLcdBitmap160x43x1 bmp_mono;   // B/W bitmap data
    lgLcdBitmapQVGAx32 bmp_qvga32;  // Color bitmap data
} lgLcdBitmap;

// Priorities
#define LGLCD_PRIORITY_IDLE_NO_SHOW                 (0)
#define LGLCD_PRIORITY_BACKGROUND                   (64)
#define LGLCD_PRIORITY_NORMAL                       (128)
#define LGLCD_PRIORITY_ALERT                        (255)
#define LGLCD_SYNC_UPDATE(priority)                 (0x80000000 | (priority))
#define LGLCD_SYNC_COMPLETE_WITHIN_FRAME(priority)  (0xC0000000 | (priority))
#define LGLCD_ASYNC_UPDATE(priority)                (priority)

// Foreground mode for client applications
#define LGLCD_LCD_FOREGROUND_APP_NO                 (0)
#define LGLCD_LCD_FOREGROUND_APP_YES                (1)

// Device family definitions
#define LGLCD_DEVICE_FAMILY_BW_160x43_GAMING        (0x00000001)
#define LGLCD_DEVICE_FAMILY_KEYBOARD_G15            (0x00000001)
#define LGLCD_DEVICE_FAMILY_BW_160x43_AUDIO         (0x00000002)
#define LGLCD_DEVICE_FAMILY_SPEAKERS_Z10            (0x00000002)
#define LGLCD_DEVICE_FAMILY_JACKBOX                 (0x00000004)
#define LGLCD_DEVICE_FAMILY_BW_160x43_BASIC         (0x00000008)
#define LGLCD_DEVICE_FAMILY_LCDEMULATOR_G15         (0x00000008)
#define LGLCD_DEVICE_FAMILY_RAINBOW                 (0x00000010)
#define LGLCD_DEVICE_FAMILY_QVGA_BASIC              (0x00000020)
#define LGLCD_DEVICE_FAMILY_QVGA_GAMING             (0x00000040)
#define LGLCD_DEVICE_FAMILY_GAMEBOARD_G13           (0x00000080)
#define LGLCD_DEVICE_FAMILY_KEYBOARD_G510           (0x00000100)
#define LGLCD_DEVICE_FAMILY_OTHER                   (0x80000000)

// Combinations of device families (device clans?)
#define LGLCD_DEVICE_FAMILY_ALL_BW_160x43           (LGLCD_DEVICE_FAMILY_BW_160x43_GAMING \
                                                   | LGLCD_DEVICE_FAMILY_BW_160x43_AUDIO \
                                                   | LGLCD_DEVICE_FAMILY_JACKBOX \
                                                   | LGLCD_DEVICE_FAMILY_BW_160x43_BASIC \
                                                   | LGLCD_DEVICE_FAMILY_RAINBOW \
                                                   | LGLCD_DEVICE_FAMILY_GAMEBOARD_G13 \
                                                   | LGLCD_DEVICE_FAMILY_KEYBOARD_G510)

#define LGLCD_DEVICE_FAMILY_ALL_QVGA                (LGLCD_DEVICE_FAMILY_QVGA_BASIC \
                                                   | LGLCD_DEVICE_FAMILY_QVGA_GAMING)

#define LGLCD_DEVICE_FAMILY_ALL                     (LGLCD_DEVICE_FAMILY_ALL_BW_160x43 \
                                                   | LGLCD_DEVICE_FAMILY_ALL_QVGA)


// Capabilities of applets connecting to LCD Manager.
#define LGLCD_APPLET_CAP_BASIC                          (0x00000000)
#define LGLCD_APPLET_CAP_BW                             (0x00000001)
#define LGLCD_APPLET_CAP_QVGA                           (0x00000002)

// Notifications sent by LCD Manager to applets connected to it.
#define LGLCD_NOTIFICATION_DEVICE_ARRIVAL               (0x00000001)
#define LGLCD_NOTIFICATION_DEVICE_REMOVAL               (0x00000002)
#define LGLCD_NOTIFICATION_CLOSE_CONNECTION             (0x00000003)
#define LGLCD_NOTIFICATION_APPLET_DISABLED              (0x00000004)
#define LGLCD_NOTIFICATION_APPLET_ENABLED               (0x00000005)
#define LGLCD_NOTIFICATION_TERMINATE_APPLET             (0x00000006)

// Device types used in notifications
#define LGLCD_DEVICE_BW                                 (0x00000001)
#define LGLCD_DEVICE_QVGA                               (0x00000002)

//************************************************************************
// Callbacks
//************************************************************************

// Callback used to notify client of soft button change
typedef DWORD (WINAPI *lgLcdOnSoftButtonsCB)(IN int device,
                                             IN DWORD dwButtons,
                                             IN const PVOID pContext);

// Callback used to allow client to pop up a "configuration panel"
typedef DWORD (WINAPI *lgLcdOnConfigureCB)(IN int connection,
                                           IN const PVOID pContext);

// Callback used to notify client of events, such as device arrival, ...
// Arrival, removal, applet enable/disable supported as of version 3.0.
typedef DWORD (WINAPI *lgLcdOnNotificationCB)(IN int connection,
                                              IN const PVOID pContext,
                                              IN DWORD notificationCode,
                                              IN DWORD notifyParm1,
                                              IN DWORD notifyParm2,
                                              IN DWORD notifyParm3,
                                              IN DWORD notifyParm4);


//************************************************************************
// lgLcdConfigureContext
//************************************************************************
typedef struct
{
    // Set to NULL if not configurable
    lgLcdOnConfigureCB  configCallback;
    PVOID               configContext;
} lgLcdConfigureContext;

//************************************************************************
// lgLcdNotificationContext
//************************************************************************
typedef struct
{
    // Set to NULL if not notifiable
    lgLcdOnNotificationCB   notificationCallback;
    PVOID                   notifyContext;
} lgLcdNotificationContext;

//************************************************************************
// lgLcdConnectContext
//************************************************************************
typedef struct
{
    // "Friendly name" display in the listing
    LPCWSTR appFriendlyName;
    // isPersistent determines whether this connection persists in the list
    BOOL isPersistent;
    // isAutostartable determines whether the client can be started by
    // LCDMon
    BOOL isAutostartable;
    lgLcdConfigureContext onConfigure;
    // --> Connection handle
    int connection;
} lgLcdConnectContextW;

typedef struct
{
    // "Friendly name" display in the listing
    LPCSTR appFriendlyName;
    // isPersistent determines whether this connection persists in the list
    BOOL isPersistent;
    // isAutostartable determines whether the client can be started by
    // LCDMon
    BOOL isAutostartable;
    lgLcdConfigureContext onConfigure;
    // --> Connection handle
    int connection;
} lgLcdConnectContextA;

//************************************************************************
// lgLcdConnectContextEx
//************************************************************************
typedef struct
{
    // "Friendly name" display in the listing
    LPCWSTR appFriendlyName;
    // isPersistent determines whether this connection persists in the list
    BOOL isPersistent;
    // isAutostartable determines whether the client can be started by
    // LCDMon
    BOOL isAutostartable;
    lgLcdConfigureContext onConfigure;
    // --> Connection handle
    int connection;
    // New additions added in 1.03 revision
    DWORD dwAppletCapabilitiesSupported;    // Or'd combination of LGLCD_APPLET_CAP_... defines
    DWORD dwReserved1;
    lgLcdNotificationContext onNotify;
} lgLcdConnectContextExW;

typedef struct
{
    // "Friendly name" display in the listing
    LPCSTR appFriendlyName;
    // isPersistent determines whether this connection persists in the list
    BOOL isPersistent;
    // isAutostartable determines whether the client can be started by
    // LCDMon
    BOOL isAutostartable;
    lgLcdConfigureContext onConfigure;
    // --> Connection handle
    int connection;
    // New additions added in 1.03 revision
    DWORD dwAppletCapabilitiesSupported;    // Or'd combination of LGLCD_APPLET_CAP_... defines
    DWORD dwReserved1;
    lgLcdNotificationContext onNotify;
} lgLcdConnectContextExA;

#ifdef UNICODE
typedef lgLcdConnectContextW lgLcdConnectContext;
typedef lgLcdConnectContextExW lgLcdConnectContextEx;
#else
typedef lgLcdConnectContextA lgLcdConnectContext;
typedef lgLcdConnectContextExA lgLcdConnectContextEx;
#endif

//************************************************************************
// lgLcdOpenContext & lgLcdOpenByTypeContext
//************************************************************************

typedef struct
{
    // Set to NULL if no softbutton notifications are needed
    lgLcdOnSoftButtonsCB softbuttonsChangedCallback;
    PVOID softbuttonsChangedContext;
} lgLcdSoftbuttonsChangedContext;

typedef struct
{
    int connection;
    // Device index to open
    int index;
    lgLcdSoftbuttonsChangedContext onSoftbuttonsChanged;
    // --> Device handle
    int device;
} lgLcdOpenContext;

typedef struct
{
    int connection;
    // Device type to open (either LGLCD_DEVICE_BW or LGLCD_DEVICE_QVGA)
    int deviceType;
    lgLcdSoftbuttonsChangedContext onSoftbuttonsChanged;
    // --> Device handle
    int device;
} lgLcdOpenByTypeContext;


//************************************************************************
// Prototypes
//************************************************************************

// Initialize the library by calling this function.
DWORD WINAPI lgLcdInit(void);

// Must be called to release the library and free all allocated structures.
DWORD WINAPI lgLcdDeInit(void);

// Connect as a client to the LCD subsystem. Provide name to be
// displayed for user when viewing the user interface of the LCD module,
// as well as a configuration callback and context, and a flag that states
// whether this client is startable by LCDMon
DWORD WINAPI lgLcdConnectW(IN OUT lgLcdConnectContextW *ctx);
DWORD WINAPI lgLcdConnectA(IN OUT lgLcdConnectContextA *ctx);
DWORD WINAPI lgLcdConnectExW(IN OUT lgLcdConnectContextExW *ctx);
DWORD WINAPI lgLcdConnectExA(IN OUT lgLcdConnectContextExA *ctx);
#ifdef UNICODE
#define lgLcdConnect lgLcdConnectW
#define lgLcdConnectEx lgLcdConnectExW
#else
#define lgLcdConnect lgLcdConnectA
#define lgLcdConnectEx lgLcdConnectExA
#endif // !UNICODE

// Must be called to release the connection and free all allocated resources
DWORD WINAPI lgLcdDisconnect(int connection);

// New additions added in 1.03 revision of API. Call this method to setup which device families the applet
// is interested in. After this call, the applet can use lgLcdEnumerateEx to determine
// if a device from the device family wanted is found.
DWORD WINAPI lgLcdSetDeviceFamiliesToUse(IN int connection,
                                         DWORD dwDeviceFamiliesSupported,    // Or'd combination of LGLCD_DEVICE_FAMILY_... defines
                                         DWORD dwReserved1);

// To determine all connected LCD devices supported by this library, and
// their capabilities. Start with index 0, and increment by one, until
// the library returns an error (WHICH?).
DWORD WINAPI lgLcdEnumerate(IN int connection, IN int index,
                            OUT lgLcdDeviceDesc *description);

// To determine all connected LCD devices supported by this library, and
// their capabilities. Start with 0, and increment by one, until
// the library returns an error (WHICH?).
DWORD WINAPI lgLcdEnumerateExW(IN int connection, IN int index,
                               OUT lgLcdDeviceDescExW *description);
DWORD WINAPI lgLcdEnumerateExA(IN int connection, IN int index,
                               OUT lgLcdDeviceDescExA *description);
#ifdef UNICODE
#define lgLcdEnumerateEx lgLcdEnumerateExW
#else
#define lgLcdEnumerateEx lgLcdEnumerateExA
#endif // !UNICODE

// Opens the LCD at position=index. Library sets the device parameter to
// its internal reference to the device. Calling application provides the
// device handle in all calls that access the LCD.
DWORD WINAPI lgLcdOpen(IN OUT lgLcdOpenContext *ctx);

// Opens an LCD of the specified type. If no such device is available, returns
// an error.
DWORD WINAPI lgLcdOpenByType(IN OUT lgLcdOpenByTypeContext *ctx);

// Closes the LCD. Must be paired with lgLcdOpen()/lgLcdOpenByType().
DWORD WINAPI lgLcdClose(IN int device);

// Reads the state of the soft buttons for the device.
DWORD WINAPI lgLcdReadSoftButtons(IN int device, OUT DWORD *buttons);

// Provides a bitmap to be displayed on the LCD. The priority field
// further describes the way in which the bitmap is to be applied.
DWORD WINAPI lgLcdUpdateBitmap(IN int device,
                               IN const lgLcdBitmapHeader *bitmap,
                               IN DWORD priority);

// Sets the calling application as the shown application on the LCD, and stops
// any type of rotation among other applications on the LCD.
DWORD WINAPI lgLcdSetAsLCDForegroundApp(IN int device, IN int foregroundYesNoFlag);

// These API calls are being deprecated. Consider using lgLcdOpenByType() and
// device arrival/removal notifications instead.
#pragma deprecated(lgLcdEnumerate,lgLcdEnumerateExA,lgLcdEnumerateExW,lgLcdOpen)


#pragma pack(pop)

#ifdef __cplusplus
}
#endif

#endif // _LGLCD_H_INCLUDED_

//** end of lglcd.h ***************************************************
