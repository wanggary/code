#包含prep.cmake(定义编译时的选项)
INCLUDE($ENV{CMAKE}/common/prep.cmake)

#设置版本号，和向下兼容性
CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
CMAKE_POLICY(VERSION 2.8)

#project name 
PROJECT(httpCenter)

# 添加待编译目录
ADD_SUBDIRECTORY(httpServer)
ADD_SUBDIRECTORY(httpClient)

