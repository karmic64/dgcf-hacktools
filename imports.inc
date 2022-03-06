
;;;;; WINMM.dll ;;;;;
waveOutClose equ $44A2F4
waveOutUnprepareHeader equ $44A2F8
waveOutReset equ $44A2FC
timeGetTime equ $44A300
mciGetErrorStringA equ $44A304
mciSendCommandA equ $44A308

;;;;; KERNEL32.dll ;;;;;
DeleteFileA equ $44A074
MultiByteToWideChar equ $44A078
GetModuleHandleA equ $44A07C
GetWindowsDirectoryA equ $44A080
GetLocalTime equ $44A084
WritePrivateProfileStringA equ $44A088
lstrlenA equ $44A08C
FreeLibrary equ $44A090
WinExec equ $44A094
GetVersion equ $44A098
GetLastError equ $44A09C
LockResource equ $44A0A0
FindResourceA equ $44A0A4
GlobalFree equ $44A0A8
GlobalUnlock equ $44A0AC
GetModuleFileNameA equ $44A0B0
SetUnhandledExceptionFilter equ $44A0B4
IsBadReadPtr equ $44A0B8
IsBadCodePtr equ $44A0BC
CompareStringA equ $44A0C0
CompareStringW equ $44A0C4
SetEnvironmentVariableA equ $44A0C8
LoadResource equ $44A0CC
OutputDebugStringA equ $44A0D0
TerminateThread equ $44A0D4
CreateThread equ $44A0D8
GetVersionExA equ $44A0DC
lstrcpyA equ $44A0E0
lstrcmpA equ $44A0E4
ReadFile equ $44A0E8
Sleep equ $44A0EC
GetFileSize equ $44A0F0
CreateFileA equ $44A0F4
WriteFile equ $44A0F8
SetFilePointer equ $44A0FC
LCMapStringA equ $44A100
CloseHandle equ $44A104
FlushFileBuffers equ $44A108
GetStringTypeW equ $44A10C
LCMapStringW equ $44A110
GetStringTypeA equ $44A114
CreateMutexA equ $44A118
SetStdHandle equ $44A11C
GetProcAddress equ $44A120
LoadLibraryA equ $44A124
ExitProcess equ $44A128
GetCurrentProcess equ $44A12C
RtlUnwind equ $44A130
GetTimeZoneInformation equ $44A134
GetSystemTime equ $44A138
GetStartupInfoA equ $44A13C
GetCommandLineA equ $44A140
HeapDestroy equ $44A144
HeapCreate equ $44A148
VirtualFree equ $44A14C
VirtualAlloc equ $44A150
HeapReAlloc equ $44A154
IsBadWritePtr equ $44A158
SetHandleCount equ $44A15C
GetStdHandle equ $44A160
GetFileType equ $44A164
HeapAlloc equ $44A168
TerminateProcess equ $44A16C
GetCPInfo equ $44A170
GetACP equ $44A174
GetOEMCP equ $44A178
UnhandledExceptionFilter equ $44A17C
FreeEnvironmentStringsA equ $44A180
FreeEnvironmentStringsW equ $44A184
WideCharToMultiByte equ $44A188
GetEnvironmentStrings equ $44A18C
HeapFree equ $44A190
GetEnvironmentStringsW equ $44A194

