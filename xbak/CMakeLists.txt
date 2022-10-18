list(APPEND SOURCE_FILES
    alt_stdint.h
    AnimationResource.cc AnimationResource.h
    Audio.h
    BadgeWidget.cc BadgeWidget.h
    BasicFile.cc BasicFile.h
    BMP.cc BMP.h
    BMPWriter.h
    BookResource.cc BookResource.h
    ButtonWidget.cc ButtonWidget.h
    Camera.cc Camera.h
    Chapter.cc Chapter.h
    CharacterButtonWidget.cc CharacterButtonWidget.h
    ChoiceWidget.cc ChoiceWidget.h
    Clock.h
    CombatViewWidget.cc CombatViewWidget.h
    CompassWidget.cc CompassWidget.h
    ConfigData.h
    ConfigFile.cc ConfigFile.h
    ConfigResource.cc ConfigResource.h
    ContainerWidget.cc ContainerWidget.h
    Defines.h
    Dialog.cc
    DialogFactory.cc DialogFactory.h
    Dialog.h
    DialogResource.cc DialogResource.h
    DialogWindow.cc DialogWindow.h
    Directories.cc Directories.h
    EquipmentItemWidget.cc EquipmentItemWidget.h
    EquipmentWidget.cc EquipmentWidget.h
    Event.cc Event.h
    EventListener.h
    Exception.cc Exception.h
    FileBuffer.cc FileBuffer.h
    FileManager.cc FileManager.h
    Font.cc Font.h
    FontResource.cc FontResource.h
    GameApplication.cc GameApplication.h
    Game.cc Game.h
    GameData.h
    GameFile.cc GameFile.h
    GameResource.cc GameResource.h
    GameState.cc GameState.h
    GameViewWidget.cc GameViewWidget.h
    GenericObject.cc GenericObject.h
    Geometry.cc Geometry.h
    HotspotWidget.cc HotspotWidget.h
    ImageButtonWidget.cc ImageButtonWidget.h
    Image.cc Image.h
    ImageResource.cc ImageResource.h
    ImageWidget.cc ImageWidget.h
    Inventory.cc Inventory.h
    InventoryItem.cc InventoryItem.h
    InventoryItemWidget.cc InventoryItemWidget.h
    InventoryWidget.cc InventoryWidget.h
    LabelResource.cc LabelResource.h
    MapViewWidget.cc MapViewWidget.h
    MediaToolkit.cc MediaToolkit.h
    MoviePlayer.cc MoviePlayer.h
    MovieResource.cc MovieResource.h
    NameResource.cc NameResource.h
    Null_Audio.cc Null_Audio.h
    ObjectResource.cc ObjectResource.h
    Observer.h
    Orientation.cc Orientation.h
    Palette.cc Palette.h
    PaletteResource.cc PaletteResource.h
    PanelWidget.cc PanelWidget.h
    Party.cc Party.h
    PartyResource.cc PartyResource.h
    PatternPolygonObject.cc PatternPolygonObject.h
    PlayerCharacter.cc PlayerCharacter.h
    Pointer.cc Pointer.h
    PointerManager.cc PointerManager.h
    PolygonObject.cc PolygonObject.h
    PopUpWidget.cc PopUpWidget.h
    PortraitWidget.cc PortraitWidget.h
    Position.cc Position.h
    Preferences.cc Preferences.h
    PreferencesDialogBridge.cc PreferencesDialogBridge.h
    RampResource.cc RampResource.h
    RatingsWidget.cc RatingsWidget.h
    RequestResource.cc RequestResource.h
    ResourceArchive.cc ResourceArchive.h
    ResourceData.h
    ResourceFile.cc ResourceFile.h
    ResourceIndex.cc ResourceIndex.h
    ResourceTag.cc ResourceTag.h
    Scene.cc Scene.h
    SceneFactory.cc SceneFactory.h
    ScreenResource.cc ScreenResource.h
    SDL_Audio.cc SDL_Audio.h
    SDL_Clock.cc SDL_Clock.h
    SDL_Toolkit.cc SDL_Toolkit.h
    SDL_Video.cc SDL_Video.h
    SkillLevelWidget.cc SkillLevelWidget.h
    SkillsWidget.cc SkillsWidget.h
    SolidPolygonObject.cc SolidPolygonObject.h
    Sound.cc Sound.h
    SoundResource.cc SoundResource.h
    SpriteObject.cc SpriteObject.h
    Statistics.cc Statistics.h
    Subject.cc Subject.h
    TableResource.cc TableResource.h
    TaggedImageResource.cc TaggedImageResource.h
    TaggedResource.cc TaggedResource.h
    TestApplication.cc TestApplication.h
    TextButtonWidget.cc TextButtonWidget.h
    Text.cc Text.h
    TextWidget.cc TextWidget.h
    TileDataResource.cc TileDataResource.h
    TileWorldResource.cc TileWorldResource.h
    Vertex.cc Vertex.h
    Video.cc Video.h
    Widget.cc Widget.h
    WidgetFactory.cc WidgetFactory.h
    WorldViewWidget.cc WorldViewWidget.h
    Zone.cc Zone.h
)

add_library(xbak ${SOURCE_FILES})
    
set_target_properties(xbak PROPERTIES CMAKE_CXX_CLANG_TIDY "")
target_link_libraries(xbak
    com
    glm
    SDL2
    SDL2_mixer_ext_Static
    ${LINK_UNIX_LIBRARIES}
)

# XBaK Executables
list(APPEND XBAK_BINARIES
    bmx2bmp
    dumpads
    dumpbmx
    dumpact_bmx
    dumpfnt
    dumpchar_bmx
    dumpzone_bmx
    dumppal
    dumpreq
    dumpobj
    dumptbl
    dumpscx
    dumpwld
    resourcedemo
)


foreach (XBAK_BIN IN LISTS XBAK_BINARIES)
    add_executable (${XBAK_BIN} ${XBAK_BIN}.cc ${XBAK})
    target_link_libraries(${XBAK_BIN}
        xbak
        ${LINK_UNIX_LIBRARIES}
    )
endforeach()

add_executable (xbak_main xbak.cc ${XBAK})
target_link_libraries(xbak_main
    xbak
    ${LINK_UNIX_LIBRARIES}
    )

set(CMAKE_CXX_CLANG_TIDY "")
