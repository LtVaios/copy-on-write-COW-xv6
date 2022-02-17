# ✔️ copy-on-write-xv6
xv6 is a re-implementation of Dennis Ritchie's and Ken Thompson's Unix
Version 6 (v6).  xv6 loosely follows the structure and style of v6,
but is implemented for a modern RISC-V multiprocessor using ANSI C.

# 🔨 Build / Run

You will need a RISC-V "newlib" tool chain from
https://github.com/riscv/riscv-gnu-toolchain, and qemu compiled for
riscv64-softmmu. Once they are installed, and in your shell
search path, you can run 
`make qemu`.

# 📝 Testing copy on write
After opening the emulator you can run: <br />
`usertests` <br />
`cowtest` <br />
which include various tests regarding the copy on write problem.

## Built with
  <img src="https://upload.wikimedia.org/wikipedia/commons/thumb/1/18/C_Programming_Language.svg/380px-C_Programming_Language.svg.png" style="height: 150px; width:130px;"/>
