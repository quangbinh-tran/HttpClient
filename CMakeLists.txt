cmake_minimum_required(VERSION 3.16)

project(HttpClient
    VERSION 0.1.0
    DESCRIPTION "A small C++17 HTTP client built on libcurl"
    LANGUAGES CXX
)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)
include(CTest)

option(HTTPCLIENT_BUILD_EXAMPLE "Build the HTTP client example" ON)

find_package(CURL REQUIRED)

add_library(http_client
    src/CurlTransport.cpp
    src/HttpClient.cpp
)

add_library(HttpClient::http_client ALIAS http_client)

target_compile_features(http_client PUBLIC cxx_std_17)

target_include_directories(http_client
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/http_client>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/http_client>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(http_client PUBLIC CURL::libcurl)

if(HTTPCLIENT_BUILD_EXAMPLE)
    add_executable(http_client_example
        main.cpp
    )

    target_compile_features(http_client_example PRIVATE cxx_std_17)
    target_link_libraries(http_client_example PRIVATE HttpClient::http_client)
endif()

if(BUILD_TESTING)
    find_package(GTest CONFIG REQUIRED)
    find_package(Threads REQUIRED)

    add_executable(http_client_tests
        test/HttpClientTest.cpp
    )

    target_compile_features(http_client_tests PRIVATE cxx_std_17)
    target_link_libraries(http_client_tests
        PRIVATE
            HttpClient::http_client
            GTest::gtest_main
            Threads::Threads
    )

    include(GoogleTest)
    gtest_discover_tests(http_client_tests)
endif()

install(
    TARGETS http_client
    EXPORT HttpClientTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(
    DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(
    EXPORT HttpClientTargets
    FILE HttpClientTargets.cmake
    NAMESPACE HttpClient::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/HttpClient
)

configure_package_config_file(
    cmake/HttpClientConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/HttpClientConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/HttpClient
)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/HttpClientConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(
    FILES
        ${CMAKE_CURRENT_BINARY_DIR}/HttpClientConfig.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/HttpClientConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/HttpClient
)
