/*
 * Main
 */

//#include <loguru.hpp>


#include "SDL.h"

#include "FileBuffer.h"
#include "MediaToolkit.h"
#include "PaletteResource.h"
#include "Geometry.h"
#include "RequestResource.h"

#include "worldFactory.hpp"
#include "eventHandler.hpp"
#include "renderer.hpp"

#include <boost/optional.hpp>

#include <fstream>   
#include <iostream>   
#include <vector>

#include <getopt.h>

#include <boost/range/adaptor/indexed.hpp>

#define LOG_S(level) (std::cout)

void exit_with_help(char *argv[]) {
    fprintf(stderr, "  nothing\n");
    exit(1);
}

static const char *arrow[] = {
  /* width height num_colors chars_per_pixel */
  "    32    32        3            1",
  /* colors */
  "X c #000000",
  ". c #ffffff",
  "  c None",
  /* pixels */
  "X                               ",
  "XX                              ",
  "X.X                             ",
  "X..X                            ",
  "X...X                           ",
  "X....X                          ",
  "X.....X                         ",
  "X......X                        ",
  "X.......X                       ",
  "X........X                      ",
  "X.....XXXXX                     ",
  "X..X..X                         ",
  "X.X X..X                        ",
  "XX  X..X                        ",
  "X    X..X                       ",
  "     X..X                       ",
  "      X..X                      ",
  "      X..X                      ",
  "       XX                       ",
  "                                ",
  "                                ",
  "                                ",
  "                                ",
  "                                ",
  "                                ",
  "                                ",
  "                                ",
  "                                ",
  "                                ",
  "                                ",
  "                                ",
  "                                ",
  "0,0"
};

static SDL_Cursor *init_system_cursor(const char *image[])
{
  int i, row, col;
  Uint8 data[4*32];
  Uint8 mask[4*32];
  int hot_x, hot_y;

  i = -1;
  for (row=0; row<32; ++row) {
    for (col=0; col<32; ++col) {
      if (col % 8) {
        data[i] <<= 1;
        mask[i] <<= 1;
      } else {
        ++i;
        data[i] = mask[i] = 0;
      }
      switch (image[4+row][col]) {
        case 'X':
          data[i] |= 0x01;
          mask[i] |= 0x01;
          break;
        case '.':
          mask[i] |= 0x01;
          break;
        case ' ':
          break;
      }
    }
  }
  sscanf(image[4+row], "%d,%d", &hot_x, &hot_y);
  return SDL_CreateCursor(data, mask, 32, 32, hot_x, hot_y);
}


