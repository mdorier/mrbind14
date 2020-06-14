add_executable(interpreter_test main.cpp interpreter_test.cpp)
target_link_libraries(interpreter_test ${Mruby_LIBRARIES} ${CPPUNIT_LIBRARIES})
add_test(NAME interpreter_test COMMAND ./interpreter_test interpreter_test.xml)

add_executable(function_test main.cpp function_test.cpp)
target_link_libraries(function_test ${Mruby_LIBRARIES} ${CPPUNIT_LIBRARIES})
add_test(NAME function_test COMMAND ./function_test function_test.xml)

add_executable(module_test main.cpp module_test.cpp)
target_link_libraries(module_test ${Mruby_LIBRARIES} ${CPPUNIT_LIBRARIES})
add_test(NAME module_test COMMAND ./module_test module_test.xml)
