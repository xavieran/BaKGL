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

#ifndef DIALOG_FACTORY_H
#define DIALOG_FACTORY_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "Dialog.h"
#include "PaletteResource.h"
#include "WidgetFactory.h"

class DialogFactory
{
private:
    RequestResource request;
    PaletteResource palette;
    ScreenResource screen;
    ImageResource normal;
    ImageResource pressed;
    ImageResource heads;
    ImageResource compass;
    ImageResource icons;
    ImageResource images;
    FontResource font;
    LabelResource label;
    WidgetFactory widgetFactory;
public:
    DialogFactory();
    virtual ~DialogFactory();
    Dialog* CreateCampDialog();
    Dialog* CreateCastDialog();
    Dialog* CreateContentsDialog();
    Dialog* CreateFullMapDialog();
    Dialog* CreateInfoDialog();
    Dialog* CreateInventoryDialog();
    Dialog* CreateLoadDialog();
    Dialog* CreateMapDialog();
    Dialog* CreateOptionsDialog ( const bool firstTime );
    Dialog* CreatePreferencesDialog();
    Dialog* CreateSaveDialog();
    Dialog* CreateWorldDialog();
};

#endif
