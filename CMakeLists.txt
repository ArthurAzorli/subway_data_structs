cmake_minimum_required(VERSION 3.31)
project(Trabalho01 C)

set(CMAKE_C_STANDARD 11)

add_executable(Trabalho01
        project/lib/provided.h
        project/lib/provided.c
        project/program.h
        project/program.c
        main.c
        project/services/file/file_repository.c
        project/services/file/file_repository.h
        project/services/utils/split.h
        project/services/utils/types.h
        project/subway/subway_header_repository.c
        project/subway/subway_header_repository.h
        project/subway/subway_record_repository.c
        project/subway/subway_record_repository.h
        project/subway/input/input_repository.c
        project/subway/input/input_repository.h
        project/subway/search_criteria.h
        project/lib/subway_record_list.c
        project/lib/subway_record_list.h
)
