# C-Shell — Modular Unix-like Shell in C

A custom Unix-like shell built in **C**, supporting command execution, foreground/background process handling, piping, redirection, shell logging, and modular built-in utilities.

## Overview

This project implements a lightweight Unix-like shell from scratch with a focus on:

- command parsing and execution
- foreground and background process handling
- piping and I/O redirection
- shell prompt rendering
- persistent command logging
- custom built-in commands
- shell customization through rc-style configuration

The shell is organized modularly so that parsing, execution, prompting, logging, and built-ins are separated across source files.

## Features

### Core Shell Features
- interactive shell prompt
- execution of standard Unix commands
- foreground and background process support
- signal-aware shell behavior
- modular command dispatch architecture

### I/O Features
- command parsing and tokenization
- piping support
- input redirection (`<`)
- output overwrite redirection (`>`)
- output append redirection (`>>`)

### Built-in Utilities
- `seek` — file and directory search utility
- `proclore` — process inspection utility
- `neonate` — monitoring utility
- `iman` — custom manual/help command
- shell logging support
- shell rc configuration via `myshrc`

## Project Structure

```text
.
├── Makefile
├── readme.md
└── src
    ├── main.c
    ├── prompt.c / prompt.h
    ├── inputhandler.c / inputhandler.h
    ├── commandhandler.c / commandhandler.h
    ├── color.c / color.h
    ├── log.c / log.h
    ├── proclore.c / proclore.h
    ├── seek.c / seek.h
    ├── myshrc.c / myshrc.h
    ├── neonate.c / neonate.h
    └── iman.c / iman.h
```

## Prerequisites

You need:

- `gcc`
- `make`
- `libcurl` development package

### Ubuntu / Debian
```bash
sudo apt update
sudo apt install build-essential libcurl4-openssl-dev
```

### Fedora
```bash
sudo dnf install gcc make libcurl-devel
```

### Arch Linux
```bash
sudo pacman -S base-devel curl
```

## Build Instructions

Clone the repository:

```bash
git clone https://github.com/garimamittal13/C-shell.git
cd C-shell
```

Build the shell:

```bash
make
```

This creates the executable:

```bash
./my_shell
```

## How to Run the Shell

After building the project, start the shell by running:

```bash
./my_shell
```

You should now see the custom shell prompt and can start typing commands.

### Example Session
```bash
./my_shell
pwd
ls
echo hello
```

### Running Commands in Background
```bash
sleep 10 &
```

### Using Pipes
```bash
ls | wc
cat file.txt | grep hello
```

### Using Redirection
```bash
echo hello > out.txt
sort < input.txt
cat file.txt >> log.txt
```

### Using Built-in Commands
```bash
seek filename
proclore
neonate
iman seek
```

## Cleaning Build Files

To remove object files and the compiled executable:

```bash
make clean
```

## Design Highlights

- **Modular architecture:** parsing, execution, prompting, logging, and built-ins are implemented as separate modules.
- **Extensible built-ins:** custom shell commands are isolated cleanly, making the shell easier to extend.
- **Systems programming focus:** emphasizes process handling, pipes, redirection, and shell control flow in C.

## Learning Outcomes

Through this project, I explored:

- Unix process creation and execution
- shell command parsing
- pipes and file descriptor redirection
- modular systems programming in C
- implementation of custom shell utilities
- integrating external libraries like `libcurl`

## Tech Stack

- **Language:** C
- **Build System:** Makefile
- **Concepts:** operating systems, processes, pipes, redirection, shell design, modular programming

## Future Improvements

Potential extensions include:

- command history navigation
- tab completion
- improved job control
- aliases and environment variable expansion
- more built-in shell utilities

