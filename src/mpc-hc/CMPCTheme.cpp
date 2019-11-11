#include "stdafx.h"
#include "CMPCTheme.h"
#include "mplayerc.h"

const COLORREF CMPCTheme::MenuBGColor = COLORREF(RGB(43, 43, 43));
const COLORREF CMPCTheme::WindowBGColor = COLORREF(RGB(25, 25, 25));
const COLORREF CMPCTheme::ControlAreaBGColor = COLORREF(RGB(56, 56, 56));

const COLORREF CMPCTheme::ContentBGColor = COLORREF(RGB(32, 32, 32));
const COLORREF CMPCTheme::ContentSelectedColor = COLORREF(RGB(119, 119, 119));
const COLORREF CMPCTheme::PlayerBGColor = COLORREF(RGB(32, 32, 32));

const COLORREF CMPCTheme::HighLightColor = GetSysColor(COLOR_HIGHLIGHT);

const COLORREF CMPCTheme::MenuSelectedColor = COLORREF(RGB(65, 65, 65));
const COLORREF CMPCTheme::MenuSeparatorColor = COLORREF(RGB(128, 128, 128));
const COLORREF CMPCTheme::MenuItemDisabledColor = COLORREF(RGB(109, 109, 109));

const COLORREF CMPCTheme::ShadowColor = COLORREF(RGB(25, 25, 25));
const COLORREF CMPCTheme::TextFGColor = COLORREF(RGB(255, 255, 255));
const COLORREF CMPCTheme::TextFGColorFade = COLORREF(RGB(200, 200, 200));
const COLORREF CMPCTheme::ContentTextDisabledFGColorFade = COLORREF(RGB(109, 109, 109));
const COLORREF CMPCTheme::ContentTextDisabledFGColorFade2 = COLORREF(RGB(60, 60, 60)); //even more faded, used for NA text on CListCtrl/audio switcher

const COLORREF CMPCTheme::SubmenuColor = COLORREF(RGB(191, 191, 191));
const COLORREF CMPCTheme::LightColor = COLORREF(RGB(100, 100, 100));
const COLORREF CMPCTheme::CloseHoverColor = COLORREF(RGB(232, 17, 35));
const COLORREF CMPCTheme::ClosePushColor = COLORREF(RGB(139, 10, 20));

const COLORREF CMPCTheme::WindowBorderColorLight = COLORREF(RGB(99, 99, 99));
const COLORREF CMPCTheme::WindowBorderColorDim = COLORREF(RGB(43, 43, 43));
const COLORREF CMPCTheme::NoBorderColor = COLORREF(RGB(0, 0, 0));
const COLORREF CMPCTheme::GripperPatternColor = COLORREF(RGB(70, 70, 74)); //visual studio dark, since explorer has no grippers

const COLORREF CMPCTheme::ScrollBGColor = COLORREF(RGB(23, 23, 23));
const COLORREF CMPCTheme::ScrollProgressColor = COLORREF(RGB(60, 60, 60));
const COLORREF CMPCTheme::ScrollChapterColor = COLORREF(RGB(100, 100, 100));
const COLORREF CMPCTheme::ScrollThumbColor = COLORREF(RGB(77, 77, 77));
const COLORREF CMPCTheme::ScrollThumbHoverColor = COLORREF(RGB(144, 144, 144));
const COLORREF CMPCTheme::ScrollThumbDragColor = COLORREF(RGB(183, 183, 183));
const COLORREF CMPCTheme::ScrollButtonArrowColor = COLORREF(RGB(103, 103, 103));
const COLORREF CMPCTheme::ScrollButtonHoverColor = COLORREF(RGB(55, 55, 55));
const COLORREF CMPCTheme::ScrollButtonClickColor = COLORREF(RGB(166, 166, 166));

const COLORREF CMPCTheme::InlineEditBorderColor = COLORREF(RGB(255, 255, 255));
const COLORREF CMPCTheme::TooltipBorderColor = COLORREF(RGB(118, 118, 118));

const COLORREF CMPCTheme::GroupBoxBorderColor = COLORREF(RGB(118, 118, 118));
const int CMPCTheme::GroupBoxTextIndent = 8;

const COLORREF CMPCTheme::PlayerButtonHotColor = COLORREF(RGB(43, 43, 43));
const COLORREF CMPCTheme::PlayerButtonCheckedColor = COLORREF(RGB(66, 66, 66)); 
const COLORREF CMPCTheme::PlayerButtonClickedColor = COLORREF(RGB(55, 55, 55));
const COLORREF CMPCTheme::PlayerButtonBorderColor = COLORREF(RGB(0, 0, 0));

