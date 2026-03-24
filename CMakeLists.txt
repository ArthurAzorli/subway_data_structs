cmake_minimum_required(VERSION 3.31)
project(Trabalho01 C)

set(CMAKE_C_STANDARD 11)
add_executable(Trabalho01
        main.c
        project/core/file/file_repository.c
        project/core/file/file_repository.h
        project/domain/subway_line_record.c
        project/domain/subway_line_record.h
        project/service/data_base_repository.c
        project/service/data_base_repository.h
        project/core/utils/types.h
)
