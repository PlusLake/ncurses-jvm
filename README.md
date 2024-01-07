# ncurses-jvm

A sample program using ncurses from JVM using IPC (unix socket).  

## Motivation

One day I want to make some text-based applications, but I am not much familiar with C.  
I have tried JNI to call ncurses functions directly, however there is some issue around the signals.  
(for instance, resizing terminals will produce SIGWINCH but JVM kind of ignore/consumes them)

Therefore I gave up the JNI approach and doing this IPC approach.

## Requirements

- gcc / libncurses / kotlinc
- native-image from GraalVM (optional)
