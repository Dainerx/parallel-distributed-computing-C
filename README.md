
# parallel-computing-C

## Introduction

This repoisotry contains different C implementations of algorithms done sequentially and in parallel to show the speed up, efficiency and cost for every scenario.

Most of the programs are launched with n and p as input.
**Where: n is the size of the input and p the number of threads to use.**

Example, launch a program with n=10 and p=3:

`./program 10 3`

**Note: Some files varies the number of threads used within the code itself.**

## Requirements and Library

GCC 4.2 or **higher** is required to compile.

Library used for parallel computing is [Openmp](https://www.openmp.org/).

## Notions

**People familiar with parallel computing can skip this section**

You'll run by some concepts I'm using such as speedup, efficiency and cost. The understanding of these concepts is necessary before working on any parallel computing problem.

- Big O notation. O(n) is used classify algorithms according to how their running time or space requirements grow as the input size grows (in this case n).
- Speed up is denoted as S(p). S(p) = T(1) / T(p).
- Efficiency is denoted as E(p). E(p) = T(1) / p*(Tp)
- Cost is denoted as W(p) or C(p). W(p) = p*T(p)

![forumals](https://images.slideplayer.com/33/8243710/slides/slide_12.jpg)

## Benchmarking

It is important to note that benchmarking varies from one exectuion to another. To benchmark in a more accurate way I highly recommend surrounding the computing section by a for loop in order to get the average computing time for each computing section.

## Contributing

Contributing is welcomed by everyone. Please feel free to open any issue or pull request.
