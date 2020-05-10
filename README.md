# Hot-Potato
TCP Socket Programming- Hot Potato

Developed a pair of programs that will interact to model a game, which is described below. 

The game is called hot potato, in which there are some number of players who quickly toss 
a potato from one player to another until, at a random point, the game ends and the player 
holding the potato is “it”. The object is to not be the one holding the potato at the end 
of the game. In this assignment, you will create a ring of “player” processes that will pass
the potato around. Thus, each player process has a left neighbor and a right neighbor. 
Also, there will be a “ringmaster” process that will start each game, report the results, 
and shut down the game.

To begin, the ringmaster creates a “potato” object, initialized with some number of hops 
and sends the potato to a randomly selected player. Each time a player receives the potato,
it will decrement the number of hops and append the player’s ID to the potato. If the remaining 
number of hops is greater than zero, the player will randomly select a neighbor and send the 
potato to that neighbor. The game ends when the hop counter reaches zero. The player holding 
the potato sends it to the ringmaster, indicating the end of the game. The ringmaster prints 
a trace of the game to the screen (using the player identities that are appended to the potato), 
and shuts the game down (by sending a message to each player to indicate they may shut down as the game is over).
Each player will establish three network socket connections for communication with the player 
to the left, the player to the right, the ringmaster. The potato can arrive on any of these three 
channels. Commands and important information may also be received from the ringmaster. 
The ringmaster will have N network socket connections. At the end of the game, the ringmaster will
receive the potato from the player who is “it”.
The programs will create one ringmaster process and some number of player processes, then play a game 
and terminate all the processes gracefully. You may explicitly create each process from an interactive shell; however, 
the player processes must exit cleanly at the end of the game in response to commands from the ringmaster.