int main(int argc, char *argv[]) {
    //loguru::init(argc, argv);
    
    char opt;
    while ((opt = getopt(argc, argv, "fvs:")) != EOF)
        switch (opt) {
        case 'h':
        case '?':
        default:
            exit_with_help(argv);
        }
    
    unsigned zone = 1;

    LOG_S(INFO) << "Loading zone: " << zone;
    
    auto worlds = std::vector<BAK::World>{};
    worlds.reserve(20);

    worlds.emplace_back(1, 9, 13);
    worlds.emplace_back(1, 9, 14);
    worlds.emplace_back(1, 9, 15);
    
    worlds.emplace_back(1, 10, 10);
    worlds.emplace_back(1, 10, 11);
    worlds.emplace_back(1, 10, 12);
    worlds.emplace_back(1, 10, 13);
    worlds.emplace_back(1, 10, 14);
    worlds.emplace_back(1, 10, 15);
    worlds.emplace_back(1, 10, 16);

    worlds.emplace_back(1, 11, 11);
    worlds.emplace_back(1, 11, 16);
    worlds.emplace_back(1, 11, 17);
    worlds.emplace_back(1, 12, 10);
    worlds.emplace_back(1, 12, 11);
    worlds.emplace_back(1, 12, 17);

    worlds.emplace_back(1, 13, 10);
    worlds.emplace_back(1, 13, 17);

    /*
    worlds.emplace_back(1, 14, 10);
    worlds.emplace_back(1, 14, 11);
    worlds.emplace_back(1, 14, 17);

    worlds.emplace_back(1, 15, 10);
    worlds.emplace_back(1, 15, 11);
    worlds.emplace_back(1, 15, 12);
    worlds.emplace_back(1, 15, 13);
    worlds.emplace_back(1, 15, 14);
    worlds.emplace_back(1, 15, 15);
    worlds.emplace_back(1, 15, 16);
    worlds.emplace_back(1, 15, 17);
    worlds.emplace_back(1, 15, 18);

    worlds.emplace_back(1, 16, 10);
    worlds.emplace_back(1, 16, 11);
    worlds.emplace_back(1, 16, 17);
    */

    auto worldCenter = worlds.at(0).mCenter;
    for (const auto& world : worlds)
    {
        std::cout << world.mCenter << std::endl;
    }

    bool drawTrees = false;

    auto worldScale  = 1.;

    MediaToolkit *media = MediaToolkit::GetInstance();

    media->GetVideo()->CreateWindow(2048, 2048);
    media->GetVideo()->SetMode(LORES_HICOL);
    
	auto cursor = init_system_cursor(arrow);
	SDL_SetCursor(cursor);
	SDL_ShowCursor(SDL_ENABLE);

    PaletteResource pal;
    pal.GetPalette()->Fill();
    pal.GetPalette()->Activate(0, 256);
    
    const auto Draw = [&](auto& world){
        for (auto& inst : world.mItemInsts)
        {   
            if (!drawTrees && inst.GetWorldItem().GetName().substr(0, 4) == "tree") continue;
            auto bloc = Vector2D{inst.GetLocation().GetX(), inst.GetLocation().GetY()};
            auto loc = BAK::TransformLoc(bloc, worldCenter, worldScale);
            auto brad = inst.GetWorldItem().GetGidItem().mRadius;
            auto rad = BAK::ScaleRad(brad, worldScale);

            if (brad == Vector2D{0, 0})
                rad = Vector2D{1, 1};
            //std::cout << loc << "-" << rad << " @ " << inst << std::endl;
            media->GetVideo()->DrawRect(
                (loc.GetX() - rad.GetX()),
                (96 * 4) - (loc.GetY() + rad.GetY()),
                rad.GetX() * 2,
                rad.GetY() * 2,
                inst.GetType());

            {
                std::cout << "Drawing: " << inst.GetWorldItem().GetName() << std::endl;
                const auto& vertices = inst.GetWorldItem().GetDatItem().mVertices;
                const auto& faces = inst.GetWorldItem().GetDatItem().mFaces;
                if (!faces.empty())
                {
                    bool stop = false;
                    for (const auto& face : faces | boost::adaptors::indexed())
                    {
                        unsigned faceVertices = face.value().size();
                        //std::cout << "Face vertices; " << faceVertices << std::endl;
                        int *x = new int[faceVertices];
                        int *y = new int[faceVertices];
                        int i = 0;
                        for (const auto& v : face.value())
                        {
                            if (v >= vertices.size())
                            {
                                std::cout << "Failed to draw: "
                                    << inst.GetWorldItem().GetName() << std::endl;
                                stop = true;
                                break;
                            }
                            assert(v < vertices.size());
                            auto vertex = vertices[v];
                            auto scaled = BAK::TransformLoc2(
                                Vector2D{vertex.GetX(), vertex.GetY()},
                                bloc - worldCenter,
                                worldScale);

                            if (brad.GetX() != 0)
                                scaled *= (rad.GetX() / brad.GetX());

                            x[i] = scaled.GetX();
                            y[i] = (96 * 4) - scaled.GetY();
                            i++;
                            /*std::cout << "Loc: "<< loc.GetX() << " : " << scaled.GetX() 
                                << " , " << loc.GetY() << " : " << scaled.GetY() 
                                << " BRad " << brad
                                << " Rad " << rad
                                << std::endl;*/
                        }
                        if (!stop)
                            media->GetVideo()->DrawPolygon(x, y, faceVertices, 250);
                        else
                            stop = true;
                    }
                }
            }

            media->GetVideo()->PutPixel(loc.GetX(), 96*4 -loc.GetY(), 254);
        }
    };

    BAK::EventRouter eventRouter{
        [&](const auto& e){
            const auto k = e.GetKey();
            int delta = (64000/16) / worldScale;

            if (k == KEY_ESCAPE)
                std::exit(0);
            else if (k == KEY_UP || k == KEY_w)
                worldCenter += Vector2D{0, delta};
            else if (k == KEY_DOWN || k == KEY_s)
                worldCenter -= Vector2D{0, delta};
            else if (k == KEY_LEFT || k == KEY_a)
                worldCenter -= Vector2D{delta, 0};
            else if (k == KEY_RIGHT || k == KEY_d)
                worldCenter += Vector2D{delta, 0};
            else if (k == KEY_z)
                worldScale += .2;
            else if (k == KEY_x)
                worldScale -= .2;
            else if (k == KEY_t)
                drawTrees = !drawTrees;

            std::cerr << "Center: " << worldCenter << std::endl;
        },
        [](const auto&){},
        [&worlds, &media, &Draw](const auto&){
            media->GetVideo()->Clear();
            for (auto& world : worlds)
            {
                Draw(world);
            }
            media->GetVideo()->Refresh();
        }
    };

    media->AddKeyboardListener(&eventRouter);
    media->AddUpdateListener(&eventRouter);

    media->GetClock()->StartTimer(TMR_TEST_APP, 1000);
    media->WaitEventLoop();
    media->GetClock()->CancelTimer(TMR_TEST_APP);

    return 0;
}


