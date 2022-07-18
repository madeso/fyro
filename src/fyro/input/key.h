#pragma once

#include "fyro/undef_windows.h"

namespace input
{

enum class MouseButton
{
    invalid
    , unbound  /// No key
    , left  /// The left mouse button
    , middle  /// The middle mouse button
    , right  /// The right mouse button
    , x1  /// The X2 mouse button
    , x2  /// The X2 mouse button
};


enum class GamecontrollerButton
{
    invalid
    , a
    , b
    , x
    , y
    , back
    , guide
    , start
    , stick_left
    , stick_right
    , shoulder_left
    , shoulder_right
    , dpad_up
    , dpad_down
    , dpad_left
    , dpad_right
    , misc1
    , paddle1
    , paddle2
    , paddle3
    , paddle4
    , touchpad
    // sdl axis
    , trigger_left
    , trigger_right
};


enum class KeyboardKey
{
    invalid
    , unbound  /// No key
    , return_key  /// The key for Return
    , escape  /// The key for Escape
    , backspace  /// The key for Backspace
    , tab  /// The key for Tab
    , space  /// The key for Space
    , exclaim  /// The key for Exclaim
    , quotedbl  /// The key for Quotedbl
    , hash  /// The key for Hash
    , percent  /// The key for Percent
    , dollar  /// The key for Dollar
    , ampersand  /// The key for Ampersand
    , quote  /// The key for Quote
    , left_paren  /// The key for Leftparen
    , right_paren  /// The key for Rightparen
    , asterix  /// The key for Asterisk
    , plus  /// The key for Plus
    , comma  /// The key for Comma
    , minus  /// The key for Minus
    , period  /// The key for Period
    , slash  /// The key for Slash
    , num_0  /// The key for Num0
    , num_1  /// The key for Num1
    , num_2  /// The key for Num2
    , num_3  /// The key for Num3
    , num_4  /// The key for Num4
    , num_5  /// The key for Num5
    , num_6  /// The key for Num6
    , num_7  /// The key for Num7
    , num_8  /// The key for Num8
    , num_9  /// The key for Num9
    , colon  /// The key for Colon
    , semicolon  /// The key for Semicolon
    , less  /// The key for Less
    , equals  /// The key for Equals
    , greater  /// The key for Greater
    , question  /// The key for Question
    , at  /// The key for At
    , left_bracket  /// The key for Leftbracket
    , backslash  /// The key for Backslash
    , right_bracket  /// The key for Rightbracket
    , caret  /// The key for Caret
    , underscore  /// The key for Underscore
    , backquote  /// The key for Backquote
    , a  /// The key for A
    , b  /// The key for B
    , c  /// The key for C
    , d  /// The key for D
    , e  /// The key for E
    , f  /// The key for F
    , g  /// The key for G
    , h  /// The key for H
    , i  /// The key for I
    , j  /// The key for J
    , k  /// The key for K
    , l  /// The key for L
    , m  /// The key for M
    , n  /// The key for N
    , o  /// The key for O
    , p  /// The key for P
    , q  /// The key for Q
    , r  /// The key for R
    , s  /// The key for S
    , t  /// The key for T
    , u  /// The key for U
    , v  /// The key for V
    , w  /// The key for W
    , x  /// The key for X
    , y  /// The key for Y
    , z  /// The key for Z
    , capslock  /// The key for Capslock
    , f1  /// The key for F1
    , f2  /// The key for F2
    , f3  /// The key for F3
    , f4  /// The key for F4
    , f5  /// The key for F5
    , f6  /// The key for F6
    , f7  /// The key for F7
    , f8  /// The key for F8
    , f9  /// The key for F9
    , f10  /// The key for F10
    , f11  /// The key for F11
    , f12  /// The key for F12
    , printscreen  /// The key for Printscreen
    , scrolllock  /// The key for Scrolllock
    , pause  /// The key for Pause
    , insert  /// The key for Insert
    , home  /// The key for Home
    , pageup  /// The key for Pageup
    , delete_key  /// The key for Delete
    , end  /// The key for End
    , pagedown  /// The key for Pagedown
    , right  /// The key for Right
    , left  /// The key for Left
    , down  /// The key for Down
    , up  /// The key for Up
    , numlock_clear  /// The key for Numlockclear
    , keypad_divide  /// The key for Kp_Divide
    , keypad_multiply  /// The key for Kp_Multiply
    , keypad_minus  /// The key for Kp_Minus
    , keypad_plus  /// The key for Kp_Plus
    , keypad_enter  /// The key for Kp_Enter
    , keypad_1  /// The key for Kp_1
    , keypad_2  /// The key for Kp_2
    , keypad_3  /// The key for Kp_3
    , keypad_4  /// The key for Kp_4
    , keypad_5  /// The key for Kp_5
    , keypad_6  /// The key for Kp_6
    , keypad_7  /// The key for Kp_7
    , keypad_8  /// The key for Kp_8
    , keypad_9  /// The key for Kp_9
    , keypad_0  /// The key for Kp_0
    , keypad_period  /// The key for Kp_Period
    , application  /// The key for Application
    , power  /// The key for Power
    , keypad_equals  /// The key for Kp_Equals
    , f13  /// The key for F13
    , f14  /// The key for F14
    , f15  /// The key for F15
    , f16  /// The key for F16
    , f17  /// The key for F17
    , f18  /// The key for F18
    , f19  /// The key for F19
    , f20  /// The key for F20
    , f21  /// The key for F21
    , f22  /// The key for F22
    , f23  /// The key for F23
    , f24  /// The key for F24
    , execute  /// The key for Execute
    , help  /// The key for Help
    , menu  /// The key for Menu
    , select  /// The key for Select
    , stop  /// The key for Stop
    , again  /// The key for Again
    , undo  /// The key for Undo
    , cut  /// The key for Cut
    , copy  /// The key for Copy
    , paste  /// The key for Paste
    , find  /// The key for Find
    , mute  /// The key for Mute
    , volume_up  /// The key for Volumeup
    , volume_down  /// The key for Volumedown
    , keypad_comma  /// The key for Kp_Comma
    , keypad_equals_as_400  /// The key for Kp_Equalsas400
    , alt_erase  /// The key for Alterase
    , sysreq  /// The key for Sysreq
    , cancel  /// The key for Cancel
    , clear  /// The key for Clear
    , prior  /// The key for Prior
    , second_return  /// The key for Return2
    , separator  /// The key for Separator
    , out  /// The key for Out
    , oper  /// The key for Oper
    , clear_again  /// The key for Clearagain
    , crsel  /// The key for Crsel
    , exsel  /// The key for Exsel
    , keypad_00  /// The key for Kp_00
    , keypad_000  /// The key for Kp_000
    , thousand_separator  /// The key for Thousandsseparator
    , decimal_separator  /// The key for Decimalseparator
    , currency_unit  /// The key for Currencyunit
    , currency_subunit  /// The key for Currencysubunit
    , keypad_leftparen  /// The key for Kp_Leftparen
    , keypad_rightparen  /// The key for Kp_Rightparen
    , keypad_leftbrace  /// The key for Kp_Leftbrace
    , keypad_rightbrace  /// The key for Kp_Rightbrace
    , keypad_tab  /// The key for Kp_Tab
    , keypad_backspace  /// The key for Kp_Backspace
    , keypad_a  /// The key for Kp_A
    , keypad_b  /// The key for Kp_B
    , keypad_c  /// The key for Kp_C
    , keypad_d  /// The key for Kp_D
    , keypad_e  /// The key for Kp_E
    , keypad_f  /// The key for Kp_F
    , keypad_xor  /// The key for Kp_Xor
    , keypad_power  /// The key for Kp_Power
    , keypad_percent  /// The key for Kp_Percent
    , keypad_less  /// The key for Kp_Less
    , keypad_greater  /// The key for Kp_Greater
    , keypad_ampersand  /// The key for Kp_Ampersand
    , keypad_double_ampersand  /// The key for Kp_Dblampersand
    , keypad_vertical_bar  /// The key for Kp_Verticalbar
    , keypad_double_verticle_bar  /// The key for Kp_Dblverticalbar
    , keypad_colon  /// The key for Kp_Colon
    , keypad_hash  /// The key for Kp_Hash
    , keypad_space  /// The key for Kp_Space
    , keypad_at  /// The key for Kp_At
    , keypad_exclam  /// The key for Kp_Exclam
    , keypad_mem_store  /// The key for Kp_Memstore
    , keypad_mem_recall  /// The key for Kp_Memrecall
    , keypad_mem_clear  /// The key for Kp_Memclear
    , keypad_mem_add  /// The key for Kp_Memadd
    , keypad_mem_subtract  /// The key for Kp_Memsubtract
    , keypad_mem_multiply  /// The key for Kp_Memmultiply
    , keypad_mem_divide  /// The key for Kp_Memdivide
    , keypad_plus_minus  /// The key for Kp_Plusminus
    , keypad_clear  /// The key for Kp_Clear
    , keypad_clear_entry  /// The key for Kp_Clearentry
    , keypad_binary  /// The key for Kp_Binary
    , keypad_octal  /// The key for Kp_Octal
    , keypad_decimal  /// The key for Kp_Decimal
    , keypad_hexadecimal  /// The key for Kp_Hexadecimal
    , ctrl_left  /// The key for Lctrl
    , shift_left  /// The key for Lshift
    , alt_left  /// The key for Lalt
    , gui_left  /// The key for Lgui
    , ctrl_right  /// The key for Rctrl
    , shift_right  /// The key for Rshift
    , alt_right  /// The key for Ralt
    , gui_right  /// The key for Rgui
    , mode  /// The key for Mode
    , audio_next  /// The key for Audionext
    , audio_prev  /// The key for Audioprev
    , audio_stop  /// The key for Audiostop
    , audio_play  /// The key for Audioplay
    , audio_mute  /// The key for Audiomute
    , mediaselect  /// The key for Mediaselect
    , www  /// The key for Www
    , mail  /// The key for Mail
    , calculator  /// The key for Calculator
    , computer  /// The key for Computer
    , ac_search  /// The key for Ac_Search
    , ac_home  /// The key for Ac_Home
    , ac_back  /// The key for Ac_Back
    , ac_forward  /// The key for Ac_Forward
    , ac_stop  /// The key for Ac_Stop
    , ac_refresh  /// The key for Ac_Refresh
    , ac_bookmarks  /// The key for Ac_Bookmarks
    , brightness_down  /// The key for Brightnessdown
    , brightness_up  /// The key for Brightnessup
    , displayswitch  /// The key for Displayswitch
    , keyboard_illum_toggle  /// The key for Kbdillumtoggle
    , keyboard_illum_down  /// The key for Kbdillumdown
    , keyboard_illum_up  /// The key for Kbdillumup
    , eject  /// The key for Eject
    , sleep  /// The key for Sleep
};


}
