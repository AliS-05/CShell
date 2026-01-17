# CShell - Custom Unix Shell

A Unix shell implementation in C supporting command piping, I/O redirection, and standard system binaries.

## Features

- **Dynamic Pipe Support**: Handles arbitrary pipeline depth
- **I/O Redirection**: Supports output redirection with `>` (overwrite) and `>>` (append)
- **Built-in Commands**: Uniquely `cd` and `exit` 
- **Standard Binaries**: Executes all system commands (ls, cat, grep, echo, etc.)

## Build & Run
```bash
git clone
gcc main.c -o main
./main
```

## Usage Examples
```bash
# Simple command
Enter Command -> ls -la

# Piping
Enter Command -> ls -l | grep .c | wc -l

# Output redirection (overwrite)
Enter Command -> cat main.c > output.txt

# Output redirection (append)
Enter Command -> echo "text" >> output.txt

# Complex pipeline
Enter Command -> cat output.txt | grep token | sort | uniq > results.txt

# Change directory
Enter Command -> cd ..

# Exit shell
Enter Command -> exit
```

## Implementation Details

### Architecture
- **Tokenization**: Parses input into pipeline of commands using `strtok_r()`
- **Process Management**: Uses `fork()/exec()` for command execution
- **Pipe Management**: Creates pipes dynamically based on command chain length
- **File Descriptor Handling**: `pipe()` and `dup2()` for file descriptor handling

### Key Components
- `tokenizeInput()`: Splits user input by `|` and tokenizes each command
- `pipelineProcess()`: Manages fork/exec for each command in pipeline
- `checkRedirection()`: Detects and handles `>` and `>>` operators
- `execute()`: Wrapper around `execvp()` with error handling

### Technical Highlights
- Dynamic memory allocation based on pipeline depth
- Proper file descriptor cleanup to prevent leaks
- Signal-aware process management with `waitpid()`
- Support for unlimited arguments per command

## Known Limitations

- **Quote Parsing**: Tokenization doesn't handle quoted strings correctly
  - Example: `grep ".c"` won't work, but `grep .c` will
- **Input Redirection**: `<` and `<<` not yet implemented
- **Signal Handling**: Ctrl-C terminates the shell (no job control)
- **Memory Management**: Minor memory leaks from `strdup()` (acceptable for shell lifecycle)
- **No Tab Completion**: Command/path autocomplete not implemented
- **No Configuration**: No `.bashrc` equivalent for aliases or customization

## Future Improvements

- Input redirection support (`<`, `<<`)
- Improved tokenizer for quoted strings
- Signal handling (Ctrl-C, Ctrl-Z for job control)
- Tab autocomplete for commands and paths
- Configuration file support (aliases, environment)
- Command history (up/down arrows)
- Background job support (`&`)
