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

#ifndef REQUEST_RESOURCE_H
#define REQUEST_RESOURCE_H

#include <vector>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "Geometry.h"
#include "ResourceData.h"

const unsigned int REQ_USERDEFINED   = 0;
const unsigned int REQ_IMAGEBUTTON   = 3;
const unsigned int REQ_SELECT        = 4;
const unsigned int REQ_TEXTBUTTON    = 6;

const unsigned int ACT_ESCAPE        = 1024;
const unsigned int ACT_STOP          = 1025;
const unsigned int ACT_UP            = 1026;
const unsigned int ACT_DOWN          = 1027;
const unsigned int ACT_LEFT          = 1028;
const unsigned int ACT_RIGHT         = 1029;

const unsigned int CAMP_EXIT         = 192;
const unsigned int CAMP_UNTIL_HEALED = 193;
const unsigned int CAMP_STOP         = 194;

const unsigned int CAST_TRIANGLE     = 7;
const unsigned int CAST_SQUARE       = 6;
const unsigned int CAST_CAST         = 2;
const unsigned int CAST_BOOKMARK     = 3;
const unsigned int CAST_CAMP1        = 4;
const unsigned int CAST_CAMP2        = 5;
const unsigned int CAST_EXIT         = 1;
const unsigned int CAST_MEMBER1      = 128;
const unsigned int CAST_MEMBER2      = 129;
const unsigned int CAST_MEMBER3      = 130;

const unsigned int CONT_EXIT         = 1;

const unsigned int FMAP_EXIT         = 18;

const unsigned int INV_MEMBER1       = 2;
const unsigned int INV_MEMBER2       = 3;
const unsigned int INV_MEMBER3       = 4;
const unsigned int INV_UNKNOWN       = 22;
const unsigned int INV_EXIT          = 1;
const unsigned int INV_MORE_INFO     = 57;

const unsigned int LOAD_RESTORE      = 193;
const unsigned int LOAD_CANCEL       = 192;

const unsigned int MAIN_LEFT         = 75;
const unsigned int MAIN_UP           = 72;
const unsigned int MAIN_DOWN         = 80;
const unsigned int MAIN_RIGHT        = 77;
const unsigned int MAIN_UNKNOWN      = 19;
const unsigned int MAIN_MAP          = 50;
const unsigned int MAIN_CAST         = 46;
const unsigned int MAIN_BOOKMARK     = 48;
const unsigned int MAIN_CAMP         = 18;
const unsigned int MAIN_OPTIONS      = 24;
const unsigned int MAIN_MEMBER1      = 2;
const unsigned int MAIN_MEMBER2      = 3;
const unsigned int MAIN_MEMBER3      = 4;

const unsigned int MAP_LEFT          = 75;
const unsigned int MAP_UP            = 72;
const unsigned int MAP_DOWN          = 80;
const unsigned int MAP_RIGHT         = 77;
const unsigned int MAP_UNKNOWN       = 19;
const unsigned int MAP_FULLMAP       = 33;
const unsigned int MAP_ZOOMOUT       = 73;
const unsigned int MAP_ZOOMIN        = 81;
const unsigned int MAP_CAMP          = 18;
const unsigned int MAP_MAIN          = 50;
const unsigned int MAP_MEMBER1       = 2;
const unsigned int MAP_MEMBER2       = 3;
const unsigned int MAP_MEMBER3       = 4;

const unsigned int OPT_CANCEL        = 18;
const unsigned int OPT_CONTENTS      = 46;
const unsigned int OPT_NEW_GAME      = 49;
const unsigned int OPT_PREFERENCES   = 25;
const unsigned int OPT_QUIT          = 32;
const unsigned int OPT_RESTORE       = 19;
const unsigned int OPT_SAVE          = 31;

const unsigned int INFO_EXIT         = 1;
const unsigned int INFO_SPELLS       = 31;

const unsigned int PREF_CANCEL       = 46;
const unsigned int PREF_DEFAULTS     = 32;
const unsigned int PREF_OK           = 24;
const unsigned int PREF_STEP_SMALL   = 192;
const unsigned int PREF_STEP_MEDIUM  = 193;
const unsigned int PREF_STEP_LARGE   = 194;
const unsigned int PREF_TURN_SMALL   = 195;
const unsigned int PREF_TURN_MEDIUM  = 196;
const unsigned int PREF_TURN_LARGE   = 197;
const unsigned int PREF_DETAIL_MIN   = 198;
const unsigned int PREF_DETAIL_LOW   = 199;
const unsigned int PREF_DETAIL_HIGH  = 200;
const unsigned int PREF_DETAIL_MAX   = 201;
const unsigned int PREF_TEXT_WAIT    = 202;
const unsigned int PREF_TEXT_MEDIUM  = 203;
const unsigned int PREF_TEXT_FAST    = 204;
const unsigned int PREF_SOUND        = 205;
const unsigned int PREF_MUSIC        = 206;
const unsigned int PREF_COMBAT_MUSIC = 207;
const unsigned int PREF_INTRODUCTION = 208;
const unsigned int PREF_CD_MUSIC     = 209;

const unsigned int SAVE_REMOVE_DIR   = 195;
const unsigned int SAVE_REMOVE_GAME  = 194;
const unsigned int SAVE_SAVE         = 193;
const unsigned int SAVE_CANCEL       = 192;

const int GROUP0 = 0;
const int GROUP1 = 1;
const int GROUP2 = 2;
const int GROUP3 = 3;

struct RequestData
{
    unsigned int widget;
    int action;
    bool visible;
    int xpos;
    int ypos;
    int width;
    int height;
    int teleport;
    int image;
    int group;
    std::string label;
};

class RequestResource
            : public ResourceData
{
private:
    bool popup;
    Rectangle rect;
    int xoff;
    int yoff;
    std::vector<RequestData> data;
public:
    RequestResource();
    virtual ~RequestResource();
    bool IsPopup() const;
    Rectangle& GetRectangle();
    int GetXOff() const;
    int GetYOff() const;
    unsigned int GetSize() const;
    RequestData GetRequestData ( const unsigned int n ) const;
    void Clear();
    void Load ( FileBuffer *buffer );
    unsigned int Save ( FileBuffer *buffer );
};

#endif
