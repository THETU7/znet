if(ENABLETEST)

include(FetchContent)

FetchContent_Declare(
	googletest 
	URL https://github.com/google/googletest/archive/609281088cfefc76f9d0ce82e1ff6c30cc3591e5.zip
	)

FetchContent_MakeAvailable(googletest)

#enable_testing()

#include(GoogleTest)
foreach(target test01 test02 test03 test04 test05 test06 test07 test08 test09 test10 test11 test12 test13 daytimeTest daytimeServer pingpongServer pingpongClient)
add_executable(
	${target}
	${target}.cc
	)

target_link_libraries(
	${target}
	#	gtest_main
	reactor
	${Alog_LIBRARYS}
)

#gtest_discover_tests(${target})

endforeach()

enable_testing()
include(GoogleTest)
add_executable(buffertest testBuffer.cc)
target_link_libraries(buffertest gtest_main reactor ${Alog_LIBRARYS})
gtest_discover_tests(buffertest)


endif(ENABLETEST)
