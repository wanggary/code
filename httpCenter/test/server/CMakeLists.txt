#包含prep.cmake(定义编译时的选项)
INCLUDE($ENV{CMAKE}/common/prep.cmake)

#设置版本号，和向下兼容性
CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
CMAKE_POLICY(VERSION 2.8)

SET(NEED_CFL TRUE)
SET(NEED_NCHTTPSERVER TRUE)

#project name
PROJECT(testHttpServer)

#target name
SET(TARGET_NAME testHttpServer)

#TARGET_TYPE 
#LIBRARY | ARCHIVE | RUNTIME
#动态库  | 静态库  | 可执行
SET(TARGET_TYPE RUNTIME)

INCLUDE($ENV{CMAKE}/common/target.cmake)
