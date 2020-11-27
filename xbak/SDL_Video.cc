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
#include "SDL_Video.h"

#include <iostream>

SDL_Video::SDL_Video()
        : Video()
        , info(SDL_GetVideoInfo())
        , window(0)
        , windowBuffer(0)
        , hireslocolSurface()
        , loreshicolSurface()
        , currentSurface(&loreshicolSurface)
{
    scaling = 1;
    hireslocolSurface.buffer = 0;
    loreshicolSurface.buffer = 0;
}

SDL_Video::~SDL_Video()
{
    if (loreshicolSurface.buffer != 0)
    {
        SDL_FreeSurface(loreshicolSurface.buffer);
    }
    if (hireslocolSurface.buffer != 0)
    {
        SDL_FreeSurface(hireslocolSurface.buffer);
    }
    if (windowBuffer != 0)
    {
        SDL_FreeSurface(windowBuffer);
    }
    if (window != 0)
    {
        SDL_FreeSurface(window);
    }
}

void SDL_Video::SetMode ( const VideoMode m )
{
    mode = m;
    switch (mode)
    {
    case HIRES_LOWCOL:
        currentSurface = &hireslocolSurface;
        break;
    case LORES_HICOL:
        currentSurface = &loreshicolSurface;
        break;
    default:
        break;
    }
}

void SDL_Video::CreateWindow ( const unsigned int width, const unsigned int height)
{
    unsigned int flags = SDL_ANYFORMAT;
    if (info->hw_available)
    {
        flags |= SDL_HWSURFACE | SDL_DOUBLEBUF;
    }
    else
    {
        flags |= SDL_SWSURFACE;
    }

    int bpp = SDL_VideoModeOK(width, height, WINDOW_BPP, flags);
    if (bpp <= 0)
    {
        throw SDL_Exception(__FILE__, __LINE__, SDL_GetError());
    }

    window = SDL_SetVideoMode(width, height, 0, flags);
    if (window == 0)
    {
        throw SDL_Exception(__FILE__, __LINE__, SDL_GetError());
    }
    SDL_ShowCursor(SDL_DISABLE);
    SDL_WM_SetCaption(WINDOW_TITLE, 0);

    windowBuffer = SDL_CreateRGBSurface(SDL_SWSURFACE, width, height, WINDOW_BPP, 0, 0, 0, 0);
    if (windowBuffer == 0)
    {
        throw SDL_Exception(__FILE__, __LINE__, SDL_GetError());
    }

    hireslocolSurface.xOffset = 0;
    hireslocolSurface.yOffset = (WINDOW_HEIGHT - HIRES_LOCOL_HEIGHT) / 2;
    hireslocolSurface.scaling = scaling;
    hireslocolSurface.buffer = SDL_CreateRGBSurface(SDL_SWSURFACE, HIRES_LOCOL_WIDTH, HIRES_LOCOL_HEIGHT, WINDOW_BPP, 0, 0, 0, 0);
    if (hireslocolSurface.buffer == 0)
    {
        throw SDL_Exception(__FILE__, __LINE__, SDL_GetError());
    }

    loreshicolSurface.xOffset = 0;
    loreshicolSurface.yOffset = 0;
    loreshicolSurface.scaling = 2 * scaling;
    loreshicolSurface.buffer = SDL_CreateRGBSurface(SDL_SWSURFACE, LORES_HICOL_WIDTH, LORES_HICOL_HEIGHT, WINDOW_BPP, 0, 0, 0, 0);
    if (loreshicolSurface.buffer == 0)
    {
        throw SDL_Exception(__FILE__, __LINE__, SDL_GetError());
    }

}

void SDL_Video::CreateWindow ( const unsigned int sc )
{
    scaling = sc;
    width = WINDOW_WIDTH * scaling;
    height = WINDOW_HEIGHT * scaling;

    CreateWindow(width, height);
}

void SDL_Video::Clear()
{
    SDL_FillRect(currentSurface->buffer, 0, 0);
}

