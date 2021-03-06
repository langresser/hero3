project(libvcmi)
cmake_minimum_required(VERSION 2.6)

include_directories(${CMAKE_HOME_DIRECTORY} ${CMAKE_CURRENT_SOURCE_DIRECTORY} ${CMAKE_HOME_DIRECTORY}/lib)
include_directories(${SDL_INCLUDE_DIR} ${ZLIB_INCLUDE_DIR})

set(lib_SRCS
        Filesystem/CBinaryReader.cpp
        Filesystem/CFilesystemLoader.cpp
        Filesystem/CMemoryStream.cpp
        Filesystem/CFileInfo.cpp
        Filesystem/CLodArchiveLoader.cpp
        Filesystem/CResourceLoader.cpp
        Filesystem/CFileInputStream.cpp
        Filesystem/CCompressedStream.cpp
        BattleAction.cpp
        BattleHex.cpp
        BattleState.cpp
        CArtHandler.cpp
        CBattleCallback.cpp
        CBuildingHandler.cpp
        CCampaignHandler.cpp
        CConfigHandler.cpp
        CConsoleHandler.cpp
        CCreatureHandler.cpp
        CCreatureSet.cpp
        CDefObjInfoHandler.cpp
        CGameInterface.cpp
        CGameState.cpp
        CGeneralTextHandler.cpp
        CHeroHandler.cpp
        CLogger.cpp
        CMapInfo.cpp
        CModHandler.cpp
        CObjectHandler.cpp
        CObstacleInstance.cpp
        Connection.cpp
        CSpellHandler.cpp
        CThreadHelper.cpp
        CTownHandler.cpp
        HeroBonus.cpp
        IGameCallback.cpp
        JsonNode.cpp
        map.cpp
        NetPacksLib.cpp
        ResourceSet.cpp
        VCMI_Lib.cpp
)

add_library(vcmi SHARED ${lib_SRCS})
target_link_libraries(vcmi ${Boost_LIBRARIES} ${SDL_LIBRARY} ${ZLIB_LIBRARIES})

install(TARGETS vcmi DESTINATION ${LIB_DIR})
