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

#include <cstring>

#include "Exception.h"
#include "Palette.h"

Palette::Palette(const unsigned int n)
        : size(n)
{
    colors = new Color[size];
    media = MediaToolkit::GetInstance();
}

Palette::~Palette()
{
    if (colors)
    {
        delete[] colors;
    }
}

unsigned int
Palette::GetSize() const
{
    return size;
}

Color&
Palette::GetColor(const unsigned int i) const
{
    return colors[i];
}

void
Palette::SetColor(const unsigned int i, const Color &c)
{
    memcpy(&(colors[i]), &c, sizeof(Color));
}

void
Palette::Fill()
{
    if (colors)
    {
        delete[] colors;
    }
    size = WINDOW_COLORS;
    colors = new Color[size];
    memset(colors, 0, size * sizeof(Color));
    colors[0].r = 0;
    colors[0].g = 0;
    colors[0].b = 0;
    colors[0].a = 0;
    colors[15].r = 255;
    colors[15].g = 255;
    colors[15].b = 255;
    colors[15].a = 0;
}

void
Palette::Activate(const unsigned int first, const unsigned int n)
{
    media->GetVideo()->SetPalette(colors, first, n);
}

void
Palette::Retrieve(const unsigned int first, const unsigned int n)
{
    if ((colors != 0) && (size < (first + n)))
    {
        delete[] colors;
        colors = 0;
    }
    if (!colors)
    {
        size = first + n;
        colors = new Color[size];
    }
    media->GetVideo()->GetPalette(colors, first, n);
}

void
Palette::FadeFrom(Color* from, const unsigned int first, const unsigned int n, const unsigned int steps, const unsigned int delay)
{
    Color* tmp = new Color[WINDOW_COLORS];
    for (unsigned int i = 0; i <= steps; i++)
    {
        float x = (float)i / (float)steps;
        for (unsigned int j = first; j < first + n; j++)
        {
            tmp[j].r = from[j].r + (int)((colors[j].r - from[j].r) * x);
            tmp[j].g = from[j].g + (int)((colors[j].g - from[j].g) * x);
            tmp[j].b = from[j].b + (int)((colors[j].b - from[j].b) * x);
        }
        media->GetVideo()->SetPalette(&tmp[first], first, n);
        media->GetVideo()->Refresh();
        media->GetClock()->Delay(delay);
        media->PollEvents();
    }
    delete[] tmp;
}

void
Palette::FadeTo(Color* to, const unsigned int first, const unsigned int n, const unsigned int steps, const unsigned int delay)
{
    Color* tmp = new Color[WINDOW_COLORS];
    media->GetVideo()->GetPalette(tmp, 0, WINDOW_COLORS);
    for (unsigned int i = 0; i <= steps; i++)
    {
        float x = (float)i / (float)steps;
        for (unsigned int j = first; j < first + n; j++)
        {
            tmp[j].r = colors[j].r + (int)((to[j].r - colors[j].r) * x);
            tmp[j].g = colors[j].g + (int)((to[j].g - colors[j].g) * x);
            tmp[j].b = colors[j].b + (int)((to[j].b - colors[j].b) * x);
        }
        media->GetVideo()->SetPalette(&tmp[first], first, n);
        media->GetVideo()->Refresh();
        media->GetClock()->Delay(delay);
        media->PollEvents();
    }
    delete[] tmp;
}

void
Palette::FadeIn(const unsigned int first, const unsigned int n, const unsigned int steps, const unsigned int delay)
{
    Color* from = new Color[WINDOW_COLORS];
    memset(from, 0, WINDOW_COLORS * sizeof(Color));
    FadeFrom(from, first, n, steps, delay);
    delete[] from;
}

void
Palette::FadeOut(const unsigned int first, const unsigned int n, const unsigned int steps, const unsigned int delay)
{
    Color* to = new Color[WINDOW_COLORS];
    memset(to, 0, WINDOW_COLORS * sizeof(Color));
    FadeTo(to, first, n, steps, delay);
    delete[] to;
}
