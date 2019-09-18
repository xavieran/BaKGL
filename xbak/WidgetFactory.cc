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

#include "Exception.h"
#include "ObjectResource.h"
#include "WidgetFactory.h"

WidgetFactory::WidgetFactory()
{}

WidgetFactory::~WidgetFactory()
{}

TextButtonWidget*
WidgetFactory::CreateTextButton(RequestData& data, FontResource &fnt, ActionEventListener *ael)
{
    TextButtonWidget *button = new TextButtonWidget(Rectangle(data.xpos, data.ypos, data.width, data.height), data.action);
    button->SetVisible(data.visible);
    button->SetLabel(data.label, fnt.GetFont());
    button->AddActionListener(ael);
    return button;
}

ImageButtonWidget*
WidgetFactory::CreateImageButton(RequestData& data, ImageResource& normal, ImageResource& pressed, ActionEventListener *ael)
{
    ImageButtonWidget *button = new ImageButtonWidget(Rectangle(data.xpos, data.ypos, data.width, data.height), data.action);
    button->SetVisible(data.visible);
    Image *normalImage = 0;
    Image *pressedImage = 0;
    if (data.image >= 0)
    {
        normalImage = normal.GetImage(data.image);
        pressedImage = pressed.GetImage(data.image);
    }
    button->SetImage(normalImage, pressedImage);
    button->AddActionListener(ael);
    return button;
}

CharacterButtonWidget*
WidgetFactory::CreateCharacterButton(RequestData& data, PlayerCharacter *pc, ImageResource& img, ActionEventListener *ael)
{
    CharacterButtonWidget *button = new CharacterButtonWidget(Rectangle(data.xpos, data.ypos, data.width, data.height), data.action);
    button->SetCharacter(pc);
    button->SetImage(img.GetImage(SELECTED_IMAGE));
    button->AddActionListener(ael);
    return button;
}

ChoiceWidget*
WidgetFactory::CreateChoice(RequestData& data, ImageResource& img, ActionEventListener *ael)
{
    ChoiceWidget *choice = new ChoiceWidget(Rectangle(data.xpos, data.ypos, data.width, data.height), data.action);
    choice->SetImage(img.GetImage(data.image + 1), img.GetImage(data.image));
    choice->AddActionListener(ael);
    return choice;
}

CompassWidget*
WidgetFactory::CreateCompass(Camera *cam, Image *img)
{
    CompassWidget *compass = new CompassWidget(cam, img);
    return compass;
}

CombatViewWidget*
WidgetFactory::CreateCombatView(RequestData& data, Game *game)
{
    return new CombatViewWidget(Rectangle(data.xpos, data.ypos, data.width, data.height), game);
}

MapViewWidget*
WidgetFactory::CreateMapView(RequestData& data, Game *game)
{
    return new MapViewWidget(Rectangle(data.xpos, data.ypos, data.width, data.height), game);
}

WorldViewWidget*
WidgetFactory::CreateWorldView(RequestData& data, Game *game)
{
    return new WorldViewWidget(Rectangle(data.xpos, data.ypos, data.width, data.height), game);
}

TextWidget*
WidgetFactory::CreateLabel(LabelData& data, FontResource& fnt, const int panelWidth)
{
    unsigned int width = 1;
    switch (data.type)
    {
    case LBL_STANDARD:
        for (unsigned int i = 0; i < data.label.length(); i++)
        {
            width += fnt.GetFont()->GetWidth((unsigned int)data.label[i] - fnt.GetFont()->GetFirst());
        }
        break;
    case LBL_TITLE:
        width = panelWidth;
        break;
    default:
        break;
    }
    TextWidget *label = new TextWidget(Rectangle(data.xpos, data.ypos, width, fnt.GetFont()->GetHeight() + 1), fnt.GetFont());
    label->SetText(data.label);
    label->SetColor(data.color);
    if (data.type == LBL_TITLE)
    {
        label->SetShadow(data.shadow, 0, 1);
        label->SetAlignment(HA_CENTER, VA_TOP);
    }
    return label;
}

BadgeWidget*
WidgetFactory::CreateBadge(const Rectangle &r, const std::string& s, Font *f)
{
    BadgeWidget *badge = new BadgeWidget(r);
    badge->SetLabel(s, f);
    return badge;
}

ImageWidget*
WidgetFactory::CreateImage(const Rectangle &r, Image *img, const Flipping flip)
{
    ImageWidget *image = new ImageWidget(r, img);
    switch (flip)
    {
    case HORIZONTAL:
        image->HorizontalFlip();
        break;
    case VERTICAL:
        image->VerticalFlip();
        break;
    default:
        break;
    }
    return image;
}

InventoryItemWidget*
WidgetFactory::CreateInventoryItem(const Rectangle &r, const int a, InventoryItem *item, Image *img, const std::string& s, Font *f, ActionEventListener *ael)
{
    InventoryItemWidget* invitem = new InventoryItemWidget(r, a);
    invitem->SetDraggable(true);
    invitem->SetInventoryItem(item);
    invitem->SetImage(img);
    invitem->SetLabel(s, f);
    invitem->AddActionListener(ael);
    return invitem;
}

InventoryWidget *
WidgetFactory::CreateInventory(const Rectangle &r, PlayerCharacter *pc, ImageResource& img, FontResource& fnt, ActionEventListener *ael)
{
    InventoryWidget *inv = new InventoryWidget(r, pc, img, fnt.GetFont());
    inv->SetDraggable(true);
    inv->AddActionListener(ael);
    return inv;
}

EquipmentItemWidget*
WidgetFactory::CreateEquipmentItem(const Rectangle &r, const ObjectType t, InventoryItem *item, Image *img, const std::string& s, Font *f)
{
    EquipmentItemWidget* eqitem = new EquipmentItemWidget(r, t);
    eqitem->SetInventoryItem(item);
    eqitem->SetImage(img);
    eqitem->SetLabel(s, f);
    return eqitem;
}

EquipmentWidget *
WidgetFactory::CreateEquipment(const Rectangle &r, PlayerCharacter *pc, ImageResource& img, ImageResource& slot, FontResource& fnt)
{
    return new EquipmentWidget(r, pc, img, slot.GetImage(11), slot.GetImage(10), fnt.GetFont());
}

PortraitWidget*
WidgetFactory::CreatePortrait(const Rectangle &r, PlayerCharacter *pc, Image *hb, Image *vb)
{
    PortraitWidget *portrait = new PortraitWidget(r, pc);
    portrait->SetBorders(hb, vb);
    return portrait;
}

RatingsWidget*
WidgetFactory::CreateRatings(const Rectangle &r, PlayerCharacter *pc, Image *hb, Image *vb, Font *f)
{
    RatingsWidget *ratings = new RatingsWidget(r, pc, f);
    ratings->SetBorders(hb, vb);
    return ratings;
}

SkillsWidget*
WidgetFactory::CreateSkills(const Rectangle &r, PlayerCharacter *pc, Image *sw, Image *bl, Font *f)
{
    SkillsWidget *skills = new SkillsWidget(r, pc, sw, bl, f);
    return skills;
}

PanelWidget*
WidgetFactory::CreatePanel(const Rectangle &r, Image *img)
{
    PanelWidget *panel = new PanelWidget(r);
    panel->SetBackground(img);
    return panel;
}
