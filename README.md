# Web68K — Simulator core

The **Motorola 68000 simulator core** behind [Web68K](https://web68k.kjorda.com/),
written in C and compiled to **WebAssembly** with [Emscripten](https://emscripten.org/).

This repository holds the low-level engine that actually executes 68000 machine code.
It is consumed by the Web68K web application, which lives in a separate repository:
**<https://github.com/kjorda02/Web68k-app>**. The web app loads the compiled
`cpu.wasm` produced here (together with the VASM assembler) and drives it from the
browser.

Both repositories were developed as part of the final degree project
*"Web68K: A web environment for simulating the Motorola 68000 processor"*.

## What's in here

- A complete **68000 CPU simulator** in C: fetch–decode–execute, all registers, the
  status register, big-endian memory, and the full set of addressing modes.
- **Debugging support** built into the engine: breakpoints and step-into / step-over /
  step-out (via a subroutine recursion counter).
- An **S-record loader** that parses the assembler's output and loads a program into
  the simulated RAM.
- A small **command-line harness** (`cli_emulator.c`) used during development to run an
  S-record file natively and print the resulting register state.
- The precompiled **VASM assembler** WebAssembly module used by the web app
  (`vasmm68k_motvasm.wasm`), see *Third-party components* below.

### Source layout

| File(s)                                                                                  | Purpose                                                        |
| ---------------------------------------------------------------------------------------- | ------------------------------------------------------------- |
| `cpu.c` / `cpu.h`                                                                         | Core CPU state, fetch/decode loop, memory, exported WASM API  |
| `instructions0_5.c`, `instructions4.c`, `instructions8_9_11.c`, `instructions12_13_14.c`, `instructions_misc.c`, `instructions.h` | Implementation of the instruction set, grouped by opcode |
| `process_srec.c` / `process_srec.h`                                                       | S-record parsing and program loading                          |
| `util.c` / `util.h`                                                                       | Helpers (flag setting, sign extension, byte-order handling…)  |
| `cli_emulator.c` / `cli_emulator.h`, `colors.h`                                           | Native CLI test harness and terminal formatting               |
| `makefile`                                                                                | Emscripten build → `cpu.js` + `cpu.wasm`                       |
| `wasmTest/`                                                                               | Minimal standalone HTML/JS page for testing the wasm modules  |
| `doc/`                                                                                    | Project notes (feature checklist, report outline)             |
| `cpu.wasm`, `cpu.js`, `vasmm68k_motvasm.wasm`                                             | Prebuilt artifacts (committed for convenience)                |

## Requirements

- The **[Emscripten SDK](https://emscripten.org/docs/getting_started/downloads.html)**
  (`emcc`) — this is the compiler used by the `makefile`.
- `make` and a Unix-like shell.
- The [VASM](http://sun.hasenbraten.de/vasm/) assembler (`vasm_m68k_mot`) on your `PATH`
  if you want the `make` target to assemble the bundled `prueba.X68` sample after
  building.

> The CPU source includes `<emscripten.h>`, so it is built with `emcc` rather than a
> native compiler. (During development a native build via GCC/GDB was used for debugging;
> reproducing that requires the Emscripten headers on the include path, or temporarily
> guarding the `emscripten.h` include.)

## Building

Compile the simulator to WebAssembly:

```bash
make
```

This runs `emcc` over the C sources and produces **`cpu.js`** and **`cpu.wasm`**.
Functions are exported to the WebAssembly module via the `EMSCRIPTEN_KEEPALIVE`
attribute (see `cpu.c`), so no explicit `EXPORTED_FUNCTIONS` list is needed.

To clean build artifacts:

```bash
make clean
```

### Using the build in the web app

The web application loads `cpu.wasm` from its own `static/` directory. After rebuilding
here, copy the result into the app repository:

```bash
cp cpu.wasm ../Web68k-app/static/cpu.wasm
```

## Exported WebAssembly interface

The web app calls the following functions, exported from `cpu.wasm`. They are documented
in full in the project's report; in summary:

| Function                              | Description                                                        |
| ------------------------------------- | ------------------------------------------------------------------ |
| `initCpu()`                           | Reset registers and RAM to zero                                    |
| `load_program(char* srec)`            | Parse an S-record string and load it; returns the entry address    |
| `run_burst(int cycles, int mode)`     | Run a burst of instructions in `RUN`/`STEP_INTO`/`STEP_OVER`/`STEP_OUT` mode |
| `step_forwards()`                     | Execute a single instruction (even on a breakpoint)               |
| `set_breakpoint(uint32_t addr, bool)` | Enable/disable a breakpoint at an address                          |
| `read_mem_window(pos, size)`          | Return a pointer to a window of simulated memory                   |
| `write_mem(pos, size, data)`          | Write a byte/word/long to memory                                   |
| `read_D_regs()` / `read_A_regs()`     | Return pointers to the data/address register arrays               |
| `read_Dn`/`read_An`/`write_Dn`/`write_An` | Read/write an individual data or address register             |
| `read_pc` / `write_pc`                | Read/write the program counter                                     |
| `read_sr` / `write_sr`                | Read/write the status register                                     |
| `read_cycles()`                       | Read the cycle counter                                             |
| `wasm_malloc` / `wasmfree`            | Allocate/free memory inside the module (e.g. for passing strings)  |

## Command-line harness

`cli_emulator.c` provides a `main()` that loads an S-record file, runs it to completion,
and prints the final register state to the terminal — handy for testing the engine
outside the browser. It is the entry point referenced by `makefile_old`.

## Third-party components

This repository bundles a precompiled WebAssembly build of the **VASM** assembler
(`vasmm68k_motvasm.wasm`, `noexit/vasmm68k_motvasm.*`). VASM is copyright Volker
Barthelmann and Frank Wille and is distributed under its **own license** (free for
personal and educational use), which is separate from this project's MIT license. Its
C source is not included here; see <http://sun.hasenbraten.de/vasm/> for the assembler
and its terms.

## License

The simulator's own source code is released under the **MIT License** — see the
[`LICENSE`](./LICENSE) file. The bundled VASM WebAssembly module is the exception noted
above and remains under VASM's own license.
