/*
 * Main
 */

#include "SDL.h"

#include "FileManager.h"
#include "FileBuffer.h"
#include "MediaToolkit.h"
#include "PaletteResource.h"
#include "Geometry.h"
#include "RequestResource.h"

#include "worldFactory.hpp"
#include "eventHandler.hpp"
#include "renderer.hpp"

#include <fstream>   
#include <iostream>   
#include <optional>
#include <vector>

#include <boost/range/adaptor/indexed.hpp>

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

// Some overlap with xbak names here...
namespace {

constexpr unsigned TILE_SIZE = 64000;
constexpr unsigned RES_X = 2048;
constexpr unsigned RES_Y = 2048;
constexpr unsigned MAP_SIZE_X = 96 * 4;
constexpr unsigned MAP_SIZE_Y = 96 * 4;

Vector2D TransformLoc(
    const Vector2D& loc,
    const Vector2D& centre,
    double scale)
{   
    auto translated = Vector2D{
        loc.GetX() - centre.GetX(), 
        loc.GetY() - centre.GetY()};
    return (translated * scale) * (static_cast<double>(MAP_SIZE_X) / ::TILE_SIZE);
}

Vector2D TransformLoc2(
    const Vector2D& loc,
    const Vector2D& centre,
    double scale)
{   
    auto translated = Vector2D{
        loc.GetX() + centre.GetX(), 
        loc.GetY() + centre.GetY()};
    return (translated * scale) * (static_cast<double>(MAP_SIZE_X) / ::TILE_SIZE);
}

Vector2D RotateAboutPoint(
    const Vector2D& p,
    const Vector2D& center,
    double theta)
{
    auto t = p;
    t -= center;

    auto j = Vector2D{
        static_cast<int>(t.GetX() * cos(theta) - t.GetY() * sin(theta)),
        static_cast<int>(t.GetX() * sin(theta) + t.GetY() * cos(theta))};
    j += center;
    return j;
}

}

