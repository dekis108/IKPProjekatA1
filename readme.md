# Documentation

## Arhitecture
System is comprised of three separate entities. First is the service component - PubSubEngine. Service component is built for servicing two types of clients: Publisher and Subscriber. General arhitecture of the system is client - server arhitecture with N number of clients (both publishers and subscribers) and one service. Protocol used for communication is TCP protocol. Separate static library is introduced - TCPLib and its functionalities are used for TCP communication between components. Data model that is used for storing data is Measurment structure and a generic list. Generic list is used for storing Measurment packets and client connection sockets in separate lists.

### PubSubEngine
Service component intended for servicing publisher/subscriber clients. It accepts connections in thread made for listening. Connections that are of publishers are stored in a generic list initialised for publishers and those that are of subscriber are stored in subscriber generic list. After clients introducment, depending on which type he is, publisher/subscriber logic begins. For publishers there is a thread pool that accepts Measurment packets sent from various Publishers. The packet is then stored in generic lists depending on the topic it has - Analog or Status. For subscribers there is a thread pool that is used for sending Measurment packets to clients depending on what kind of topic they are subscribed to. 

### Publisher
Client component that is intended for publishing Measurment packets to the PubSubEngine.Before publishing it introduces itself to the PubSubEngine as a Publisher client. It then randomly generates packet values. Some of those values are delibretly left unvalid so that the subscriber can use validation on them. It publishes on a interval of 1001ms. Everytime it has a packet to publish it uses TCPSend function from TCPLib.

### Subscriber
Client component that is intended for recieving Measurment packets which correspont to clients subscription topic. Client upon connection with PubSubEngine introduces itself as a Subscriber client. After introduction, it picks a topic or topics it will subscribe to. After subscription, it creates a thread for packet recieving. This thread listenes on the client socket for packets that are being sent. For every packet, Subscriber has to do a validation on it and write on the console its validation status.

### TCPLib
A static library that has separate functions for sending and recieving on the TCP protocol. TCPSend function has two versions, one is intended for sending Measurment packets and other one is intended for introducing clients type to the service. TCPRecieve is a function that uses standard recv function to recieve anytype of packet as TCPRecieve will write in the recieved bytes into the recvbuf - recieve buffer and it is callers job to cast it to the expected type.

### Common
Common library contains Measurment.h that contains Measurment structure definition and its helper functions and GenericList.h. Measurment structure is comprised of enum MeasurmentTopic that has values of Analog and Status, then MeasurmentType enum that has values of SWG,CRB,MER and finally an integer value. GenericList contains definition and helper functions for a Generic List that is comprised of nodes that can take any type of data to be stored in them as it has a void pointer in the node structure.

## Functionality and usage
First the PubSubEngine must be started. After that, clients can be started. It does not matter in which order the clients are started. After starting Publisher, the publishing data is sent to the PubSubEngine and via the engine it is being routed to the Subscribers that are subscribed to the packets topic.

