
# distributed-computing-C

## Introduction

This section contains different C implementations of algorithms done in [distributed manner](https://en.wikipedia.org/wiki/Distributed_computing). 

## Requirements and Library

GCC 4.2 or **higher** is required to compile.

Library used for parallel computing is [Open MPI](https://www.open-mpi.org/)

## Hostfile

Hostfile contains the list of machine to distribute the execution between. The connection is done through **ssh**.

An example of Hostfile in my school lab (lab_room_number-machine_number) comes as follow: 

`
f205-14

f205-15

f205-16

localhost
`
