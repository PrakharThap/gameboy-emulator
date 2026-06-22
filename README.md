
# Game Boy Emulator

Simple emulator for the **DMG GameBoy** written in C. 


## Features

- Instruction level accurate CPU 
- Fully implemented PPU 
- Save/Load functionality
- Supports No MBC, MBC1
- Passes Blargg tests "cpu_instrs", "instr_timing", "interrupt_time", and "halt_bug".

## Controls:
- **Joypad**: Arrow Keys
- **B/A**: Z/X
- **Select/Start**: Backspace/Enter
- **All Key Press**: Spacebar

## Usage:

Either simply execute binary and follow terminal prompts or pass in optional flags -s/-l (save, load) along with desired ROM path.

**ROM Paths are expected in a ROMS/ directory and Save/Loads must be done in a ROMS/Saves/ directory.**
## Future Plans

- Add APU implementation.
- Create M-Cycle/T-Cycle level accuracy. 
- Add support for MBC2, MBC3, and MBC5.

