cmake_minimum_required(VERSION 4.2)
project(Working_Repo)

set(CMAKE_CXX_STANDARD 14)

include_directories(CCAnr_code)

add_executable(Working_Repo
        CCAnr_code/basis.h
        CCAnr_code/cca.cpp
        CCAnr_code/cca.h
        CCAnr_code/cw.h
        CCAnr_code/preprocessor.h
        CCAnr_code/mab.h
        CCAnr_code/contribs.h)
