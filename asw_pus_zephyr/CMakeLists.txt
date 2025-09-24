cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(asw_pus_zephyr)

# Desactiva la inicialización thread-safe de estáticos si no se enlaza el
# runtime de C++ (evita la dependencia de __cxa_guard_*)
set(NO_THREADSAFE_STATICS $<TARGET_PROPERTY:compiler-cpp,no_threadsafe_statics>)
zephyr_compile_options($<$<COMPILE_LANGUAGE:CXX>:${NO_THREADSAFE_STATICS}>)

zephyr_compile_options(-fno-sized-deallocation)

# Opción global para todo el proyecto
zephyr_compile_definitions(EDROOMBP_DEF)

# ---- Fuentes del proyecto ----
target_sources(app PRIVATE
  # Raíz
  ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/uah_asw_mmesp_project.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/service_libraries/emu_tc_programming/src/emu_tc_programming_st03.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/service_libraries/pus_services/pus_service12/src/aux_pus_service12_exec_tc.c
)

# -----------------------------
# Fuentes de la aplicación
# -----------------------------
file(GLOB_RECURSE APP_SOURCES
  ${CMAKE_CURRENT_LIST_DIR}/asw/**/*.c
  ${CMAKE_CURRENT_LIST_DIR}/asw/**/*.cpp
  ${CMAKE_CURRENT_LIST_DIR}/service_libraries/**/*.c
  ${CMAKE_CURRENT_LIST_DIR}/service_libraries/**/*.cpp
  ${CMAKE_CURRENT_LIST_DIR}/llsw/**/*.c
  ${CMAKE_CURRENT_LIST_DIR}/llsw/**/*.cpp
)

# Fuera FreeRTOS kernel
list(FILTER APP_SOURCES EXCLUDE REGEX ".*/llsw/freertos_osswr/.*")

# --- EXCLUSIONES CLAVE ---
list(FILTER APP_SOURCES EXCLUDE REGEX ".*/asw_pus_zephyr_innecesario/.*")
list(FILTER APP_SOURCES EXCLUDE REGEX ".*/service_libraries/pus_services/pus_service12/src/aux_pus_service12_exec_tc\\.c$")
list(FILTER APP_SOURCES EXCLUDE REGEX ".*/build/.*")

# Añade fuentes al target app
target_sources(app PRIVATE ${APP_SOURCES})

# -----------------------------
# Directorios de includes
# -----------------------------

# EDROOM (versión de service_libraries) primero para resolver <public/edroombp.h>
target_include_directories(app PRIVATE
  ${CMAKE_CURRENT_LIST_DIR}/service_libraries/edroomsl/edroombp/include
  ${CMAKE_CURRENT_LIST_DIR}/service_libraries/edroomsl/edroombp/include/public
)


# Recolecta TODOS los '.../include' del árbol (asw, llsw, service_libraries)
# IMPORTANTE: LIST_DIRECTORIES true para capturar directorios
file(GLOB_RECURSE APP_INCLUDE_DIRS LIST_DIRECTORIES true
  ${CMAKE_CURRENT_LIST_DIR}/asw/**/include
  ${CMAKE_CURRENT_LIST_DIR}/llsw/**/include
  ${CMAKE_CURRENT_LIST_DIR}/service_libraries/**/include
)

# Quedarse solo con rutas que acaban exactamente en /include (evita subcarpetas intermedias)
list(FILTER APP_INCLUDE_DIRS INCLUDE REGEX ".*/include$")

# Elimina duplicados por si hay solapes
list(REMOVE_DUPLICATES APP_INCLUDE_DIRS)

target_include_directories(app PRIVATE ${APP_INCLUDE_DIRS})

# ---- (Opcional) Forzar C++17 si se necesita ----
#target_compile_features(app PRIVATE cxx_std_17)

# Fuerza modo BLOQUEANTE (sin tarea de deployment separada)
target_compile_definitions(app PRIVATE
  CONFIG_EDROOMBP_DEPLOYMENT_NEED_TASK=0
  EDROOM_THREAD_STACK_SIZE=32768
  EDROOM_MAX_TASKS=8
)

# Si en algún momento quieres la tarea separada, pon =1
# target_compile_definitions(app PRIVATE CONFIG_EDROOMBP_DEPLOYMENT_NEED_TASK=1)



