cmake_minimum_required(VERSION 3.20)

project(libuev LANGUAGES C)

set(SOURCE_PATH "${PROJECT_SOURCE_DIR}/src" )
set(INCLUDE_PATH "${PROJECT_SOURCE_DIR}/src" )


file( GLOB_RECURSE HS "${INCLUDE_PATH}/*.h" )
file( GLOB_RECURSE CS "${SOURCE_PATH}/*.c" )

# add_library(objlib OBJECT ${HS} ${CS} uev.c)
add_library(objlib OBJECT ${HS} ${CS})

set_property(TARGET objlib PROPERTY POSITION_INDEPENDENT_CODE 1)

# shared and static libraries built from the same object files
add_library(libuev_shared SHARED $<TARGET_OBJECTS:objlib>)
add_library(libuev_static STATIC $<TARGET_OBJECTS:objlib>)
set_target_properties(libuev_shared PROPERTIES OUTPUT_NAME uev)
# target_sources(libuev_shared PUBLIC FILE_SET HEADERS FILES uev.h)
set_target_properties(libuev_static PROPERTIES OUTPUT_NAME uev)
set_target_properties(libuev_static PROPERTIES PUBLIC_HEADER ${PROJECT_SOURCE_DIR}/src/uev.h)
set_target_properties(libuev_shared PROPERTIES PUBLIC_HEADER ${PROJECT_SOURCE_DIR}/src/uev.h)
# target_sources(libuev_static PUBLIC FILE_SET HEADERS FILES uev.h)
install (TARGETS 
            libuev_shared
            # libuev_static
        LIBRARY           # Following options apply to library artifacts.
            DESTINATION lib
        ARCHIVE           # Following options apply to archive artifacts.
            DESTINATION lib
        # FILE_SET HEADERS  # Following options apply to file set HEADERS.
        #     COMPONENT Development
        CONFIGURATIONS 
            Release
        PUBLIC_HEADER 
            DESTINATION include
        RUNTIME
            DESTINATION lib
)
install (TARGETS 
            # libuev_shared
            libuev_static
        LIBRARY           # Following options apply to library artifacts.
            DESTINATION lib
        # ARCHIVE           # Following options apply to archive artifacts.
        #     DESTINATION lib
        # FILE_SET HEADERS  # Following options apply to file set HEADERS.
        #     COMPONENT Development
        # CONFIGURATIONS 
        #     Release
        PUBLIC_HEADER 
            DESTINATION include
        # RUNTIME
        #     DESTINATION Debug/lib
)
# install (TARGETS 
#             libuev_shared
#             libuev_static
#         CONFIGURATIONS 
#             Debug
#         RUNTIME
#             DESTINATION Release/lib
# )