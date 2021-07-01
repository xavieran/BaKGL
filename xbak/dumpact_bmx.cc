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
            char charA = 'A' - 1;
            for (int k = 0; k < 4; k++)
            {
                std::stringstream actN{};
                actN << "ACT0" << std::setw ( 2 ) << std::setfill('0') << i;

                std::stringstream spriteStream{""};
                spriteStream << actN.str();
                if (charA >= 'A') spriteStream << charA;
                spriteStream << ".BMX";

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
                        fname << actN.str();
                        if (charA >= 'A') fname << charA;
                        fname << ".BMP";
                        out.open(fname.str(), std::ios::out | std::ios::binary);

                        std::cout << "Writing: " << fname.str() << std::endl;
                        WriteBMP(out, *img, *palette->GetPalette());
                    }
                }
                charA++;
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

