# Copyright 2019 Proyectos y Sistemas de Mantenimiento SL (eProsima).
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

###########################################################################
# Create and link executable                                              #
###########################################################################
set(
    THROUGHPUTTEST_SOURCE ThroughputPublisher.cpp
    ThroughputSubscriber.cpp
    ThroughputTypes.cpp
    main_ThroughputTest.cpp
)
add_executable(ThroughputTest ${THROUGHPUTTEST_SOURCE})

target_compile_definitions(ThroughputTest PRIVATE
    BOOST_ASIO_STANDALONE
    ASIO_STANDALONE
    $<$<AND:$<NOT:$<BOOL:${WIN32}>>,$<STREQUAL:"${CMAKE_BUILD_TYPE}","Debug">>:__DEBUG>
    $<$<BOOL:${INTERNAL_DEBUG}>:__INTERNALDEBUG> # Internal debug activated.
    )

target_include_directories(ThroughputTest PRIVATE ${Asio_INCLUDE_DIR})

target_link_libraries(
    ThroughputTest
    fastdds
    fastcdr
    foonathan_memory
    fastdds::optionparser
    ${CMAKE_THREAD_LIBS_INIT}
    ${CMAKE_DL_LIBS}
)

###########################################################################
# List Throughput tests                                                   #
###########################################################################
set(
    THROUGHPUT_TEST_LIST
    intraprocess_best_effort_profile
    intraprocess_reliable_profile
    interprocess_best_effort_udp_profile
    interprocess_reliable_udp_profile
#   interprocess_best_effort_tcp_profile
#   interprocess_reliable_tcp_profile
    interprocess_best_effort_shm_profile
    interprocess_reliable_shm_profile
)

###########################################################################
# List of tests supporting specific features                              #
# Each entry in this list means a specific test case added                #
###########################################################################

set(
    DATA_SHARING_LIST
    intraprocess_best_effort_profile
    intraprocess_reliable_profile
    interprocess_best_effort_shm_profile
    interprocess_reliable_shm_profile
)

set(
    LOAN_SAMPLES_LIST
    intraprocess_best_effort_profile
    intraprocess_reliable_profile
    interprocess_best_effort_udp_profile
    interprocess_reliable_udp_profile
#   interprocess_best_effort_tcp_profile
#   interprocess_reliable_tcp_profile
    interprocess_best_effort_shm_profile
    interprocess_reliable_shm_profile
)

set(
    DATA_SHARING_AND_LOAN_SAMPLES_LIST
    intraprocess_best_effort_profile
    intraprocess_reliable_profile
    interprocess_best_effort_shm_profile
    interprocess_reliable_shm_profile
)

###########################################################################
# Configure XML files                                                     #
###########################################################################
foreach(throughput_test_name ${THROUGHPUT_TEST_LIST})
    configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/xml/${throughput_test_name}.xml
        ${CMAKE_CURRENT_BINARY_DIR}/xml/${throughput_test_name}.xml
        COPYONLY
    )
endforeach(throughput_test_name)

