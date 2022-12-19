# Socket Programming
## Description
I this project we are going to implement an online job market system using socket programming and system calls in C.

## Implementation:
There is a server that wants to hand over a number of projects to expert users. Users participate in groups of 5 for each existing project. Then they offer a price for the project and the user with the lowest price offer wins that project. Finally, the server removes that project from the list of projects.

## Timer:
To measure each user's turn time, we use unix signals, and more precisely, the SIGALRM signal.

## System Synchronizition:
Throughout the program (in client and server code), the entire system must be running concurrently, so that the server can handle multiple clients at the same time. Considering that, some of the system calls are blocking, we use the SELECT system call to solve this problem. This system call is responsible for monitoring concurrent communications and makes all I/Os to be done asynchronously and no part of code is blocked.