;;;;; USER32.dll ;;;;;
DialogBoxParamA equ $44A1A8
DefWindowProcA equ $44A1AC
SetMenu equ $44A1B0
DestroyWindow equ $44A1B4
SetRect equ $44A1B8
IsWindow equ $44A1BC
CheckDlgButton equ $44A1C0
GetDlgItem equ $44A1C4
IsDlgButtonChecked equ $44A1C8
RegisterWindowMessageA equ $44A1CC
SetDlgItemTextA equ $44A1D0
GetDlgItemTextA equ $44A1D4
GetSysColor equ $44A1D8
DrawTextA equ $44A1DC
CheckRadioButton equ $44A1E0
SendDlgItemMessageA equ $44A1E4
CallNextHookEx equ $44A1E8
GetFocus equ $44A1EC
DispatchMessageA equ $44A1F0
TranslateMessage equ $44A1F4
GetMessageA equ $44A1F8
PeekMessageA equ $44A1FC
SetWindowsHookExA equ $44A200
EndDialog equ $44A204
DrawMenuBar equ $44A208
SetCursorPos equ $44A20C
FrameRect equ $44A210
AppendMenuA equ $44A214
KillTimer equ $44A218
ShowWindow equ $44A21C
SendMessageA equ $44A220
EnableMenuItem equ $44A224
TrackPopupMenu equ $44A228
GetCursorPos equ $44A22C
ReleaseCapture equ $44A230
SetCapture equ $44A234
LoadImageA equ $44A238
GetClientRect equ $44A23C
ClientToScreen equ $44A240
ValidateRect equ $44A244
InvalidateRect equ $44A248
UpdateWindow equ $44A24C
LoadIconA equ $44A250
RegisterClassA equ $44A254
SetWindowTextA equ $44A258
PostMessageA equ $44A25C
GetDC equ $44A260
FillRect equ $44A264
ReleaseDC equ $44A268
LoadCursorA equ $44A26C
SetCursor equ $44A270
ShowCursor equ $44A274
GetMenuState equ $44A278
SetMenuItemInfoA equ $44A27C
GetWindowLongA equ $44A280
GetWindowTextA equ $44A284
GetWindowRect equ $44A288
ScreenToClient equ $44A28C
BeginPaint equ $44A290
EndPaint equ $44A294
DestroyMenu equ $44A298
PostQuitMessage equ $44A29C
EnumWindows equ $44A2A0
CreateMenu equ $44A2A4
CreatePopupMenu equ $44A2A8
CreateWindowExA equ $44A2AC
SetTimer equ $44A2B0
SystemParametersInfoA equ $44A2B4
SetWindowLongA equ $44A2B8
SetWindowPos equ $44A2BC
WINNLSEnableIME equ $44A2C0
ChangeDisplaySettingsA equ $44A2C4
EnumDisplaySettingsA equ $44A2C8
LoadStringA equ $44A2CC
ModifyMenuA equ $44A2D0
UnhookWindowsHookEx equ $44A2D4
CheckMenuItem equ $44A2D8
GetSystemMetrics equ $44A2DC
GetAsyncKeyState equ $44A2E0
MessageBoxA equ $44A2E4
wsprintfA equ $44A2E8
RemoveMenu equ $44A2EC

;;;;; GDI32.dll ;;;;;
DeleteObject equ $44A020
GetStockObject equ $44A024
SetBkMode equ $44A028
TextOutA equ $44A02C
SetTextColor equ $44A030
SelectObject equ $44A034
SetBkColor equ $44A038
GetGlyphOutlineA equ $44A03C
SetMapMode equ $44A040
CreateCompatibleDC equ $44A044
GetObjectA equ $44A048
GetDIBColorTable equ $44A04C
BitBlt equ $44A050
CreateDIBSection equ $44A054
GetDeviceCaps equ $44A058
CreateSolidBrush equ $44A05C
RealizePalette equ $44A060
SelectPalette equ $44A064
CreateFontIndirectA equ $44A068
DeleteDC equ $44A06C

;;;;; ADVAPI32.dll ;;;;;
RegCloseKey equ $44A000
RegQueryValueExA equ $44A004
RegOpenKeyExA equ $44A008

;;;;; SHELL32.dll ;;;;;
FindExecutableA equ $44A19C
ShellExecuteA equ $44A1A0

;;;;; ole32.dll ;;;;;
CoCreateInstance equ $44A34C
CoInitialize equ $44A350
CoUninitialize equ $44A354

;;;;; WSOCK32.dll ;;;;;
; (ignored ordinal import $80000009, VA $44A310)
; (ignored ordinal import $80000034, VA $44A314)
; (ignored ordinal import $8000000A, VA $44A318)
; (ignored ordinal import $80000017, VA $44A31C)
; (ignored ordinal import $80000073, VA $44A320)
; (ignored ordinal import $8000006F, VA $44A324)
; (ignored ordinal import $80000002, VA $44A328)
; (ignored ordinal import $80000015, VA $44A32C)
; (ignored ordinal import $8000000C, VA $44A330)
; (ignored ordinal import $80000039, VA $44A334)
; (ignored ordinal import $80000014, VA $44A338)
; (ignored ordinal import $80000074, VA $44A33C)
; (ignored ordinal import $80000003, VA $44A340)
; (ignored ordinal import $8000000B, VA $44A344)

;;;;; DDRAW.dll ;;;;;
DirectDrawCreate equ $44A010

;;;;; DSOUND.dll ;;;;;
; (ignored ordinal import $80000001, VA $44A018)
