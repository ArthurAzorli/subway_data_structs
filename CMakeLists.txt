cmake_minimum_required(VERSION 3.31)
project(Trabalho01 C)

set(CMAKE_C_STANDARD 11)

add_executable(Trabalho01
        project/core/utils/types.h
        project/core/utils/string.h
        project/core/file/file_repository.c
        project/core/file/file_repository.h
        project/domain/subway_record.h
        project/domain/subway_record.c
        project/service/data_base_repository.c
        project/service/data_base_repository.h
        project/service/record_repository.c
        project/service/record_repository.h
        project/service/header_repository.c
        project/service/header_repository.h
        main.c)
