cmake_minimum_required(VERSION 2.8)

project(CmdLineParams)

add_executable(${PROJECT_NAME}Test
 main.cpp
 ctkCmdLineApplication.hxx
 ctkParam.hxx
 StringUtil.hxx
)
