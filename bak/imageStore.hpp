#pragma once

#include <vector>

namespace BAK {

class Image;
class FileBuffer;

std::vector<Image> LoadImages(FileBuffer& fb);

}
