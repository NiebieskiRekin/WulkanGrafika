# Na bazie https://github.com/tgalaj/OpenGLSampleCmake/blob/master/CMakeLists.txt

### NIE DZIAŁA!!!

CMAKE_MINIMUM_REQUIRED( VERSION 3.22 )
project(grafika)


link_directories(${CMAKE_SOURCE_DIR}/Libraries)

# Add source files
file(GLOB_RECURSE SOURCE_FILES 
	${CMAKE_SOURCE_DIR}/*.c
	${CMAKE_SOURCE_DIR}/*.cpp)
	
# Add header files
file(GLOB_RECURSE HEADER_FILES 
	${CMAKE_SOURCE_DIR}/*.h
	${CMAKE_SOURCE_DIR}/*.hpp)

include_directories(
	"${CMAKE_SOURCE_DIR}/assimp"
	"${CMAKE_SOURCE_DIR}/glm"
	"${CMAKE_SOURCE_DIR}/glfw"
	"${CMAKE_SOURCE_DIR}/glew"
)

add_executable(${PROJECT_NAME} ${HEADER_FILES} ${SOURCE_FILES})

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

# set(OpenGL_GL_PREFERENCE GLVND)
# find_package(OpenGL REQUIRED)

# find_package(ASSIMP REQUIRED)
# message(STATUS "Found ASSIMP in ${ASSIMP_INCLUDE_DIR}")

# find_package(GLFW3 REQUIRED)
# message(STATUS "Found GLFW3 in ${GLFW3_INCLUDE_DIR}")

# find_package(GLM REQUIRED)
# message(STATUS "GLM included at ${GLM_INCLUDE_DIR}")

# find_package(GLEW REQUIRED)
# message(STATUS "Found GLEW in ${GLEW_INCLUDE_DIR}")


set(LIBS ${GLFW3_LIBRARY} ${OPENGL_LIBRARY} GLAD ${CMAKE_DL_LIBS} ${ASSIMP_LIBRARY} STB_IMAGE)

target_link_libraries(grafika ${LIBS})


# Create virtual folders to make it look nicer in VS
if(MSVC_IDE)
	# Macro to preserve source files hierarchy in the IDE
	macro(GroupSources curdir)
		file(GLOB children RELATIVE ${PROJECT_SOURCE_DIR}/${curdir} ${PROJECT_SOURCE_DIR}/${curdir}/*)

		foreach(child ${children})
			if(IS_DIRECTORY ${PROJECT_SOURCE_DIR}/${curdir}/${child})
				GroupSources(${curdir}/${child})
			else()
				string(REPLACE "/" "\\" groupname ${curdir})
				string(REPLACE "src" "Sources" groupname ${groupname})
				source_group(${groupname} FILES ${PROJECT_SOURCE_DIR}/${curdir}/${child})
			endif()
		endforeach()
	endmacro()

	# Run macro
	GroupSources(src)
endif()

# Copy dlls
if(WIN32)
	add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_directory
		"${PROJECT_SOURCE_DIR}/dlls"
		$<TARGET_FILE_DIR:${PROJECT_NAME}>)
endif()

## Stary makefile
# LIBS=-lGL -lglfw -lGLEW
# HEADERS=constants.h lodepng.h myCube.h myTeapot.h shaderprogram.h
# FILES=lodepng.cpp main_file.cpp shaderprogram.cpp
# main_file: $(FILES) $(HEADERS)
	# g++ -o main_file $(FILES)  $(LIBS) -I.
