#Link Directories
INCLUDE_DIRECTORIES(
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/..
)

#Get Files Lists
FILE(GLOB_RECURSE sources_dbo Dbo/*.cpp Dbo/*.h)
FILE(GLOB_RECURSE sources_application Application/*.cpp Application/*.h)
FILE(GLOB_RECURSE sources_widgets Widgets/*.cpp Widgets/*.h)
FILE(GLOB sources_localdir *.cpp *.h)
#Group File
SOURCE_GROUP(Dbo FILES ${sources_dbo})
SOURCE_GROUP(Application FILES ${sources_application})
SOURCE_GROUP(Widgets FILES ${sources_widgets})
SOURCE_GROUP("" FILES ${sources_localdir})

#Concatenate Files Lists
SET(Game_SRCS
	${sources_dbo}
	${sources_application}
	${sources_widgets}
	${sources_localdir}
)
#Set Target/Project name
SET(GAME_PROJECT_TARGET Game)

#Add Program CXX files
ADD_EXECUTABLE(${GAME_PROJECT_TARGET} ${Game_SRCS})

#Parallel compile
IF(MSVC)
	SET_TARGET_PROPERTIES(${GAME_PROJECT_TARGET} PROPERTIES COMPILE_FLAGS "${BUILD_PARALLEL} /bigobj")
ENDIF(MSVC)

#Link Libraries
TARGET_LINK_LIBRARIES(${GAME_PROJECT_TARGET}
	optimized ${Wt_LIBRARY} debug ${Wt_DEBUG_LIBRARY}
	optimized ${Wt_HTTP_LIBRARY} debug ${Wt_HTTP_DEBUG_LIBRARY}
	optimized ${Wt_DBO_LIBRARY} debug ${Wt_DBO_DEBUG_LIBRARY}
	optimized ${Wt_DBOSQLITE3_LIBRARY} debug ${Wt_DBOSQLITE3_DEBUG_LIBRARY}
)