int main(int argc, char *argv[])
{
	std::string zone = "Z01";

    PaletteResource *palz = new PaletteResource;
    std::stringstream palStr{""};
    palStr << std::setfill('0') << std::setw(2) << zone << ".PAL";
    FileManager::GetInstance()->Load(palz, palStr.str());

    auto& pal = *palz->GetPalette();

    auto textures  = BAK::TextureStore{zone, pal};
    auto zoneItems = BAK::ZoneItemStore{zone, textures};

    auto worlds = std::vector<BAK::World>{};
    worlds.reserve(60);

    //worlds.emplace_back(zoneItems, 9, 13);
    //worlds.emplace_back(zoneItems, 9, 13);
    //worlds.emplace_back(zoneItems, 9, 14);
    //worlds.emplace_back(zoneItems, 9, 15);
    //
    //worlds.emplace_back(zoneItems, 10, 10);
    //worlds.emplace_back(zoneItems, 10, 11);
    //worlds.emplace_back(zoneItems, 10, 12);
    //worlds.emplace_back(zoneItems, 10, 13);
    //worlds.emplace_back(zoneItems, 10, 14);
    //worlds.emplace_back(zoneItems, 10, 15);
    worlds.emplace_back(zoneItems, 10, 16);

    worlds.emplace_back(zoneItems, 11, 11);
    worlds.emplace_back(zoneItems, 11, 16);
    worlds.emplace_back(zoneItems, 11, 17);
    worlds.emplace_back(zoneItems, 12, 10);
    //worlds.emplace_back(zoneItems, 12, 11);
    //worlds.emplace_back(zoneItems, 12, 17);


    //worlds.emplace_back(zoneItems, 13, 10);
    //worlds.emplace_back(zoneItems, 13, 17);

    //worlds.emplace_back(zoneItems, 14, 10);
    //worlds.emplace_back(zoneItems, 14, 11);
    //worlds.emplace_back(zoneItems, 14, 17);

    //worlds.emplace_back(zoneItems, 15, 10);
    //worlds.emplace_back(zoneItems, 15, 11);
    //worlds.emplace_back(zoneItems, 15, 12);
    //worlds.emplace_back(zoneItems, 15, 13);
    //worlds.emplace_back(zoneItems, 15, 14);
    //worlds.emplace_back(zoneItems, 15, 15);
    //worlds.emplace_back(zoneItems, 15, 16);
    //worlds.emplace_back(zoneItems, 15, 17);
    //worlds.emplace_back(zoneItems, 15, 18);

    //worlds.emplace_back(zoneItems, 16, 10);
    //worlds.emplace_back(zoneItems, 16, 11);
    //worlds.emplace_back(zoneItems, 16, 17);

    /*auto worldCenter = Vector2D{
        static_cast<int>(worlds.at(0).GetCenter().x),
        static_cast<int>(worlds.at(0).GetCenter().y)};*/

    auto worldCenter = Vector2D{815243, 1073855};
    for (const auto& world : worlds)
    {
        std::cout << glm::to_string(world.GetCenter()) 
            << " T: " << glm::to_string(world.GetTile()) << std::endl;
    }

    bool drawTrees = false;

    auto worldScale  = 1.;

    MediaToolkit *media = MediaToolkit::GetInstance();

    media->GetVideo()->CreateWindow(800, 800);
    media->GetVideo()->SetMode(LORES_HICOL);
    
	auto cursor = init_system_cursor(arrow);
	SDL_SetCursor(cursor);
	SDL_ShowCursor(SDL_ENABLE);
    
    pal.Activate(0, 256);

    const auto Draw = [&](auto& world){
        for (auto& inst : world.GetItems())
        {   
            if (!drawTrees && inst.GetZoneItem().GetName().substr(0, 4) == "tree") continue;
            auto bloc = Vector2D{
                static_cast<int>(inst.GetLocation().x),
                static_cast<int>(inst.GetLocation().y)};
            auto loc = TransformLoc(bloc, worldCenter, worldScale);
            {
                const auto& vertices = inst.GetZoneItem().GetDatItem().mVertices;
                const auto& faces = inst.GetZoneItem().GetDatItem().mFaces;
                const auto& colors = inst.GetZoneItem().GetDatItem().mColors;

                if (!faces.empty())
                {
                    bool stop = false;
                    for (const auto& face : faces | boost::adaptors::indexed())
                    {
                        unsigned faceVertices = face.value().size();
                        int *x = new int[faceVertices];
                        int *y = new int[faceVertices];
                        int i = 0;
                        for (const auto& v : face.value())
                        {
                            if (v >= vertices.size())
                            {
                                std::cout << "Failed to draw: "
                                    << inst.GetZoneItem().GetName() << std::endl;
                                stop = true;
                                break;
                            }
                            auto scaleFactor = inst.GetZoneItem().GetDatItem().mScale;
                            assert(v < vertices.size());
                            auto rawV = Vector2D{vertices[v].GetX(), vertices[v].GetY()};
                            auto vertex = RotateAboutPoint(
                                rawV,
                                Vector2D{0,0},
                                //bloc,
                                inst.GetRotation().z);

                            auto scaled = TransformLoc2(
                                Vector2D{vertex.GetX(), vertex.GetY()} * scaleFactor,
                                bloc - worldCenter,
                                worldScale);

                            x[i] = scaled.GetX();
                            y[i] = (96 * 4) - scaled.GetY();
                            i++;
                        }
                        if (!stop)
                            media->GetVideo()->FillPolygon(
                                x, y,
                                faceVertices,
                                colors[face.index()]);
                        else
                            stop = true;
                    }
                }
            }

            media->GetVideo()->PutPixel(loc.GetX(), 96*4 -loc.GetY(), 0x8f);
        }
    };
    
    std::optional<Key> heldKey{};

    BAK::EventRouter eventRouter{
        [&](const auto& e){
            const auto k = e.GetKey();
            heldKey = k;

            if (k == KEY_ESCAPE)
                std::exit(0);
            else if (k == KEY_t)
                drawTrees = !drawTrees;

            std::cerr << "Center: " << worldCenter << std::endl;
        },
        [&](const auto&){
            heldKey.reset();
        },
        [&](const auto&){
            media->GetVideo()->Clear();
            for (auto& world : worlds)
            {
                Draw(world);
            }
            
            int delta = (64000/64) / worldScale;

            if (heldKey)
            {
                auto k = *heldKey;
                if (k == KEY_UP || k == KEY_w)
                    worldCenter += Vector2D{0, delta};
                else if (k == KEY_DOWN || k == KEY_s)
                    worldCenter -= Vector2D{0, delta};
                else if (k == KEY_LEFT || k == KEY_a)
                    worldCenter -= Vector2D{delta, 0};
                else if (k == KEY_RIGHT || k == KEY_d)
                    worldCenter += Vector2D{delta, 0};
                else if (k == KEY_z)
                    worldScale += .04;
                else if (k == KEY_x)
                    worldScale -= .04;

                heldKey.reset();
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


