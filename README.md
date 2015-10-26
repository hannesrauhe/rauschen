Rauschen is a peer-to-peer communication tool to send encrypted and signed messages directly from one machine to another (or multiple) without a server in the middle.

Rauschen consists of the daemon rauschend, the library librauschen to use the daemon for your application's purposes, and a few command line tools for simple purposes:

* Rauschend discovers other peers on the network and takes care of encryption and signatures
* librauschen provides a few simple functions to relay messages from your application to the daemon and vice versa
* command line tools are there to send/broadcast a message and execute programs when messages arrive (because messages can have a custom type different actions can executed)