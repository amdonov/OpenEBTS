include_directories(/usr/include/nbis ../)
link_directories(/usr/lib64/nbis)
file(GLOB EBTS_SOURCES "*.cpp")
add_library(OpenEBTS SHARED ${EBTS_SOURCES})
target_link_libraries(OpenEBTS freeimage curl wsq ioutil jpegl fet util)
install(FILES OpenEBTS.h OpenEBTSErrors.h DESTINATION usr/include)
install(TARGETS OpenEBTS
	LIBRARY DESTINATION usr/lib64)
