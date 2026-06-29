cmake_minimum_required(VERSION 3.10)
project(gameboy-emulator C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

find_package(SDL2 REQUIRED)

add_executable(gameboy
    src/gb.c
    src/cartridge.c
    src/memory.c
    src/clock.c
    src/interrupts.c
    src/dma.c
    src/joypad.c
    src/serial.c
    src/CPU/cpu.c
    src/CPU/registers.c
    src/PPU/ppu.c
    src/PPU/window.c
    src/APU/apu.c
    src/APU/samples.c
    src/dialogs.c
)

target_include_directories(gameboy PRIVATE src)
target_link_libraries(gameboy PRIVATE SDL2::SDL2)
target_link_libraries(gameboy PRIVATE m)
