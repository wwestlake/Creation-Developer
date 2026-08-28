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
set(FRUST_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third_party/FrustLang" CACHE PATH "Path to the FRust compiler source")

add_subdirectory("${JUCE_DIR}" juce)

foreach(component VFS AssetSystem Services UI)
    if(EXISTS "${CREATION_SUITE_SHARED_DIR}/${component}/CMakeLists.txt")
        string(TOLOWER "${component}" component_lower)
        add_subdirectory("${CREATION_SUITE_SHARED_DIR}/${component}"
                         "${CMAKE_CURRENT_BINARY_DIR}/CreationSuiteShared${component}")
    endif()
endforeach()

add_subdirectory("${CREATION_SUITE_SHARED_DIR}/JuceDocking"
                 "${CMAKE_CURRENT_BINARY_DIR}/CreationSuiteSharedJuceDocking")

if(NOT EXISTS "${FRUST_SOURCE_DIR}/projects/01_language_paradigms/02_functional/CMakeLists.txt")
    message(FATAL_ERROR "FRust source is required for Creation Developer. Initialize third_party/FrustLang.")
endif()
add_subdirectory("${FRUST_SOURCE_DIR}/projects/05_frate" "${CMAKE_CURRENT_BINARY_DIR}/FrustFrate")
add_subdirectory("${FRUST_SOURCE_DIR}/projects/01_language_paradigms/02_functional" "${CMAKE_CURRENT_BINARY_DIR}/FrustCompiler")
add_subdirectory("${FRUST_SOURCE_DIR}/projects/09_frust_plugin_host" "${CMAKE_CURRENT_BINARY_DIR}/FrustPluginHost")

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
    COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:CreationDeveloperApp>/plugins"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/BuiltInPlugins/SuitePluginPodCommands.frust" "$<TARGET_FILE_DIR:CreationDeveloperApp>/plugins/"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/BuiltInPlugins/FrustReplPlugin.frust" "$<TARGET_FILE_DIR:CreationDeveloperApp>/plugins/"
    COMMAND ${CMAKE_COMMAND} -E make_directory "D:/CreationSuite-Workspaces/codex-$<LOWER_CASE:$<CONFIG>>-bin"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:CreationDeveloperApp>" "D:/CreationSuite-Workspaces/codex-$<LOWER_CASE:$<CONFIG>>-bin/"
    COMMAND ${CMAKE_COMMAND} -E make_directory "D:/CreationSuite-Workspaces/codex-$<LOWER_CASE:$<CONFIG>>-bin/plugins"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/BuiltInPlugins/SuitePluginPodCommands.frust" "D:/CreationSuite-Workspaces/codex-$<LOWER_CASE:$<CONFIG>>-bin/plugins/"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/BuiltInPlugins/FrustReplPlugin.frust" "D:/CreationSuite-Workspaces/codex-$<LOWER_CASE:$<CONFIG>>-bin/plugins/"
    COMMENT "Copying CreationDeveloperApp to the shared Codex bin directory"
)
juce_generate_juce_header(CreationDeveloperApp)

target_sources(CreationDeveloperApp PRIVATE
    Source/Branding.cpp
    Source/Branding.h
    Source/Main.cpp
    Source/MainComponent.cpp
    Source/MainComponent.h
    Source/ConsolePanel.h
    Source/ConsolePanel.cpp
    Source/FrateTerminalPanel.h
    Source/FrateTerminalPanel.cpp
    Source/TerminalPanel.h
    Source/TerminalPanel.cpp
    Source/TerminalInstance.h
    Source/TerminalInstance.cpp
    Source/PersistentShellSession.h
    Source/PersistentShellSession.cpp
    Source/ShellTokeniser.h
    Source/ShellTokeniser.cpp
    Source/PluginPodWorkspace.h
    Source/PluginPodWorkspace.cpp
    Source/BuiltInPluginHost.h
    Source/BuiltInPluginHost.cpp
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
    CreationDock
    frust_lang
    frust_plugin_host
    juce::juce_gui_extra
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
)

install(TARGETS CreationDeveloperApp RUNTIME DESTINATION . COMPONENT CreationDeveloper)
install(FILES BuiltInPlugins/SuitePluginPodCommands.frust BuiltInPlugins/FrustReplPlugin.frust DESTINATION plugins COMPONENT CreationDeveloper)

set(CPACK_PACKAGE_NAME "Creation Developer")
set(CPACK_PACKAGE_VENDOR "LagDaemon Software")
set(CPACK_PACKAGE_VERSION "${CREATION_DEVELOPER_VERSION}")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "Creation Developer")
set(CPACK_GENERATOR "ZIP")
include(CPack)
