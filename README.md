# Basic recursive "ls" with flag support
Implementation of the Linux "ls" command with "-fdalh" flag support

## Description
The dirls program will print all appropriate files and directories in the provided directory or the cwd if no directory is provided. It will also traverse down all sub-directories and print their contents. The program attempts to format the output intuitively on stdout like a tree.

## Getting Started

### Dependencies

* To be run on a x86-64 Linux machine

### Executing program
./dirls can be run as "dirls" where the users cwd will be used as the default. It can also be run with any combination of the following flags "-fdalh" and one or more directories. Ex: "./dirls -fa example_directory"

## Authors

* Stefan Silverio
* stefansilverio@gmail.com
* https://twitter.com/stefansilverio