void SDL_Video::Clear(const int x, const int y, const int w, const int h)
{
    SDL_Rect rect = {
        static_cast<short int>(x),
        static_cast<short int>(y),
        static_cast<short unsigned int>(w),
        static_cast<short unsigned int>(h)};
    SDL_FillRect(currentSurface->buffer, &rect, 0);
}

unsigned int SDL_Video::GetPixel(const int x, const int y)
{
    if ((x >= 0) && (x < currentSurface->buffer->w) && (y >= 0) && (y < currentSurface->buffer->h))
    {
        uint8_t *p = (uint8_t *)currentSurface->buffer->pixels + y * currentSurface->buffer->pitch + x;
        return (unsigned int)(*p);
    }
    return 0;
}

void SDL_Video::PutPixel(const int x, const int y, const unsigned int c)
{
    if ((x >= 0) && (x < currentSurface->buffer->w) && (y >= 0) && (y < currentSurface->buffer->h))
    {
        uint8_t *p = (uint8_t *)currentSurface->buffer->pixels + y * currentSurface->buffer->pitch + x;
        *p = (uint8_t)c;
    }
}

void SDL_Video::DrawHLine(const int x, const int y, const int w, const unsigned int c)
{
    int xx = (x < 0 ? 0 : (x < currentSurface->buffer->w ? x : currentSurface->buffer->w));
    int yy = (y < 0 ? 0 : (y < currentSurface->buffer->h ? y : currentSurface->buffer->h));
    int ww = w + x - xx;
    if (ww <= 0)
    {
        return;
    }
    if (ww + xx > currentSurface->buffer->w)
    {
        ww = currentSurface->buffer->w - xx;
    }
    SDL_Rect rect = {
        static_cast<short int>(xx), 
        static_cast<short int>(yy), 
        static_cast<short unsigned int>(ww), 1};
    SDL_FillRect(currentSurface->buffer, &rect, c);
}

void SDL_Video::DrawVLine(const int x, const int y, const int h, const unsigned int c)
{
    int xx = (x < 0 ? 0 : (x < currentSurface->buffer->w ? x : currentSurface->buffer->w));
    int yy = (y < 0 ? 0 : (y < currentSurface->buffer->h ? y : currentSurface->buffer->h));
    int hh = h + y - yy;
    if (hh + yy > currentSurface->buffer->h)
    {
        hh = currentSurface->buffer->h - yy;
    }
    SDL_Rect rect = { 
        static_cast<short int>(xx), 
        static_cast<short int>(yy), 
        1,
        static_cast<short unsigned int>(hh)};

    SDL_FillRect(currentSurface->buffer, &rect, c);
}

#define swap(a,b) { int h = a; a = b; b = h; }

void SDL_Video::DrawLine(int x1, int y1, int x2, int y2, const unsigned int c)
{
    // Bresenham's line algorithm
    bool steep = abs(y2 - y1) > abs(x2 - x1);
    if (steep)
    {
        swap(x1, y1);
        swap(x2, y2);
    }
    if (x1 > x2)
    {
        swap(x1, x2);
        swap(y1, y2);
    }
    int dx = x2 - x1;
    int dy = abs(y2 - y1);
    int err = 0;
    int yy;
    if (y1 < y2)
    {
        yy = 1;
    }
    else
    {
        yy = -1;
    }
    int y = y1;
    for (int x = x1; x <= x2; x++)
    {
        if (steep)
        {
            PutPixel(y, x, c);
        }
        else
        {
            PutPixel(x, y, c);
        }
        err += dy;
        if ((err + err) >= dx)
        {
            y += yy;
            err -= dx;
        }
    }
}

