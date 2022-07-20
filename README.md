# PolymorphicEngine

A simple, polymorphic code engine written in C++. Compatible with programs written in C/C++ (possibly more languages, not tested). Combats hash based scans and 
signature based scans (to a certain extent). 

## What is a polymorphic engine and how does it work.

A polymorphic engine (a.k.a mutation engine) is a program that is able to change the code of a program whilst preserving its functionality. It is primarily used to combat
**signature based scans**, which is a popular method utilized by AntiVirus or AntiCheat software to detect malware, although it is also used to prevent reverse engineering.
**signature based scans** works by looking at a particular section of a program's code, memorizing its characteristics and saving it for future references. A malicious
function could be logged in a signature based scan, have its "signature" saved, and any future program with the same malicious function will be able to be detected. 

A polymorphic engine circumvent this by mutating or changing the code, but without changing the overall functionality. If done properly, then previous signatures of
a given function will no longer be valid since the function itself (the source code, so to speak) is changed. It also prevents decompilers (programs that turn compiled 
byte code into human readable C code) from functioning properly, increasing the difficulty to reverse engineer it.

## How does this polymorphic engine work

An ideal polymorphic engine should be able to mutate the entire code section of a program. However, this is not an easy task as changing the source code will often lead 
to changes in its behaviour (extra difficult if the mutation is unpredictable). 

This polymorphic engine simplies things by only mutating the functions that aren't being actively used, whilst keeping the ones that are intact. The engine will decompile
a program, identify a list of functions/subroutines, remove those that are being actively used and mutate the rest. Whilst this is not ideal, as signatures of certain
functions will still remain the same, statistically speaking certain signatures built by 3rd party programs are bound to be changed.

## Demo

Consider the following program, written in C++

```
#include <iostream>

int function1() {
    return 0;
}

int function2() {
    int i = 0;
    i += 1;
    return i;
}

int main()
{
    while (true) {
        char key;
        scanf("%c", &key);
        printf("%d", function1());
    }
}
```

Although it contains 2 separate functions, `function2()` is not being utilized, yet will still be compiled into the final program by most standard compilers.
Here is what function 2 looks like in memory:

![asdf](https://i.imgur.com/rDTKMiq.jpeg)

Here is what `function2()` looks like after running the polymorphic engine.

![asdf](https://i.imgur.com/McmbfTz.jpeg)

Any program that have built a signature based on `function2()` will not be able to identify it after the mutation.

## Known bugs

C++'s `std::cin` will cause the program to crash after running the polymorphic engine. Use scanf (or one of its variants) instead.
