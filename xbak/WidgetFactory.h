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

#ifndef WIDGET_FACTORY_H
#define WIDGET_FACTORY_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "BadgeWidget.h"
#include "CharacterButtonWidget.h"
#include "ChoiceWidget.h"
#include "CombatViewWidget.h"
#include "CompassWidget.h"
#include "EquipmentItemWidget.h"
#include "EquipmentWidget.h"
#include "FontResource.h"
#include "ImageButtonWidget.h"
#include "ImageWidget.h"
#include "InventoryItemWidget.h"
#include "InventoryWidget.h"
#include "LabelResource.h"
#include "MapViewWidget.h"
#include "PanelWidget.h"
#include "PortraitWidget.h"
#include "RatingsWidget.h"
#include "RequestResource.h"
#include "ScreenResource.h"
#include "SkillsWidget.h"
#include "TextButtonWidget.h"
#include "WorldViewWidget.h"

class WidgetFactory
{
public:
    WidgetFactory();
    virtual ~WidgetFactory();
    TextButtonWidget* CreateTextButton ( RequestData& data, FontResource& fnt, ActionEventListener *ael );
    ImageButtonWidget* CreateImageButton ( RequestData& data, ImageResource& normal, ImageResource& pressed, ActionEventListener *ael );
    CharacterButtonWidget* CreateCharacterButton ( RequestData& data, PlayerCharacter *pc, ImageResource& img, ActionEventListener *ael );
    ChoiceWidget* CreateChoice ( RequestData& data, ImageResource& img, ActionEventListener *ael );
    CompassWidget* CreateCompass ( Camera *cam, Image *img );
    CombatViewWidget* CreateCombatView ( RequestData& data, Game *game );
    MapViewWidget* CreateMapView ( RequestData& data, Game *game );
    WorldViewWidget* CreateWorldView ( RequestData& data, Game *game );
    TextWidget* CreateLabel ( LabelData& data, FontResource& fnt, const int panelWidth );
    BadgeWidget* CreateBadge ( const Rectangle &r, const std::string& s, Font *f );
    ImageWidget* CreateImage ( const Rectangle &r, Image *img, const Flipping flip = NONE );
    InventoryItemWidget* CreateInventoryItem ( const Rectangle &r, const int a, InventoryItem *item, Image *img, const std::string& s, Font *f, ActionEventListener *ael );
    InventoryWidget *CreateInventory ( const Rectangle &r, PlayerCharacter *pc, ImageResource& img, FontResource& fnt, ActionEventListener *ael );
    EquipmentItemWidget* CreateEquipmentItem ( const Rectangle &r, const ObjectType t, InventoryItem *item, Image *img, const std::string& s, Font *f );
    EquipmentWidget *CreateEquipment ( const Rectangle &r, PlayerCharacter *pc, ImageResource& img, ImageResource& slot, FontResource& fnt );
    PortraitWidget* CreatePortrait ( const Rectangle &r, PlayerCharacter *pc, Image *hb, Image *vb );
    RatingsWidget* CreateRatings ( const Rectangle &r, PlayerCharacter *pc, Image *hb, Image *vb, Font *f );
    SkillsWidget* CreateSkills ( const Rectangle &r, PlayerCharacter *pc, Image *sw, Image *bl, Font *f );
    PanelWidget* CreatePanel ( const Rectangle &r, Image *img );
};

#endif

