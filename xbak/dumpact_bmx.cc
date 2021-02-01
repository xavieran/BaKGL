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
        if ( ! ( ( argc == 1 )))
        {
            std::cerr << "Usage" << std::endl;
            return 1;
        }

        unsigned int i = 1;
        bool found = true;

        for (unsigned i = 1; i < 54; i++)
        {
            std::string charA = "";
            for (int k = 0; k < 2; k++)
            {
                std::stringstream actN{};
                actN << "ACT0" << std::setw ( 2 ) << std::setfill('0') << i;

                std::stringstream spriteStream{""};
                spriteStream << actN.str() << charA << ".BMX";
                std::stringstream palStream{""};
                palStream << actN.str() << ".PAL";

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
                        std::stringstream fname{""};
                        fname << actN.str() << charA << ".BMP";
                        out.open(fname.str(), std::ios::out | std::ios::binary);

                        std::cout << "Writing: " << fname.str() << std::endl;
                        WriteBMP(out, *img, *palette->GetPalette());
                    }
                    charA = "A";
                }
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

