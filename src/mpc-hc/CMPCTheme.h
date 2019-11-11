#pragma once
#include "mplayerc.h"

class CMPCTheme {
public:
    static const COLORREF MenuBGColor;
    static const COLORREF WindowBGColor;  //used in explorer for left nav
    static const COLORREF ControlAreaBGColor;  //used in file open dialog for button / file selection bg
    static const COLORREF ContentBGColor; //used in explorer for bg of file list
    static const COLORREF ContentSelectedColor; //used in explorer for bg of file list
    static const COLORREF PlayerBGColor;
    static const COLORREF HighLightColor;

    static const COLORREF MenuSelectedColor;
    static const COLORREF MenuItemDisabledColor;
    static const COLORREF MenuSeparatorColor;

    static const COLORREF ShadowColor;
    static const COLORREF TextFGColor;
    static const COLORREF TextFGColorFade;
    static const COLORREF ContentTextDisabledFGColorFade;
    static const COLORREF ContentTextDisabledFGColorFade2;
    static const COLORREF SubmenuColor;
    static const COLORREF LightColor;
    static const COLORREF CloseHoverColor;
    static const COLORREF ClosePushColor;
    static const COLORREF CloseColor;
    static const COLORREF WindowBorderColorLight;
    static const COLORREF WindowBorderColorDim;
    static const COLORREF NoBorderColor;
    static const COLORREF GripperPatternColor;

    static const COLORREF ScrollBGColor;
    static const COLORREF ScrollProgressColor;
    static const COLORREF ScrollChapterColor;
    static const COLORREF ScrollThumbColor;
    static const COLORREF ScrollThumbHoverColor;
    static const COLORREF ScrollThumbDragColor;
    static const COLORREF ScrollButtonArrowColor;
    static const COLORREF ScrollButtonHoverColor;
    static const COLORREF ScrollButtonClickColor;

    static const COLORREF InlineEditBorderColor;
    static const COLORREF TooltipBorderColor;

    static const COLORREF GroupBoxBorderColor;
    static const int GroupBoxTextIndent;

    static const COLORREF DebugColorRed;
    static const COLORREF DebugColorYellow;
    static const COLORREF DebugColorGreen;

    static const COLORREF PlayerButtonHotColor;
    static const COLORREF PlayerButtonCheckedColor;
    static const COLORREF PlayerButtonClickedColor;
    static const COLORREF PlayerButtonBorderColor;

    static const COLORREF ButtonBorderOuterColor;
    static const COLORREF ButtonBorderInnerFocusedColor;
    static const COLORREF ButtonBorderInnerColor;
    static const COLORREF ButtonBorderSelectedKBFocusColor;
    static const COLORREF ButtonBorderHoverKBFocusColor;
    static const COLORREF ButtonBorderKBFocusColor;
    static const COLORREF ButtonFillColor;
    static const COLORREF ButtonFillHoverColor;
    static const COLORREF ButtonFillSelectedColor;
    static const COLORREF ButtonDisabledFGColor;

    static const COLORREF CheckboxBorderColor;
    static const COLORREF CheckboxBGColor;
    static const COLORREF CheckboxBorderHoverColor;
    static const COLORREF CheckboxBGHoverColor;

    static const COLORREF ImageDisabledColor;

    static const COLORREF SliderChannelColor;

    static const COLORREF EditBorderColor;

    static const COLORREF TreeCtrlLineColor;
    static const COLORREF TreeCtrlHoverColor;
    static const COLORREF TreeCtrlFocusColor;

    static const COLORREF CheckColor;

    static const COLORREF ColumnHeaderHotColor;

    static const COLORREF StaticEtchedColor;

    static const COLORREF ListCtrlDisabledBGColor;
    static const COLORREF ListCtrlGridColor;
    static const COLORREF HeaderCtrlGridColor;
    static const COLORREF AudioSwitcherGridColor;

    static const COLORREF TabCtrlBorderColor;
    static const COLORREF TabCtrlInactiveColor;

    static const COLORREF StatusBarBGColor;
    static const COLORREF StatusBarSeparatorColor;
    static const COLORREF StatusBarEditBorderColor;

    static const COLORREF W10DarkThemeFileDialogInjectedTextColor;
    static const COLORREF W10DarkThemeFileDialogInjectedBGColor;
    static const COLORREF W10DarkThemeFileDialogInjectedEditBorderColor;
    static const COLORREF W10DarkThemeTitlebarBGColor;
    static const COLORREF W10DarkThemeTitlebarInactiveBGColor;
    static const COLORREF W10DarkThemeTitlebarFGColor;
    static const COLORREF W10DarkThemeTitlebarInactiveFGColor;
    static const COLORREF W10DarkThemeTitlebarIconPenColor;
    static const COLORREF W10DarkThemeTitlebarControlHoverBGColor;
    static const COLORREF W10DarkThemeTitlebarInactiveControlHoverBGColor;
    static const COLORREF W10DarkThemeTitlebarControlPushedBGColor;
    static const COLORREF W10DarkThemeWindowBorderColor;

    static const COLORREF ProgressBarBGColor;
    static const COLORREF ProgressBarColor;

    static const COLORREF SubresyncFadeText1;
    static const COLORREF SubresyncFadeText2;
    static const COLORREF SubresyncActiveFadeText;
    static const COLORREF SubresyncHLColor1;
    static const COLORREF SubresyncHLColor2;
    static const COLORREF SubresyncGridSepColor;

    static const COLORREF ActivePlayListItemColor;
    static const COLORREF ActivePlayListItemHLColor;
    static const COLORREF StaticLinkColor;
    static const COLORREF SeekbarCurrentPositionColor;

    static const BYTE GripperBitsH[10];
    static const BYTE GripperBitsV[8];
    static const int gripPatternShort;
    static const int gripPatternLong;

    static wchar_t* const uiTextFont;
    static wchar_t* const uiStaticTextFont;
    static wchar_t* const uiSymbolFont;


    static const COLORREF ComboboxArrowColor;
    static const COLORREF ComboboxArrowColorDisabled;

    static const BYTE CheckBits[14];
    static const int CheckWidth;
    static const int CheckHeight;

    const static UINT ThemeCheckBoxes[5];
    const static UINT ThemeRadios[5];

    enum pathState {
        linePath,
        newPath,
        closePath
    };

    struct pathPoint {
        float x;
        float y;
        pathState state;
    };
    static const std::vector<pathPoint> minimizeIcon96, minimizeIcon120, minimizeIcon144, minimizeIcon168, minimizeIcon192;
    static const std::vector<pathPoint> maximizeIcon96, maximizeIcon120, maximizeIcon144, maximizeIcon168, maximizeIcon192;
    static const std::vector<pathPoint> restoreIcon96, restoreIcon120, restoreIcon144, restoreIcon168, restoreIcon192;
    static const std::vector<pathPoint> closeIcon96, closeIcon120, closeIcon144, closeIcon168, closeIcon192;
    static const int CMPCTheme::W10TitlebarIconPathHeight[5];
    static const int CMPCTheme::W10TitlebarIconPathWidth[5];
    static const float CMPCTheme::W10TitlebarIconPathThickness[5];
    static const int CMPCTheme::W10TitlebarButtonWidth[5];
    static const int CMPCTheme::W10TitlebarButtonSpacing[5];
};