###########################################################################
# Create tests                                                            #
###########################################################################
find_package(Python3 COMPONENTS Interpreter REQUIRED)
if(Python3_Interpreter_FOUND)
    # Loop over the test names
    foreach(throughput_test_name ${THROUGHPUT_TEST_LIST})

        # decide if add security testing
        set(ADD_THROUGHPUT_SECURITY OFF)
        if(SECURITY AND (${throughput_test_name} MATCHES "^interprocess"))
            set(ADD_THROUGHPUT_SECURITY ON)
        endif()

        # list of all the test cases generated in this iteration
        set(test_cases_setup performance.throughput.${throughput_test_name})

        # Set the interprocess flag
        if(${throughput_test_name} MATCHES "^interprocess")
            set(interproces_flag "--interprocess")
        else()
            set(interproces_flag "")
        endif()

        # Set the reliability flag
        if(${throughput_test_name} MATCHES "reliable")
            set(reliability_flag "--reliability")
        else()
            set(reliability_flag "")
        endif()

        # Add the test
        add_test(
            NAME performance.throughput.${throughput_test_name}
            COMMAND ${Python3_EXECUTABLE}
            ${CMAKE_CURRENT_SOURCE_DIR}/throughput_tests.py
            --xml_file ${CMAKE_CURRENT_SOURCE_DIR}/xml/${throughput_test_name}.xml
            --recoveries_file ${CMAKE_CURRENT_SOURCE_DIR}/recoveries.csv
            --demands_file ${CMAKE_CURRENT_SOURCE_DIR}/payloads_demands.csv
            ${interproces_flag}
            ${reliability_flag}
        )

        # Add environment
        if(WIN32)
            set(WIN_PATH "$ENV{PATH}")
            get_target_property(LINK_LIBRARIES_ ${PROJECT_NAME} LINK_LIBRARIES)
            if(NOT "${LINK_LIBRARIES_}" STREQUAL "LINK_LIBRARIES_-NOTFOUND")
            list(APPEND LINK_LIBRARIES_ ${PROJECT_NAME})
                foreach(LIBRARY_LINKED ${LINK_LIBRARIES_})
                    if(TARGET ${LIBRARY_LINKED})
                        # Check if is a real target or a target interface
                        get_target_property(dependency_type ${LIBRARY_LINKED} TYPE)
                        if(NOT dependency_type STREQUAL "INTERFACE_LIBRARY")
                            set(WIN_PATH "$<TARGET_FILE_DIR:${LIBRARY_LINKED}>;${WIN_PATH}")
                        endif()
                        unset(dependency_type)
                    endif()
                endforeach()
            endif()
            string(REPLACE ";" "\\;" WIN_PATH "${WIN_PATH}")
        endif()

        if(ADD_THROUGHPUT_SECURITY)

            # Add the secure version
            list(APPEND test_cases_setup performance.throughput.${throughput_test_name}.security)

            add_test(
                NAME performance.throughput.${throughput_test_name}.security
                COMMAND ${Python3_EXECUTABLE}
                ${CMAKE_CURRENT_SOURCE_DIR}/throughput_tests.py
                --xml_file ${CMAKE_CURRENT_SOURCE_DIR}/xml/${throughput_test_name}.xml
                --recoveries_file ${CMAKE_CURRENT_SOURCE_DIR}/recoveries.csv
                --demands_file ${CMAKE_CURRENT_SOURCE_DIR}/payloads_demands.csv
                --security
                ${interproces_flag}
                ${reliability_flag}
            )

            # Hint certificates location
            set_property(
                TEST performance.throughput.${throughput_test_name}.security
                APPEND PROPERTY ENVIRONMENT "CERTS_PATH=${PROJECT_SOURCE_DIR}/test/certs"
            )

        endif()

        # Check if a data sharing test is required
        if(throughput_test_name IN_LIST DATA_SHARING_LIST)

            # append to the list of cases
            list(APPEND test_cases_setup performance.throughput.${throughput_test_name}.data_sharing)

            add_test(
                NAME performance.throughput.${throughput_test_name}.data_sharing
                COMMAND ${Python3_EXECUTABLE}
                ${CMAKE_CURRENT_SOURCE_DIR}/throughput_tests.py
                --xml_file ${CMAKE_CURRENT_SOURCE_DIR}/xml/${throughput_test_name}.xml
                --recoveries_file ${CMAKE_CURRENT_SOURCE_DIR}/recoveries.csv
                --demands_file ${CMAKE_CURRENT_SOURCE_DIR}/payloads_demands.csv
                --data_sharing=on
                ${interproces_flag}
                ${reliability_flag}
            )

        endif()

        # Check if a loans test is required
        if(throughput_test_name IN_LIST LOAN_SAMPLES_LIST)

            # append to the list of cases
            list(APPEND test_cases_setup performance.throughput.${throughput_test_name}.data_loans)

            add_test(
                NAME performance.throughput.${throughput_test_name}.data_loans
                COMMAND ${Python3_EXECUTABLE}
                ${CMAKE_CURRENT_SOURCE_DIR}/throughput_tests.py
                --xml_file ${CMAKE_CURRENT_SOURCE_DIR}/xml/${throughput_test_name}.xml
                --recoveries_file ${CMAKE_CURRENT_SOURCE_DIR}/recoveries.csv
                --demands_file ${CMAKE_CURRENT_SOURCE_DIR}/payloads_demands.csv
                --data_loans
                ${interproces_flag}
                ${reliability_flag}
            )

            if(ADD_THROUGHPUT_SECURITY)

                # Add the secure version
                list(APPEND test_cases_setup performance.throughput.${throughput_test_name}.data_loans.security)

                add_test(
                    NAME performance.throughput.${throughput_test_name}.data_loans.security
                    COMMAND ${Python3_EXECUTABLE}
                    ${CMAKE_CURRENT_SOURCE_DIR}/throughput_tests.py
                    --xml_file ${CMAKE_CURRENT_SOURCE_DIR}/xml/${throughput_test_name}.xml
                    --recoveries_file ${CMAKE_CURRENT_SOURCE_DIR}/recoveries.csv
                    --demands_file ${CMAKE_CURRENT_SOURCE_DIR}/payloads_demands.csv
                    --security
                    ${interproces_flag}
                    --data_loans
                    ${reliability_flag}
                )

                # Hint certificates location
                set_property(
                    TEST performance.throughput.${throughput_test_name}.data_loans.security
                    APPEND PROPERTY ENVIRONMENT "CERTS_PATH=${PROJECT_SOURCE_DIR}/test/certs"
                )

            endif()

        endif()

        # Check if a test using data sharing and loans is required
        if(throughput_test_name IN_LIST DATA_SHARING_AND_LOAN_SAMPLES_LIST)

            # append to the list of cases
            list(APPEND test_cases_setup performance.throughput.${throughput_test_name}.data_loans_and_sharing)

            add_test(
                NAME performance.throughput.${throughput_test_name}.data_loans_and_sharing
                COMMAND ${Python3_EXECUTABLE}
                ${CMAKE_CURRENT_SOURCE_DIR}/throughput_tests.py
                --xml_file ${CMAKE_CURRENT_SOURCE_DIR}/xml/${throughput_test_name}.xml
                --recoveries_file ${CMAKE_CURRENT_SOURCE_DIR}/recoveries.csv
                --demands_file ${CMAKE_CURRENT_SOURCE_DIR}/payloads_demands.csv
                --data_loans
                --data_sharing=on
                ${interproces_flag}
                ${reliability_flag}
            )

        endif()

        # populate the properties for each test
        foreach(throughput_test_case ${test_cases_setup})

            # Set test properties
            set_property(
                TEST ${throughput_test_case}
                PROPERTY LABELS "NoMemoryCheck"
            )
            set_property(
                TEST ${throughput_test_case}
                APPEND PROPERTY ENVIRONMENT "THROUGHPUT_TEST_BIN=$<TARGET_FILE:ThroughputTest>"
            )
            set_property(
                TEST ${throughput_test_case}
                APPEND PROPERTY ENVIRONMENT "CMAKE_CURRENT_SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}"
            )

            if(WIN32)
                set_property(
                    TEST ${throughput_test_case}
                    APPEND PROPERTY ENVIRONMENT "PATH=${WIN_PATH}")
            endif()

        endforeach(throughput_test_case)

    endforeach(throughput_test_name)
endif()