const COLORREF CMPCTheme::DebugColorRed = COLORREF(RGB(255, 0, 0));
const COLORREF CMPCTheme::DebugColorYellow = COLORREF(RGB(255, 255, 0));
const COLORREF CMPCTheme::DebugColorGreen = COLORREF(RGB(0, 255, 0));

const COLORREF CMPCTheme::ButtonBorderOuterColor = COLORREF(RGB(240, 240, 240));
const COLORREF CMPCTheme::ButtonBorderInnerFocusedColor = COLORREF(RGB(255, 255, 255));
const COLORREF CMPCTheme::ButtonBorderInnerColor = COLORREF(RGB(155, 155, 155));
const COLORREF CMPCTheme::ButtonBorderSelectedKBFocusColor = COLORREF(RGB(150, 150, 150));
const COLORREF CMPCTheme::ButtonBorderHoverKBFocusColor = COLORREF(RGB(181, 181, 181));
const COLORREF CMPCTheme::ButtonBorderKBFocusColor = COLORREF(RGB(195, 195, 195));
const COLORREF CMPCTheme::ButtonFillColor = COLORREF(RGB(51, 51, 51));
const COLORREF CMPCTheme::ButtonFillHoverColor = COLORREF(RGB(69, 69, 69));
const COLORREF CMPCTheme::ButtonFillSelectedColor = COLORREF(RGB(102, 102, 102));
const COLORREF CMPCTheme::ButtonDisabledFGColor = COLORREF(RGB(109, 109, 109));

const COLORREF CMPCTheme::CheckboxBorderColor = COLORREF(RGB(137, 137, 137));
const COLORREF CMPCTheme::CheckboxBGColor = COLORREF(RGB(0, 0, 0));
const COLORREF CMPCTheme::CheckboxBorderHoverColor = COLORREF(RGB(121, 121, 121));
const COLORREF CMPCTheme::CheckboxBGHoverColor = COLORREF(RGB(8, 8, 8));

const COLORREF CMPCTheme::ImageDisabledColor = COLORREF(RGB(109, 109, 109));

const COLORREF CMPCTheme::SliderChannelColor = COLORREF(RGB(109, 109, 109));

const COLORREF CMPCTheme::EditBorderColor = COLORREF(RGB(106, 106, 106));

const COLORREF CMPCTheme::TreeCtrlLineColor = COLORREF(RGB(106, 106, 106));
const COLORREF CMPCTheme::TreeCtrlHoverColor = COLORREF(RGB(77, 77, 77));
const COLORREF CMPCTheme::TreeCtrlFocusColor = COLORREF(RGB(98, 98, 98));

const COLORREF CMPCTheme::CheckColor = COLORREF(RGB(222, 222, 222));

const COLORREF CMPCTheme::ColumnHeaderHotColor = COLORREF(RGB(67, 67, 67));

const COLORREF CMPCTheme::StaticEtchedColor = COLORREF(RGB(65, 65, 65));

const COLORREF CMPCTheme::ListCtrlDisabledBGColor = COLORREF(RGB(40, 40, 40));
const COLORREF CMPCTheme::ListCtrlGridColor = COLORREF(RGB(43, 43, 43));
const COLORREF CMPCTheme::HeaderCtrlGridColor = COLORREF(RGB(99, 99, 99));
const COLORREF CMPCTheme::AudioSwitcherGridColor = COLORREF(RGB(99, 99, 99));

const COLORREF CMPCTheme::TabCtrlBorderColor = COLORREF(RGB(99, 99, 99));
const COLORREF CMPCTheme::TabCtrlInactiveColor = COLORREF(RGB(40, 40, 40));


const COLORREF CMPCTheme::StatusBarBGColor = COLORREF(RGB(51, 51, 51));
const COLORREF CMPCTheme::StatusBarSeparatorColor = COLORREF(RGB(247, 247, 247));

const COLORREF CMPCTheme::W10DarkThemeFileDialogInjectedTextColor = COLORREF(RGB(255, 255, 255));
const COLORREF CMPCTheme::W10DarkThemeFileDialogInjectedBGColor = COLORREF(RGB(56, 56, 56));
const COLORREF CMPCTheme::W10DarkThemeFileDialogInjectedEditBorderColor = COLORREF(RGB(155, 155, 155));
const COLORREF CMPCTheme::W10DarkThemeTitlebarBGColor = COLORREF(RGB(0, 0, 0));
const COLORREF CMPCTheme::W10DarkThemeTitlebarInactiveBGColor = COLORREF(RGB(43, 43, 43));
const COLORREF CMPCTheme::W10DarkThemeTitlebarFGColor = COLORREF(RGB(255, 255, 255));
const COLORREF CMPCTheme::W10DarkThemeTitlebarInactiveFGColor = COLORREF(RGB(170, 170, 170));
const COLORREF CMPCTheme::W10DarkThemeTitlebarIconPenColor = COLORREF(RGB(255, 255, 255));
const COLORREF CMPCTheme::W10DarkThemeTitlebarControlHoverBGColor = COLORREF(RGB(43, 43, 43));
const COLORREF CMPCTheme::W10DarkThemeTitlebarInactiveControlHoverBGColor = COLORREF(RGB(65, 65, 65));
const COLORREF CMPCTheme::W10DarkThemeTitlebarControlPushedBGColor = COLORREF(RGB(70, 70, 70));
const COLORREF CMPCTheme::W10DarkThemeWindowBorderColor = COLORREF(RGB(57, 57, 57));


