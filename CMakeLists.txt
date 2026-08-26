cmake_minimum_required(VERSION 3.22)

set(CREATION_DEVELOPER_VERSION "0.1.0" CACHE STRING "Creation Developer product version")
project(CreationDeveloper VERSION ${CREATION_DEVELOPER_VERSION} LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT DEFINED JUCE_DIR)
    message(FATAL_ERROR "Set JUCE_DIR to the JUCE root before configuring Creation Developer.")
endif()

if(MSVC)
    string(APPEND CMAKE_CXX_FLAGS_RELEASE " /Zi")
    string(APPEND CMAKE_EXE_LINKER_FLAGS_RELEASE " /DEBUG /OPT:REF /OPT:ICF")
endif()

set(CREATION_SUITE_SHARED_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../shared" CACHE PATH "Path to the Creation Suite shared code root")

add_subdirectory("${JUCE_DIR}" juce)

foreach(component VFS AssetSystem Services UI)
    if(EXISTS "${CREATION_SUITE_SHARED_DIR}/${component}/CMakeLists.txt")
        string(TOLOWER "${component}" component_lower)
        add_subdirectory("${CREATION_SUITE_SHARED_DIR}/${component}"
                         "${CMAKE_CURRENT_BINARY_DIR}/CreationSuiteShared${component}")
    endif()
endforeach()

if(MSVC)
    # Shared static libraries compile in this app's build tree. /FS prevents
    # concurrent compiler processes from racing their target PDB files.
    foreach(target creation_suite_vfs creation_suite_asset_system creation_suite_services creation_suite_ui)
        if(TARGET ${target})
            target_compile_options(${target} PRIVATE /FS)
        endif()
    endforeach()
endif()

juce_add_gui_app(CreationDeveloperApp
    PRODUCT_NAME "Creation Developer"
    COMPANY_NAME "LagDaemon Software"
    BUNDLE_ID "com.lagdaemon.creationdeveloper"
    VERSION "${CREATION_DEVELOPER_VERSION}"
)

add_custom_command(TARGET CreationDeveloperApp POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "D:/CreationSuite-Workspaces/codex-$<LOWER_CASE:$<CONFIG>>-bin"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:CreationDeveloperApp>" "D:/CreationSuite-Workspaces/codex-$<LOWER_CASE:$<CONFIG>>-bin/"
    COMMENT "Copying CreationDeveloperApp to the shared Codex bin directory"
)
juce_generate_juce_header(CreationDeveloperApp)

target_sources(CreationDeveloperApp PRIVATE
    Source/Branding.cpp
    Source/Branding.h
    Source/Main.cpp
    Source/MainComponent.cpp
    Source/MainComponent.h
)

target_compile_definitions(CreationDeveloperApp PRIVATE
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_APPLICATION_NAME_STRING="$<TARGET_PROPERTY:CreationDeveloperApp,JUCE_PRODUCT_NAME>"
    JUCE_APPLICATION_VERSION_STRING="$<TARGET_PROPERTY:CreationDeveloperApp,JUCE_VERSION>"
)

if(MSVC)
    target_compile_options(CreationDeveloperApp PRIVATE /FS)
endif()

target_link_libraries(CreationDeveloperApp PRIVATE
    creation_suite_asset_system
    creation_suite_services
    creation_suite_ui
    juce::juce_gui_extra
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
)

install(TARGETS CreationDeveloperApp RUNTIME DESTINATION . COMPONENT CreationDeveloper)

set(CPACK_PACKAGE_NAME "Creation Developer")
set(CPACK_PACKAGE_VENDOR "LagDaemon Software")
set(CPACK_PACKAGE_VERSION "${CREATION_DEVELOPER_VERSION}")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "Creation Developer")
set(CPACK_GENERATOR "ZIP")
include(CPack)
