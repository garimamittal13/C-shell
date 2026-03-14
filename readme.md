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

## How to Run the Shell

After building the project, start the shell by running:

```bash
./my_shell
