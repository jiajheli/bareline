# Bareline
Bareline is a command line interface (CLI) framework. It doesn't depend on other libraries. It doesn't allocate from data or BSS sections. It can be easily brought up on bare metal environments.

## Highlights
* Easy to port
* Line editing
* History
* Extensible command
* Complete
* Hint
* VT100 compatible

## Tested environments
* GCC 5.4.0 + Ubuntu 16.04.5 LTS
* GCC 4.8.5 + Proprietary MIPS board
* GCC 8.25 + Raspberry Pi 3 B+

### Build and run example for Ubuntu
```bash
make PLAT=plat_ubuntu all
./bin/demo.out
```
### Build example for MIPS
`plat_mips` assumes an 16550 compatible UART, and RAM starts from 0x80000000.
```bash
CROSS_COMPILE=<prefix of mips toolkit> UART_BASE_ADDR=<base addr.> make PLAT=plat_mips all
```
Then load `./bin/demo.out` to run.

### Build example for RPi3
```bash
make PLAT=plat_rpi3 all
```
Then load `./bin/demo.out` to RPi3 to run.

## Porting Bareline
Bareline has no dependency to other libraries. This makes porting Bareline fairly easy. Implementing your own `bl_putc()` and `bl_getc()` is enough to make Bareline works.
To start Bareline, one just invokes `bl_main_loop()` with buffer pointer, buffer size, and line length. Bareline doesn't allocate from data and BSS sections. It relies on its caller to provide necessary memory space.
Check `./plat_ubuntu/bareline_example.c` for an example.

## Operations
Bareline supports moving cursor left and right. Backspace, delete, and insert characters are also supported.
Every entered line is kept in a circular history buffer. One can traverse available history lines with `up` and `down`.
Bareline comes with some common key bindings:

| Key | Description |
| --- | --- |
| `ctrl` + `a` | move cursor to head of line |
| `ctrl` + `b` | move cursor left |
| `ctrl` + `c` | exit Bareline |
| `ctrl` + `d` | delete char. to the right of cursor |
| `ctrl` + `e` | move cursor to end of line |
| `ctrl` + `f` | move cursor right |
| `ctrl` + `n` | get newer history |
| `ctrl` + `p` | get older history |

## Extensible command
Bareline provides a macro `BL_REG_CMD()` to expand commands in Bareline. For example,
```c
BL_REG_CMD(exit, do_exit, "exit bareline");
```
registers a command, `exit`, to Bareline. When `exit` is entered, Bareline invokes `do_exit()` for response. And finally, `"exit bareline"` is the message to be showed for `help` command. Check `./src/bareline_builtin.c` for more examples.

## Complete and hint
Bareline supports complete and hint when typing a command, as long as the command is registered by `BL_REG_CMD()`.

Given a partial command followed by `enter`, Bareline executes it directly if it can be distinguished. Otherwise Bareline does nothing.

Given a partial command followed by `tab`, Bareline completes it in line buffer if it can be distinguished and unique. If it is ambiguous, Bareline lists all candidates. Otherwise Bareline does nothing.

## Acknowledgments
Bareline uses `printf` by [mpaland/printf](https://github.com/mpaland/printf)@21eea6c.

Bareline uses `xmodem` by [Thuffir/xmodem](https://github.com/Thuffir/xmodem)@8ca6c42.
