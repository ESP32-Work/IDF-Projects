idf_component_register(SRCS "LED_Blink.c"
                    INCLUDE_DIRS ".")


# target_compile_options(${COMPONENT_LIB} PRIVATE --coverage)
# target_link_libraries(${COMPONENT_LIB} PRIVATE --coverage)
target_compile_options(${COMPONENT_LIB} PRIVATE -fprofile-arcs -ftest-coverage)
target_link_libraries(${COMPONENT_LIB} PRIVATE -lgcov)
