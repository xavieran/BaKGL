#pragma once

#include "bak/image.hpp"

#include "bak/fileBuffer.hpp"

namespace BAK {

std::vector<Image> LoadImages(FileBuffer& fb);

}