const COLORREF CMPCTheme::ProgressBarBGColor = COLORREF(RGB(0, 0, 0));
const COLORREF CMPCTheme::ProgressBarColor = COLORREF(RGB(75, 75, 75));

const COLORREF CMPCTheme::SubresyncFadeText1 = COLORREF(RGB(190, 190, 190));
const COLORREF CMPCTheme::SubresyncFadeText2 = COLORREF(RGB(160, 160, 160));
const COLORREF CMPCTheme::SubresyncActiveFadeText = COLORREF(RGB(215, 215, 215));
const COLORREF CMPCTheme::SubresyncHLColor1 = COLORREF(RGB(100, 100, 100));
const COLORREF CMPCTheme::SubresyncHLColor2 = COLORREF(RGB(80, 80, 80));
const COLORREF CMPCTheme::SubresyncGridSepColor = COLORREF(RGB(220, 220, 220));

const COLORREF CMPCTheme::ActivePlayListItemColor = COLORREF(RGB(38, 160, 218));
const COLORREF CMPCTheme::ActivePlayListItemHLColor = COLORREF(RGB(0, 40, 110));
const COLORREF CMPCTheme::StaticLinkColor = COLORREF(RGB(38, 160, 218));

const COLORREF CMPCTheme::SeekbarCurrentPositionColor = COLORREF(RGB(38, 160, 218));

wchar_t* const CMPCTheme::uiTextFont = L"Segoe UI";
wchar_t* const CMPCTheme::uiStaticTextFont = L"Segoe UI Semilight";
wchar_t* const CMPCTheme::uiSymbolFont = L"MS UI Gothic";


const int CMPCTheme::gripPatternLong = 5;
const int CMPCTheme::gripPatternShort = 4;

const BYTE CMPCTheme::GripperBitsH[10] = {
    0x80, 0x00,
    0x00, 0x00,
    0x20, 0x00,
    0x00, 0x00,
    0x80, 0x00,
};

const BYTE CMPCTheme::GripperBitsV[8] = {
    0x88, 0x00,
    0x00, 0x00,
    0x20, 0x00,
    0x00, 0x00,
};

const COLORREF CMPCTheme::ComboboxArrowColor = COLORREF(RGB(200, 200, 200));
const COLORREF CMPCTheme::ComboboxArrowColorDisabled = COLORREF(RGB(100, 100, 100));

const BYTE CMPCTheme::CheckBits[14] = {
    0x02, 0x00,
    0x06, 0x00,
    0x8E, 0x00,
    0xDC, 0x00,
    0xF8, 0x00,
    0x70, 0x00,
    0x20, 0x00,
};

const int CMPCTheme::CheckWidth = 7;
const int CMPCTheme::CheckHeight = 7;


const UINT CMPCTheme::ThemeCheckBoxes[5] = {
    IDB_DT_CB_96,
    IDB_DT_CB_120,
    IDB_DT_CB_144,
    IDB_DT_CB_144,
    IDB_DT_CB_192,
};

const UINT CMPCTheme::ThemeRadios[5] = {
    IDB_DT_RADIO_96,
    IDB_DT_RADIO_120,
    IDB_DT_RADIO_144,
    IDB_DT_RADIO_144,
    IDB_DT_RADIO_192,
};

const std::vector<CMPCTheme::pathPoint> CMPCTheme::minimizeIcon96 ({
    {2,6,newPath},
    {11,6,closePath},
});

const std::vector<CMPCTheme::pathPoint> CMPCTheme::minimizeIcon120 ({
    {3,7,newPath},
    {14,7,closePath},
});

const std::vector<CMPCTheme::pathPoint> CMPCTheme::minimizeIcon144 ({
    {4,9,newPath},
    {18,9,closePath},
});

//same size as 144, but centered better
const std::vector<CMPCTheme::pathPoint> CMPCTheme::minimizeIcon168({
    {2,9,newPath},
    {16,9,closePath},
});

const std::vector<CMPCTheme::pathPoint> CMPCTheme::minimizeIcon192 ({
    {5.5,12.5,newPath},
    {23.5,12.5,closePath},
});

