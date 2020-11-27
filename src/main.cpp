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
    
    BAK::World world{1, 11, 16};
    bool drawTrees = false;

    auto worldCenter = Vector2D{0, 0};

    MediaToolkit *media = MediaToolkit::GetInstance();

    media->GetVideo()->CreateWindow(1024, 1024);
    media->GetVideo()->SetMode(LORES_HICOL);
    
	auto cursor = init_system_cursor(arrow);
	SDL_SetCursor(cursor);
	SDL_ShowCursor(SDL_ENABLE);

    PaletteResource pal;
    pal.GetPalette()->Fill();
    pal.GetPalette()->Activate(0, 256);
    
    const auto Draw = [&](){
        for (const auto& inst : world.mItemInsts)
        {   
            if (!drawTrees && inst.GetWorldItem().GetName().substr(0, 4) == "tree") continue;
            auto bloc = inst.GetLocation();
            auto loc = world.ScaleLoc(bloc) + worldCenter;
            auto brad = inst.GetWorldItem().GetGidItem().mRadius;
            auto rad = world.ScaleRad(brad);
            if (brad == Vector2D{0,0})
                rad = Vector2D{1,1};
            //std::cout << bloc << "-" << brad << " @ " << inst << std::endl;
            std::cout << loc << "-" << rad << " @ " << inst << std::endl;
            media->GetVideo()->DrawRect(
                (loc.GetX() - rad.GetX()),
                (96 * 4) - (loc.GetY() + rad.GetY()),
                rad.GetX() * 2,
                rad.GetY() * 2,
                inst.GetType());//244);

            const auto& textVertices = inst.GetWorldItem().GetGidItem().mTextureCoords;
            const auto& otherVertices = inst.GetWorldItem().GetGidItem().mOtherCoords;
            if (!textVertices.empty())
            {
                std::cout << "TV:" << std::endl;
                for (const auto& tv : textVertices)
                    std::cout << tv << std::endl;
            }

            {
                const auto& vertices = otherVertices;
                if (!vertices.empty())
                {
                    int *x = new int[vertices.size()];
                    int *y = new int[vertices.size()];
                    for (const auto& vertex : vertices | boost::adaptors::indexed())
                    {
                        auto scaled = world.ScaleLoc(
                            Vector2D{vertex.value().GetX(), vertex.value().GetY()}
                            + bloc) + worldCenter;
                        x[vertex.index()] = scaled.GetX();
                        y[vertex.index()] = (96*4) - scaled.GetY();
                        
                        std::cout << "(" << x[vertex.index()] << "," << y[vertex.index()] << ")" << std::endl;
                    }
                    media->GetVideo()->DrawPolygon(x, y, vertices.size(), inst.GetType());
                }
            }
            {
                const auto& vertices = inst.GetWorldItem().GetDatItem().mVertices;
                if (!vertices.empty())
                {
                    int *x = new int[vertices.size()];
                    int *y = new int[vertices.size()];
                    for (const auto& vertex : vertices | boost::adaptors::indexed())
                    {
                        auto scaled = world.ScaleLoc(
                            Vector2D{vertex.value().GetX(), vertex.value().GetY()}
                            + bloc) + worldCenter;
                        x[vertex.index()] = scaled.GetX();
                        y[vertex.index()] = (96*4) - scaled.GetY();
                        
                        std::cout << "(" << x[vertex.index()] << "," << y[vertex.index()] << ")" << std::endl;
                    }
                    media->GetVideo()->DrawPolygon(x, y, vertices.size(), 245);
                }
            }

            media->GetVideo()->PutPixel(loc.GetX(),96*4 -loc.GetY(), 254);
        }
    };

     BAK::EventRouter eventRouter{

        [&](const auto& e){
            const auto k = e.GetKey();
            int delta = 10;
            if (k & (KEY_LSHIFT)) delta = 30;

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
            else if (k == KEY_t)
                drawTrees = !drawTrees;

            std::cout << "Center: " << worldCenter << std::endl;
        },
        [](const auto&){},
        [&](const auto&){
            media->GetVideo()->Clear();
            Draw();
            media->GetVideo()->Refresh();
        }
    };

    media->AddKeyboardListener(&eventRouter);
    media->AddUpdateListener(&eventRouter);

    media->GetClock()->StartTimer(TMR_TEST_APP, 100);
    media->WaitEventLoop();
    media->GetClock()->CancelTimer(TMR_TEST_APP);

    return 0;
}


