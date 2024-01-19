#include "bak/file/fileBuffer.hpp"

#include "com/bits.hpp"
#include "com/ostream.hpp"


namespace BAK {

class EventStateManagement
{
public:
    EventStateManagement(FileBuffer& fileBuffer)
    :
        mFileBuffer{fileBuffer}
    {
    }

private:
    FileBuffer& mBuffer;
};

}
