# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.18.1)

# Declares and names the project.

project("ffmpeg")

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.


#file(GLOB source_file ${CMAKE_SOURCE_DIR}/*.cpp)
#add_library(ffmpeg
#        SHARED
#        ${source_file})

add_library( # Sets the name of the library.
        ffmpeg
        # Sets the library as a shared library.
        SHARED
        # Provides a relative path to your source file(s).
        native-lib.cpp
        FFmpeg.h
        FFmpeg.cpp
        JvmCallHelper.h
        JvmCallHelper.cpp
        utils.h
        AudioChannel.h
        AudioChannel.cpp
        VideoChannel.h
        VideoChannel.cpp
        BaseChannel.h
        safe_queue.h
        )

# Searches for a specified prebuilt library and stores the path as a
# variable. Because CMake includes system libraries in the search path by
# default, you only need to specify the name of the public NDK library
# you want to add. CMake verifies that the library exists before
# completing its build.

find_library( # Sets the name of the path variable.
        log-lib

        # Specifies the name of the NDK library that
        # you want CMake to locate.
        log)

# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in this
# build script, prebuilt third-party libraries, or system libraries.
target_include_directories(ffmpeg PRIVATE
        ${CMAKE_SOURCE_DIR}/include)

target_link_libraries( # Specifies the target library.
        ffmpeg

        avcodec
        avfilter
        avformat
        avutil
        swresample
        swscale

        android
        OpenSLES
        # Links the target library to the log library
        # included in the NDK.
        ${log-lib})


set(libs ${CMAKE_SOURCE_DIR}/../jni)

add_library(avcodec SHARED IMPORTED)
set_target_properties(avcodec PROPERTIES IMPORTED_LOCATION
       ${libs}/${CMAKE_ANDROID_ARCH_ABI}/libavcodec.so)

add_library(avfilter SHARED IMPORTED)
set_target_properties(avfilter PROPERTIES IMPORTED_LOCATION
        ${libs}/${CMAKE_ANDROID_ARCH_ABI}/libavfilter.so)

add_library(avformat SHARED IMPORTED)
set_target_properties(avformat PROPERTIES IMPORTED_LOCATION
        ${libs}/${CMAKE_ANDROID_ARCH_ABI}/libavformat.so)

add_library(avutil SHARED IMPORTED)
set_target_properties(avutil PROPERTIES IMPORTED_LOCATION
        ${libs}/${CMAKE_ANDROID_ARCH_ABI}/libavutil.so)

add_library(swresample SHARED IMPORTED)
set_target_properties(swresample PROPERTIES IMPORTED_LOCATION
        ${libs}/${CMAKE_ANDROID_ARCH_ABI}/libswresample.so)


add_library(swscale SHARED IMPORTED)
set_target_properties(swscale PROPERTIES IMPORTED_LOCATION
        ${libs}/${CMAKE_ANDROID_ARCH_ABI}/libswscale.so)




