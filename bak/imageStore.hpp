#pragma once

#include "bak/image.hpp"

#include "xbak/FileBuffer.h"

namespace BAK {

std::vector<Image> LoadImages(FileBuffer& fb);

}