void SDL_Video::DrawRect(const int x, const int y, const int w, const int h, const unsigned int c)
{
    short int X = static_cast<short int>(x);
    short int Y = static_cast<short int>(y);
    short unsigned int W = static_cast<short unsigned int>(w);
    short unsigned int H = static_cast<short unsigned int>(h);
    short unsigned int C = static_cast<short unsigned int>(c);
    SDL_Rect top = {X, Y, W, 1};
    SDL_FillRect(currentSurface->buffer, &top, C);
    SDL_Rect left = {X, Y, 1, H};
    SDL_FillRect(currentSurface->buffer, &left, C);
    SDL_Rect right = {static_cast<short int>(X + W - 1), Y, 1, H};
    SDL_FillRect(currentSurface->buffer, &right, C);
    SDL_Rect bottom = {X, static_cast<short int>(Y + H - 1), W, 1};
    SDL_FillRect(currentSurface->buffer, &bottom, C);
}

void SDL_Video::FillRect(const int x, const int y, const int w, const int h, const unsigned int c)
{
    SDL_Rect rect = {
        static_cast<short int>(x),
        static_cast<short int>(y),
        static_cast<short unsigned int>(w),
        static_cast<short unsigned int>(h)};
    SDL_FillRect(currentSurface->buffer, &rect, c);
}

void SDL_Video::FillRect(const int x, const int y, const int w, const int h,
                         uint8_t *p, const int xx, const int yy, const int ww)
{
    p += (yy + y) * ww + xx + x;
    for (int j = y; j < y + h; j++)
    {
        for (int i = x; i < x + w; i++)
        {
            PutPixel(i, j, *p++);
        }
        p += ww - w;
    }
}

void SDL_Video::DrawPolygon(const int *x, const int *y, const unsigned int n, const unsigned int c)
{
    for (unsigned int i = 1; i < n; i++)
    {
        DrawLine(x[i-1], y[i-1], x[i], y[i], c);
    }
    DrawLine(x[n-1], y[n-1], x[0], y[0], c);
}

bool SDL_Video::CreateEdge(PolygonEdge &edge, const int x1, const int y1, const int x2, const int y2)
{
    if (y1 == y2)
    {
        return false;
    }
    else
    {
        if (y1 < y2)
        {
            edge.x0 = x1;
            edge.y0 = y1;
            edge.x1 = x2;
            edge.y1 = y2;
        }
        else
        {
            edge.x0 = x2;
            edge.y0 = y2;
            edge.x1 = x1;
            edge.y1 = y1;
        }
        edge.dxdy = (float)(edge.x1 - edge.x0) / (float)(edge.y1 - edge.y0);
        return true;
    }
}

void SDL_Video::SortEdges(PolygonEdge* &edges, const unsigned int n)
{
    unsigned int m = 1;
    while (m < n)
    {
        unsigned int i = 0;
        PolygonEdge pe;
        memcpy(&pe, &edges[m], sizeof(PolygonEdge));
        while ((i < m) && ((edges[i].y0 < pe.y0) || ((edges[i].y0 == pe.y0) && (edges[i].x0 < pe.x0)) ||
                           ((edges[i].y0 == pe.y0) && (edges[i].x0 == pe.x0) && (edges[i].y1 < pe.y1))))
        {
            i++;
        }
        for (unsigned int j = m; j > i; j--)
        {
            memcpy(&edges[j], &edges[j-1], sizeof(PolygonEdge));
        }
        memcpy(&edges[i], &pe, sizeof(PolygonEdge));
        m++;
    }
}

void SDL_Video::FillPolygon(const int* x, const int* y, const unsigned int n, const unsigned int c)
{
    PolygonEdge *edges = new PolygonEdge[n];
    unsigned int m = 0;
    if (CreateEdge(edges[m], x[n-1], y[n-1], x[0], y[0]))
    {
        m++;
    }
    for (unsigned int i = 1; i < n; i++)
    {
        if (CreateEdge(edges[m], x[i-1], y[i-1], x[i], y[i]))
        {
            m++;
        }
    }
    SortEdges(edges, m);
    unsigned int l = 0;
    while (l < m)
    {
        bool draw = false;
        int xx = edges[l].x0;
        int yy = edges[l].y0;
        bool isdot = (yy == edges[l].y1);
        unsigned int i = l + 1;
        while ((i < m) && (edges[i].y0 == yy))
        {
            if (!(isdot && (xx == edges[i].x0) && (yy < edges[i].y1)))
            {
                draw = !draw;
            }
            if (draw)
            {
                DrawHLine(xx, yy, edges[i].x0 - xx + 1, c);
            }
            xx = edges[i].x0;
            isdot = (yy == edges[i].y1);
            i++;
        }
        i = l;
        while ((i < m) && (edges[i].y0 == yy))
        {
            if (edges[i].y1 > yy)
            {
                edges[i].y0++;
            }
            edges[i].x0 = edges[i].x1 - (int)(((float)(edges[i].y1 - edges[i].y0) * edges[i].dxdy) - 0.5);
            i++;
        }
        SortEdges(edges, m);
        while ((l < m) && (edges[l].y1 == yy))
        {
            l++;
        }
    }
    delete[] edges;
}

