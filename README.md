## Directions:
You need to write an Internet Time and Weather Service (ITWS) software
package, which consists of a server program and a client program.  The
client program allows a user to make time and weather queries in an
interactive way.  The server program, as expected, duly answers such
queries.  You don't need to be a meteorologist to carry out this
project: the weather provided to the client can be generated at
random.  The time the server provides to the client can be the clock
reading at the server machine.  The server also needs to keep track of
"who has asked for what?", i.e., to have a log recording all
client/server interactions.  You are required to use either 
multi-processing (covered in class) or multi-threading in designing
the server code.  You are also required to program it using either C
or C++.

### To run, run the following commands:
* gcc -o client client.c
* gcc -pthread -o server server.c

### Then:
* ./server to run the server
* ./client to run the client
