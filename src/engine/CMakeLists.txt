# Engine CMakeLists.txt
add_library(morpheus-engine STATIC
    graph/csr_graph.cpp
    graph/graph_generator.cpp
    algorithms/bfs.cpp
    algorithms/pagerank.cpp
    algorithms/betweenness.cpp
    monitoring/performance_monitor.cpp
    ../utils/config_parser.cpp
)

target_include_directories(morpheus-engine
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_link_libraries(morpheus-engine
    PUBLIC
        Threads::Threads
)