void SDL_Video::FillPolygon(const int* x, const int* y, const unsigned int n,
                            uint8_t *p, const int xx, const int yy, const int ww)
{
    PolygonEdge *edges = new PolygonEdge[n];
    unsigned int m = 0;
    if (CreateEdge(edges[m], x[n-1], y[n-1], x[0], y[0]))
    {
        m++;
    }
    for (unsigned int i = 1; i < n; i++)
    {
        if (CreateEdge(edges[m], x[i-1], y[i-1], x[i], y[i]))
        {
            m++;
        }
    }
    SortEdges(edges, m);
    unsigned int l = 0;
    while (l < m)
    {
        bool draw = false;
        int x0 = edges[l].x0;
        int y0 = edges[l].y0;
        bool isdot = (y0 == edges[l].y1);
        unsigned int i = l + 1;
        while ((i < m) && (edges[i].y0 == y0))
        {
            if (!(isdot && (x0 == edges[i].x0) && (y0 < edges[i].y1)))
            {
                draw = !draw;
            }
            if (draw)
            {
                if ((y0 >= 0) && (y0 < currentSurface->buffer->h))
                {
                    uint8_t *q = p + (yy + y0) * ww + xx + x0;
                    for (int x1 = x0 < 0 ? 0 : (x0 < currentSurface->buffer->w ? x0 : currentSurface->buffer->w - 1); (x1 <= edges[i].x0) && (x1 < currentSurface->buffer->w); x1++)
                    {
                        PutPixel(x1, y0, *q++);
                    }
                }
            }
            x0 = edges[i].x0;
            isdot = (y0 == edges[i].y1);
            i++;
        }
        i = l;
        while ((i < m) && (edges[i].y0 == y0))
        {
            if (edges[i].y1 > y0)
            {
                edges[i].y0++;
            }
            edges[i].x0 = edges[i].x1 - (int)(((float)(edges[i].y1 - edges[i].y0) * edges[i].dxdy) - 0.5);
            i++;
        }
        SortEdges(edges, m);
        while ((l < m) && (edges[l].y1 == y0))
        {
            l++;
        }
    }
    delete[] edges;
}

void
SDL_Video::ReadImage(const int x, const int y, const int w, const int h, uint8_t *p)
{
    for (int j = y; j < y + h; j++)
    {
        for (int i = x; i < x + w; i++)
        {
            *p++ = GetPixel(i, j);
        }
    }
}

void SDL_Video::DrawImage(const int x, const int y, const int w, const int h, uint8_t *p)
{
    for (int j = y; j < y + h; j++)
    {
        for (int i = x; i < x + w; i++)
        {
            PutPixel(i, j, *p++);
        }
    }
}

void SDL_Video::DrawImage(const int x, const int y, const int w, const int h, uint8_t *p, const uint8_t transparant)
{
    for (int j = y; j < y + h; j++)
    {
        for (int i = x; i < x + w; i++)
        {
            if (*p != transparant)
            {
                PutPixel(i, j, *p);
            }
            p++;
        }
    }
}

void SDL_Video::DrawImage(const int x, const int y, const int w, const int,
                          const int xx, const int yy, const int ww, const int hh, uint8_t *p)
{
    p += yy * w;
    for (int j = y + yy; j < y + yy + hh; j++)
    {
        p += xx;
        for (int i = x + xx; i < x + xx + ww; i++)
        {
            PutPixel(i, j, *p);
            p++;
        }
        p += w - ww - xx;
    }
}

