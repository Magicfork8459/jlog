cmake_minimum_required(VERSION 3.29.2)

project(jlog)

find_package(Boost REQUIRED COMPONENTS log log_setup unit_test_framework json)

# Setup Library
    file(GLOB SOURCE
        ${PROJECT_SOURCE_DIR}/silver/jlog/*.cpp
        ${PROJECT_SOURCE_DIR}/silver/jlog/*.h
    )

    add_library(${PROJECT_NAME} ${SOURCE})

    target_include_directories(${PROJECT_NAME} PUBLIC
        ${Boost_INCLUDE_DIRS}  
        ${PROJECT_SOURCE_DIR}      
    )

    target_link_libraries(${PROJECT_NAME}
        ${Boost_LIBRARIES}
    )

    set_target_properties(${PROJECT_NAME} PROPERTIES
        LINKER_LANGUAGE CXX
        CXX_STANDARD 23
    )

if(${TESTS_ENABLED})
    enable_testing()
    add_subdirectory(tests)
endif()