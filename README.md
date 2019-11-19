# File sharing system

This application shares the contents of a folder from multiples computers connected to the same local network. 

To make it work on the same computer, open two Linux terminals, the first one inside PeerToPeer folder, the other one inside PeerToPeerC. In both terminals run the comand *make*. 

To start the service, run the next commands:

From PeerToPeer: (First user)

`./peerPrototipo <PATH TO SHARED FOLDER 1> <PATH TO TRASH FOLDER 1>`

From PeerToPeerC: (Second user)

`./peerPrototipo \<PATH TO SHARED FOLDER 2\> \<PATH TO TRASH FOLDER 2>`

The first argument is the absolute path to the folder to be shared. The second argument contains the path to a folder in which, if there is a file that also exists in the SHARED FOLDER, it is deleted from all the users.

As shown in the previous example, the all the paths presented are different between them, this is done in order to avoid read and write errors.