void SDL_Video::DrawImage(const int x, const int y, const int w, const int,
                          const int xx, const int yy, const int ww, const int hh,
                          uint8_t *p, const uint8_t transparant)
{
    p += yy * w;
    for (int j = y + yy; j < y + yy + hh; j++)
    {
        p += xx;
        for (int i = x + xx; i < x + xx + ww; i++)
        {
            if (*p != transparant)
            {
                PutPixel(i, j, *p);
            }
            p++;
        }
        p += w - ww - xx;
    }
}

void SDL_Video::DrawGlyph(const int x, const int y, const int w, const int h, const uint8_t c, uint16_t *p)
{
    for (int j = 0; j < h; j++)
    {
        for (int i = 0; i < w; i++)
        {
            if (*p & (0x8000 >> i))
            {
                PutPixel(x + i, y + j, c);
            }
        }
        p++;
    }
}

void SDL_Video::DrawGlyphItalic(const int x, const int y, const int w, const int h, const uint8_t c, uint16_t *p)
{
    for (int j = 0; j < h; j++)
    {
        for (int i = 0; i < w; i++)
        {
            if (*p & (0x8000 >> i))
            {
                PutPixel(x + i + 4 - (j / 3), y + j, c);
            }
        }
        p++;
    }
}

void SDL_Video::GetPalette(Color *color, const unsigned int first, const unsigned int n)
{
    if (currentSurface->buffer->format->palette)
    {
        memcpy(color, &(currentSurface->buffer->format->palette->colors[first]), n * sizeof(SDL_Color));
    }
}

void SDL_Video::SetPalette(Color *color, const unsigned int first, const unsigned int n)
{   
    if (currentSurface->buffer->format->palette)
    {
        SDL_SetPalette(currentSurface->buffer, SDL_LOGPAL, (SDL_Color *)color, first, n);
    }
    if (windowBuffer->format->palette)
    {
        SDL_SetPalette(windowBuffer, SDL_LOGPAL, (SDL_Color *)color, first, n);
    }
}

void SDL_Video::Refresh()
{
    SDL_UpdateRect(currentSurface->buffer, 0, 0, 0, 0);
    for (int y = 0; y < currentSurface->buffer->h; y++)
    {
        for (int x = 0; x < currentSurface->buffer->w; x++)
        {
            memset((uint8_t *)windowBuffer->pixels + ((y + currentSurface->yOffset) * windowBuffer->pitch + (x + currentSurface->xOffset)) * currentSurface->scaling,
                   *((uint8_t *)currentSurface->buffer->pixels + y * currentSurface->buffer->pitch + x), currentSurface->scaling);
        }
        for (unsigned int i = 1; i < currentSurface->scaling; i++)
        {
            memcpy((uint8_t *)windowBuffer->pixels + ((y + currentSurface->yOffset) * currentSurface->scaling + i) * windowBuffer->pitch,
                   (uint8_t *)windowBuffer->pixels + (y + currentSurface->yOffset) * currentSurface->scaling * windowBuffer->pitch, windowBuffer->w);
        }
    }
    SDL_UpdateRect(windowBuffer, 0, 0, 0, 0);
    SDL_Surface *tmp = SDL_DisplayFormat(windowBuffer);
    SDL_BlitSurface(tmp, 0, window, 0);
    SDL_FreeSurface(tmp);
    SDL_Flip(window);
}

#ifdef DEBUG
void SDL_Video::GrabInput(const bool)
{
}
#else
void SDL_Video::GrabInput(const bool toggle)
{
    SDL_WM_GrabInput(toggle ? SDL_GRAB_ON : SDL_GRAB_OFF);
}
#endif

void SDL_Video::SaveScreenShot(const std::string& filename)
{
    SDL_UpdateRect(windowBuffer, 0, 0, 0, 0);
    SDL_SaveBMP(windowBuffer, filename.c_str());
}