const std::vector<CMPCTheme::pathPoint> CMPCTheme::restoreIcon96({
    {2,4,newPath},
    {9,4,linePath},
    {9,11,linePath},
    {2,11,linePath},
    {2,4,linePath},
    {4,4,newPath},
    {4,2,linePath},
    {11,2,linePath},
    {11,9,linePath},
    {9,9,linePath}
});

const std::vector<CMPCTheme::pathPoint> CMPCTheme::restoreIcon120({
    {2,4,newPath},
    {11,4,linePath},
    {11,13,linePath},
    {2,13,linePath},
    {2,4,linePath},
    {4,4,newPath},
    {4,2,linePath},
    {13,2,linePath},
    {13,11,linePath},
    {11,11,linePath},
});

const std::vector<CMPCTheme::pathPoint> CMPCTheme::restoreIcon144({
    {2,5,newPath},
    {13,5,linePath},
    {13,16,linePath},
    {2,16,linePath},
    {2,5,linePath},
    {5,5,newPath},
    {5,2,linePath},
    {16,2,linePath},
    {16,13,linePath},
    {13,13,linePath},
});

const std::vector<CMPCTheme::pathPoint> CMPCTheme::restoreIcon168 = CMPCTheme::restoreIcon144;

const std::vector<CMPCTheme::pathPoint> CMPCTheme::restoreIcon192({
    { 3.5, 7.5, newPath},
    { 17.5,7.5,linePath },
    { 17.5,21.5,linePath },
    { 3.5,21.5,linePath },
    { 3.5,7.5,linePath },
    { 7.5,7.5,newPath },
    { 7.5,3.5,linePath },
    { 21.5,3.5,linePath },
    { 21.5,17.5,linePath },
    { 17.5,17.5,linePath },
});

const std::vector<CMPCTheme::pathPoint> CMPCTheme::maximizeIcon96({
    {1,1,newPath},
    {1,10,linePath},
    {10,10,linePath},
    {10,1,linePath},
    {1,1,linePath}
});

const std::vector<CMPCTheme::pathPoint> CMPCTheme::maximizeIcon120({
    {2,2,newPath},
    {2,13,linePath},
    {13,13,linePath},
    {13,2,linePath},
    {2,2,linePath},
});

const std::vector<CMPCTheme::pathPoint> CMPCTheme::maximizeIcon144({
    {2,2,newPath},
    {2,16,linePath},
    {16,16,linePath},
    {16,2,linePath},
    {2,2,linePath},
});

const std::vector<CMPCTheme::pathPoint> CMPCTheme::maximizeIcon168 = CMPCTheme::maximizeIcon144;

const std::vector<CMPCTheme::pathPoint> CMPCTheme::maximizeIcon192({
    {3.5,3.5,newPath},
    {3.5,21.5,linePath},
    {21.5,21.5,linePath},
    {21.5,3.5,linePath},
    {3.5,3.5,linePath},
});

const std::vector<CMPCTheme::pathPoint> CMPCTheme::closeIcon96({
    {1,1,newPath},
    {10,10,closePath},
    {1,10,newPath},
    {10,1,closePath}
});

const std::vector<CMPCTheme::pathPoint> CMPCTheme::closeIcon120({
    {2,2,newPath},
    {13,13,linePath},
    {2,13,newPath},
    {13,2,linePath},
});

const std::vector<CMPCTheme::pathPoint> CMPCTheme::closeIcon144({
    {2,2,newPath},
    {16,16,linePath},
    {2,16,newPath},
    {16,2,linePath},
});

const std::vector<CMPCTheme::pathPoint> CMPCTheme::closeIcon168 = CMPCTheme::closeIcon144;

const std::vector<CMPCTheme::pathPoint> CMPCTheme::closeIcon192({
    {3.5,3.5,newPath},
    {21.5,21.5,linePath},
    {3.5,21.5,newPath},
    {21.5,3.5,linePath},
});

//windows10 centers the icon "path" on the button, inside a frame
//sometimes this frame is centered, but at different dpis it's misaligned by 1-2 pixels
//we use the width/height of the frame to tweak the "center" position
const int CMPCTheme::W10TitlebarIconPathHeight[5] = {
    12,
    15, //should be 16, but to match windows 10
    18, //should be 19
    18, //should be 19
    26,
};

const int CMPCTheme::W10TitlebarIconPathWidth[5] = {
    12,
    17, //should be 16, but to match windows 10
    19,
    19,
    28,
};

const float CMPCTheme::W10TitlebarIconPathThickness[5] = {
    1,
    1,
    1,
    1,
    2,
};

const int CMPCTheme::W10TitlebarButtonWidth[5] = {
    45,
    58,
    69,
    80,
    91,
};

const int CMPCTheme::W10TitlebarButtonSpacing[5] = {
    1,
    1,
    2,
    1, //makes no sense, but spacing goes back to 1
    2,
};
