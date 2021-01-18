# Documentation

## Arhitecture
System is comprised of three separate entities. First is the service component - PubSubEngine. Service component is built for servicing two types of clients: Publisher and Subscriber. General arhitecture of the system is client - server arhitecture with N number of clients (both publishers and subscribers) and one service. Protocol used for communication is TCP protocol. Separate static library is introduced - TCPLib and its functionalities are used for TCP communication between components. Data model that is used for storing data is Measurment structure and a generic list. Generic list is used for storing Measurment packets and client connection sockets in separate lists.

### PubSubEngine

### Publisher

### Subscriber

### TCPLib

### Common

## Functionality

## Usage
