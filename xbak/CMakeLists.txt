include_directories(${CMAKE_CURRENT_LIST_DIR})

add_library( xbak
    alt_stdint.h
    ConfigFile.h
    Defines.h     
    FileBuffer.cc FileBuffer.h
    FileManager.h
    ResourceArchive.h
	ResourceFile.h
	ResourceTag.cc
	BasicFile.cc
	ConfigData.h
	ConfigResource.cc
	Exception.cc
    GameData.h
	ResourceData.h
	ResourceIndex.cc
	ResourceTag.h
	BasicFile.h
	ConfigFile.cc
	ConfigResource.h
	Exception.h
	FileManager.cc
	ResourceArchive.cc
	ResourceFile.cc
	ResourceIndex.h
)
