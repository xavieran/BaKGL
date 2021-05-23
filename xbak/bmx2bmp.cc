#include <algorithm>
#include <iostream>
#include <iomanip>
#include <sstream>

#include "BMPWriter.h"
#include "Directories.h"
#include "Exception.h"
#include "FileManager.h"
#include "ImageResource.h"
#include "PaletteResource.h"
#include "TaggedImageResource.h"

int main ( int argc, char *argv[] )
{
    try
    {
        if ( ! ( ( argc != 2 )))
        {
            std::cerr << "Usage BMX PAL" << std::endl;
            return 1;
        }

        bool found = true;
        
        const auto bmxFile = std::string{argv[1]};
        const auto palFile = std::string{argv[2]};
        const auto baseName = bmxFile.substr(
            0,
            bmxFile.find(".BMX"));

        found = FileManager::GetInstance()->ResourceExists ( bmxFile );
        if ( found )
        {
            ImageResource spriteSlot;
            FileManager::GetInstance()->Load ( &spriteSlot, bmxFile );

            PaletteResource *palette = new PaletteResource;
            FileManager::GetInstance()->Load(palette, palFile);

            for ( unsigned int j = 0; j < spriteSlot.GetNumImages(); j++ )
            {
                Image *img = new Image(spriteSlot.GetImage(j));

                std::ofstream out{""};
                std::stringstream fname{""};
                fname << baseName << "-" << j << ".BMP";
                out.open(fname.str(), std::ios::out | std::ios::binary);

                std::cout << "Writing: " << fname.str() << std::endl;
                WriteBMP(out, *img, *palette->GetPalette());
            }
        }

        FileManager::CleanUp();
        Directories::CleanUp();

    }
    catch ( Exception &e )
    {
        e.Print ( "main" );
    }
    catch ( ... )
    {
        /* every exception should have been handled before */
        std::cerr << "Unhandled exception" << std::endl;
    }
    return 0;
}

