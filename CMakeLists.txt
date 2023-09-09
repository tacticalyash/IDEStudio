cmake_minimum_required(VERSION 3.20)
project(KMTextEditor VERSION 1.0 LANGUAGES CXX)

add_compile_options(-Wall -Wextra -Wpedantic)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

#if you are using ui files then uncomment it
#set(CMAKE_AUTOUIC ON)

find_package(Qt6 COMPONENTS Widgets REQUIRED)
add_executable(
    ${PROJECT_NAME}     #binary name

    CppCompletionModel.cpp  #sources
    MainWindow.cpp
    NewFileDialog.cpp
    NewProjectDialog.cpp
    ProjectExplorer.cpp
    ProjectModel.cpp
    ProjectTab.cpp
    SyntaxHighlighter.cpp
    TextEditor.cpp
    main.cpp

    CppCompletionModel.hpp    #headers
    MainWindow.hpp
    NewFileDialog.hpp
    NewProjectDialog.hpp
    ProjectDetails.hpp
    ProjectExplorer.hpp
    ProjectModel.hpp
    ProjectTab.hpp
    SyntaxHighlighter.hpp
    TextEditor.hpp

    KMX/kmx.cpp
    KMX/kmx.hpp
)
target_link_libraries(
    ${PROJECT_NAME}
    PRIVATE Qt6::Widgets
)
