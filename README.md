## This is a demo of solving a hard problem with multithreading 

The function we are trying to solve is ![image](https://upload.wikimedia.org/math/0/b/c/0bc08045195dc823c22d1fa283cb0759.png).

The implementation present 4 different multithread collaboration scheme: 
* Sequential
* Select
* Poll
* Epoll 

## To run the example, make sure you're working under a Linux distro. (should 
also work in Unix machines if you take out the 'Epoll' part).

Run `make sequential` to monitor workers sequentially
Run `make select` to monitor workers using `select` mode
Run `make poll` to monitor workers using `poll` mode
Run `make epoll` to monitor workers using `epoll` mode

To test individual worker, use the syntax:
    $ ./worker -x 2 -n 3

If you want to manually initiate master, use the following syntax.
    $ ./master --worker_path ./worker --wait_mechanism MECHANISM -x 2 -n 12
