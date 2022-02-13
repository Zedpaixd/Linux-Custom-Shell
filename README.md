# Linux-Custom-Shell
Written in C, this project is a custom implementation of the linux terminal. Similarly, the project provides a Server / Client implementation where the "main" file (**fp.c** in our case) can be used as a way of giving a command and receiving the output by simply giving the command as an argument ( e.g. **./fp ls | rev** )

## Compiling the files
First and foremost, you must have the readline library installed. To do so, open your linux terminal and run the following command
" **sudo apt-get install libreadline-dev** "

From that point onwards, you are ready to start compiling. You will want to do as following:
" **gcc filename.c -o outputname -lreadline** " 

You can compile **server.c** , **client.c** and **fp.c** with any names you want however **cp2.c** and **mv.c** must be given the names of "cp", respectively "mv" and to be put in the "bin" folder. You are able to direct the output location by simply doing " **gcc cp2.c -o bin/cp -lreadline** " 

### Functionalities
- The user is free to use any sort of command they would want, provided their functionality in the normal Linux terminal.
- File management (the "**cp**" and "**mv**" commands, together with "**dirname**" have been customly implemented
- Infinite piping is customly implemented
- Redirection support is provided for **STDIN** **STDOUT** and **STDERR**
- Mandatorily, all the previously stated points can be used together.

### Extra features
- Simple command history management
- Autocompletion supported


Shall you encounter any bugs, feel free to reach out to me on discord ( *Planta#9305* ) or via email ( *planta636@gmail.com* )
