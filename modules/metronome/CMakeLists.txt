set(HDRS
include/AppSettings.hpp
include/BarPlanner.hpp
include/BeatRowWidget.hpp
include/ClickConfig.hpp
include/ClickGenerator.hpp
include/ClickSynth.hpp
include/Controls.hpp
include/LibraryStore.hpp
include/LibraryTab.hpp
include/MainWindow.hpp
include/MetronomeEngine.hpp
include/MetronomeTab.hpp
include/PendulumWidget.hpp
include/PhaseChime.hpp
include/PlayButton.hpp
include/PracticeSession.hpp
include/PracticeTab.hpp
include/RepeatButton.hpp
include/SleepInhibitor.hpp
include/SettingsTab.hpp
include/StatsStore.hpp
include/SubdivisionSelector.hpp
include/TapTempo.hpp
include/TempoPanel.hpp
include/Texts.hpp
)

set(SRCS
src/AppSettings.cpp
src/BarPlanner.cpp
src/BeatRowWidget.cpp
src/ClickConfig.cpp
src/ClickGenerator.cpp
src/ClickSynth.cpp
src/Controls.cpp
src/LibraryStore.cpp
src/LibraryTab.cpp
src/MainWindow.cpp
src/MetronomeEngine.cpp
src/MetronomeTab.cpp
src/PendulumWidget.cpp
src/PhaseChime.cpp
src/PlayButton.cpp
src/PracticeSession.cpp
src/PracticeTab.cpp
src/RepeatButton.cpp
src/SleepInhibitor.cpp
src/SettingsTab.cpp
src/StatsStore.cpp
src/SubdivisionSelector.cpp
src/TapTempo.cpp
src/TempoPanel.cpp
)

# Icono de la aplicación (macOS: .icns dentro del bundle; Windows: .ico vía .rc)
set(APP_ICON_ICNS ${CMAKE_CURRENT_SOURCE_DIR}/resources/icons/metronome.icns)
set(APP_ICON_ICO ${CMAKE_CURRENT_SOURCE_DIR}/resources/icons/metronome.ico)

qt_add_executable(metrodoro WIN32 MACOSX_BUNDLE
    ${HDRS}
    ${SRCS}
    src/main.cpp
)

if(APPLE)
    target_sources(metrodoro PRIVATE ${APP_ICON_ICNS})
    set_source_files_properties(${APP_ICON_ICNS} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
    set_target_properties(metrodoro PROPERTIES
        MACOSX_BUNDLE_ICON_FILE metronome.icns
        MACOSX_BUNDLE_GUI_IDENTIFIER dev.sigifredo.metrodoro
        MACOSX_BUNDLE_BUNDLE_NAME Metrodoro
        MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
        MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION}
        MACOSX_BUNDLE_COPYRIGHT "Metrodoro"
        MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/resources/Info.plist.in)
elseif(WIN32)
    set(APP_RC ${CMAKE_CURRENT_BINARY_DIR}/metronome_icon.rc)
    file(WRITE ${APP_RC} "IDI_ICON1 ICON \"${APP_ICON_ICO}\"\n")
    target_sources(metrodoro PRIVATE ${APP_RC})
endif()

qt_add_resources(metrodoro "metronome_resources"
    PREFIX "/metronome"
    BASE resources
    FILES
        resources/icons/noires.png
        resources/icons/croches.png
        resources/icons/doubles.png
        resources/icons/triolets.png
        resources/icons/shuffle.png
        resources/icons/metronome.svg
        resources/style/style.qss
        resources/fonts/Hack-Regular.ttf
        resources/fonts/Hack-Bold.ttf
        resources/fonts/Inter-Regular.ttf
        resources/fonts/Inter-Medium.ttf
        resources/fonts/Inter-SemiBold.ttf
)

target_compile_definitions(metrodoro PRIVATE METRODORO_VERSION="${PROJECT_VERSION}")

target_include_directories(metrodoro PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(metrodoro PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Multimedia
    Qt6::Svg
)

if(APPLE)
    target_link_libraries(metrodoro PRIVATE "-framework IOKit" "-framework CoreFoundation")
endif()

if(UNIX AND NOT APPLE)
    target_link_libraries(metrodoro PRIVATE Qt6::DBus)
    target_compile_definitions(metrodoro PRIVATE METRODORO_HAS_DBUS)
endif()
