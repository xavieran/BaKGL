/*
 * This file is part of xBaK.
 *
 * xBaK is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * xBaK is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with xBaK.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Copyright (C) Guido de Jong <guidoj@users.sf.net>
 */

#ifndef EVENT_H
#define EVENT_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "InventoryItem.h"

/* Enum's heavily based on SDLKey and SDLMod */
typedef enum
{
    KEY_BACKSPACE    = 8,
    KEY_TAB          = 9,
    KEY_CLEAR        = 12,
    KEY_RETURN       = 13,
    KEY_ESCAPE       = 27,
    KEY_SPACE        = 32,
    KEY_EXCLAIM      = 33,
    KEY_QUOTEDBL     = 34,
    KEY_HASH         = 35,
    KEY_DOLLAR       = 36,
    KEY_AMPERSAND    = 38,
    KEY_QUOTE        = 39,
    KEY_LEFTPAREN    = 40,
    KEY_RIGHTPAREN   = 41,
    KEY_ASTERISK     = 42,
    KEY_PLUS         = 43,
    KEY_COMMA        = 44,
    KEY_MINUS        = 45,
    KEY_PERIOD       = 46,
    KEY_SLASH        = 47,
    KEY_0            = 48,
    KEY_1            = 49,
    KEY_2            = 50,
    KEY_3            = 51,
    KEY_4            = 52,
    KEY_5            = 53,
    KEY_6            = 54,
    KEY_7            = 55,
    KEY_8            = 56,
    KEY_9            = 57,
    KEY_COLON        = 58,
    KEY_SEMICOLON    = 59,
    KEY_LESS         = 60,
    KEY_EQUALS       = 61,
    KEY_GREATER      = 62,
    KEY_QUESTION     = 63,
    KEY_AT           = 64,

    KEY_LEFTBRACKET  = 91,
    KEY_BACKSLASH    = 92,
    KEY_RIGHTBRACKET = 93,
    KEY_CARET        = 94,
    KEY_UNDERSCORE   = 95,
    KEY_BACKQUOTE    = 96,
    KEY_a            = 97,
    KEY_b            = 98,
    KEY_c            = 99,
    KEY_d            = 100,
    KEY_e            = 101,
    KEY_f            = 102,
    KEY_g            = 103,
    KEY_h            = 104,
    KEY_i            = 105,
    KEY_j            = 106,
    KEY_k            = 107,
    KEY_l            = 108,
    KEY_m            = 109,
    KEY_n            = 110,
    KEY_o            = 111,
    KEY_p            = 112,
    KEY_q            = 113,
    KEY_r            = 114,
    KEY_s            = 115,
    KEY_t            = 116,
    KEY_u            = 117,
    KEY_v            = 118,
    KEY_w            = 119,
    KEY_x            = 120,
    KEY_y            = 121,
    KEY_z            = 122,
    KEY_DELETE       = 127,

    /* Numeric keypad */
    KEY_KP0          = 256,
    KEY_KP1          = 257,
    KEY_KP2          = 258,
    KEY_KP3          = 259,
    KEY_KP4          = 260,
    KEY_KP5          = 261,
    KEY_KP6          = 262,
    KEY_KP7          = 263,
    KEY_KP8          = 264,
    KEY_KP9          = 265,
    KEY_KP_PERIOD    = 266,
    KEY_KP_DIVIDE    = 267,
    KEY_KP_MULTIPLY  = 268,
    KEY_KP_MINUS     = 269,
    KEY_KP_PLUS      = 270,
    KEY_KP_ENTER     = 271,
    KEY_KP_EQUALS    = 272,

    /* Arrows + Home/End pad */
    KEY_UP           = 273,
    KEY_DOWN         = 274,
    KEY_RIGHT        = 275,
    KEY_LEFT         = 276,
    KEY_INSERT       = 277,
    KEY_HOME         = 278,
    KEY_END          = 279,
    KEY_PAGEUP       = 280,
    KEY_PAGEDOWN     = 281,

    /* Function keys */
    KEY_F1           = 282,
    KEY_F2           = 283,
    KEY_F3           = 284,
    KEY_F4           = 285,
    KEY_F5           = 286,
    KEY_F6           = 287,
    KEY_F7           = 288,
    KEY_F8           = 289,
    KEY_F9           = 290,
    KEY_F10          = 291,
    KEY_F11          = 292,
    KEY_F12          = 293,

    /* Key state modifier keys */
    KEY_NUMLOCK      = 300,
    KEY_CAPSLOCK     = 301,
    KEY_SCROLLOCK    = 302,
    KEY_RSHIFT       = 303,
    KEY_LSHIFT       = 304,
    KEY_RCTRL        = 305,
    KEY_LCTRL        = 306,
    KEY_RALT         = 307,
    KEY_LALT         = 308

} Key;

/* Keyboard modifiers */
const int MOD_NONE   = 0x0000;

const int MOD_LSHIFT = 0x0001;
const int MOD_RSHIFT = 0x0002;
const int MOD_LCTRL  = 0x0040;
const int MOD_RCTRL  = 0x0080;
const int MOD_LALT   = 0x0100;
const int MOD_RALT   = 0x0200;

const int MOD_NUM    = 0x1000;
const int MOD_CAPS   = 0x2000;

const int MOD_CTRL   = MOD_LCTRL | MOD_RCTRL;
const int MOD_SHIFT  = MOD_LSHIFT | MOD_RSHIFT;
const int MOD_ALT    = MOD_LALT | MOD_RALT;

/* Timer ID's */
const unsigned long TMR_TEST_APP     = 1;
const unsigned long TMR_GAME_APP     = 2;
const unsigned long TMR_CHAPTER      = 3;
const unsigned long TMR_MOVIE_PLAYER = 4;
const unsigned long TMR_PALETTE      = 5;
const unsigned long TMR_MOVING       = 6;
const unsigned long TMR_TURNING      = 7;


typedef enum
{
    PB_PRIMARY,
    PB_SECONDARY,
    PB_TERTIARY
}
PointerButton;


class KeyboardEvent
{
private:
    Key key;
public:
    KeyboardEvent ( const Key k );
    virtual ~KeyboardEvent();
    Key GetKey() const;
};

class PointerButtonEvent
{
private:
    PointerButton button;
    int xpos;
    int ypos;
public:
    PointerButtonEvent ( const PointerButton b, const int x, const int y );
    virtual ~PointerButtonEvent();
    PointerButton GetButton() const;
    int GetXPos() const;
    int GetYPos() const;
};

class PointerMotionEvent
{
private:
    int xpos;
    int ypos;
public:
    PointerMotionEvent ( const int x, const int y );
    virtual ~PointerMotionEvent();
    int GetXPos() const;
    int GetYPos() const;
};

class TimerEvent
{
private:
    unsigned long id;
public:
    TimerEvent ( const unsigned long n );
    virtual ~TimerEvent();
    unsigned long GetID() const;
};

class LoopEvent
{
private:
    int ticks;
public:
    LoopEvent ( const int t );
    virtual ~LoopEvent();
    int GetTicks() const;
};

class ActionEvent
{
private:
    int action;
    int xpos;
    int ypos;
public:
    ActionEvent ( const int a, const int x, const int y );
    virtual ~ActionEvent();
    int GetAction() const;
    int GetXPos() const;
    int GetYPos() const;
};

class DragEvent
{
private:
    bool toggle;
    int xpos;
    int ypos;
public:
    DragEvent ( const bool t, const int x, const int y );
    virtual ~DragEvent();
    bool GetToggle() const;
    int GetXPos() const;
    int GetYPos() const;
};

#endif
