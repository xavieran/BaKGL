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
        if ( ! ( ( argc == 2 )))
        {
            std::cerr << "Usage: " << argv[0] << " <Zone number>" << std::endl;
            return 1;
        }

        auto n = argv[1];

        unsigned int i = 0;
        bool found = true;
        unsigned imgCount = 0;
        while ( found )
        {
            std::stringstream spriteStream{""};
            spriteStream << "Z" << std::setw ( 2 ) << std::setfill ( '0' ) << n << "SLOT" << std::setw ( 1 ) << i << ".BMX";
            std::stringstream palStream{""};
            palStream << "Z" << n << ".PAL";

            found = FileManager::GetInstance()->ResourceExists ( spriteStream.str() );
            if ( found )
            {
                ImageResource spriteSlot;
                FileManager::GetInstance()->Load ( &spriteSlot, spriteStream.str() );

                PaletteResource *palette = new PaletteResource;
                FileManager::GetInstance()->Load(palette, palStream.str());

                for ( unsigned int j = 0; j < spriteSlot.GetNumImages(); j++ )
                {
                    Image *img = new Image(spriteSlot.GetImage(j));

                    std::ofstream out{""};
                    std::stringstream fname{"Z"};
                    fname << argv[1]  << "-" << imgCount <<  ".BMP";
                    out.open(fname.str(), std::ios::out | std::ios::binary);

                    std::cout << "Writing: " << fname.str() << std::endl;
                    WriteBMP(out, *img, *palette->GetPalette());
                    imgCount++;
                }
                i++;
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

