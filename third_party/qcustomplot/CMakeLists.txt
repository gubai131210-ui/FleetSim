add_library(QCustomPlot STATIC
    qcustomplot.cpp
    qcustomplot.h
)

target_include_directories(QCustomPlot PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(QCustomPlot
    PUBLIC
        Qt6::Widgets
        Qt6::PrintSupport
)

target_compile_definitions(QCustomPlot PRIVATE QCUSTOMPLOT_COMPILE_LIBRARY)

if(MSVC)
    target_compile_options(QCustomPlot PRIVATE /wd4996 /bigobj)
else()
    # qcustomplot.cpp is a large amalgamated TU; MinGW COFF needs big-obj.
    target_compile_options(QCustomPlot PRIVATE -Wno-deprecated-declarations -Wa,-mbig-obj)
endif()
