# Tests CMakeLists.txt
enable_testing()

add_executable(test_graph
    test_graph.cpp
)

add_executable(test_bfs
    test_bfs.cpp
)

add_executable(test_pagerank
    test_pagerank.cpp
)

add_executable(test_integration
    test_integration.cpp
)

add_executable(test_classifier_validation
    test_classifier_validation.cpp
)

# Link tests with engine
target_link_libraries(test_graph morpheus-engine)
target_link_libraries(test_bfs morpheus-engine)
target_link_libraries(test_pagerank morpheus-engine)
target_link_libraries(test_integration morpheus-engine)
target_link_libraries(test_classifier_validation morpheus-engine)

# Add tests
add_test(NAME test_graph COMMAND test_graph)
add_test(NAME test_bfs COMMAND test_bfs)
add_test(NAME test_pagerank COMMAND test_pagerank)
add_test(NAME test_integration COMMAND test_integration)
add_test(NAME test_classifier_validation COMMAND test_classifier_validation)