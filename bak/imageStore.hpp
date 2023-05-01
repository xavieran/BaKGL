#pragma once

#include "bak/image.hpp"

#include "bak/fileBufferFactory.hpp"

namespace BAK {

std::vector<Image> LoadImages(FileBuffer& fb);

}
