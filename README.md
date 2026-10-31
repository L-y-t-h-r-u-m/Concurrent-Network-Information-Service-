# Concurrent Network Information Service

A client-server networking application implemented in **C** using **TCP/UDP sockets** and **POSIX threads**. The project provides a DNS-like lookup service over TCP, supports multiple concurrent clients, and includes a UDP-based server status monitoring service.

## Features

- **TCP-based Lookup Service:** Accepts domain name and record type queries from clients.
- **Concurrent Client Handling:** Uses POSIX threads to handle multiple TCP clients simultaneously.
- **DNS-like Record Support:** Supports predefined `A`, `CNAME`, `MX`, and `NS` records.
- **UDP Status Monitoring:** Provides a separate UDP service to check server availability and active TCP client count.
- **Thread Synchronization:** Uses mutex locks to protect shared client-count data.
- **Client Identification:** Tracks and displays connected clients' IP addresses and port numbers.
- **Persistent Client Sessions:** Allows clients to send multiple queries over a single TCP connection.

## Project Architecture

The application consists of three main components:

| File | Description |
|---|---|
| `server.c` | Runs the TCP lookup server and UDP status service. |
| `tcp_client.c` | Connects to the TCP server and sends lookup queries. |
| `udp_client.c` | Sends UDP status requests and displays server information. |

### Communication Overview

**TCP Lookup Service**
1. The TCP server listens for incoming client connections on port `8080`.
2. Each accepted client is handled by a separate thread.
3. Clients send a domain name and record type.
4. The server searches its predefined record database and returns the result.
5. Clients can send multiple queries or terminate the session using `EXIT`.

**UDP Status Service**
1. The UDP server listens on port `8081`.
2. The UDP client sends a `STATUS` request.
3. The server responds with its availability and the current number of connected TCP clients.

## Supported Record Types

The lookup service supports the following predefined record types:

| Record Type | Description |
|---|---|
| `A` | Maps a domain name to an IPv4 address. |
| `CNAME` | Represents a canonical name alias. |
| `MX` | Represents a mail exchange server. |
| `NS` | Represents a name server. |

Example queries:

```text
www.example.com A
web.example.com CNAME
example.com MX
example.com NS
```

## Technologies Used

- **Language:** C
- **Networking:** TCP and UDP socket programming
- **Concurrency:** POSIX threads (`pthreads`)
- **Synchronization:** POSIX mutexes
- **Networking APIs:** BSD sockets

## Requirements

- GCC compiler
- Linux or another POSIX-compatible environment
- POSIX threads library

## Compilation

Open a terminal in the project directory.

Compile the server:

```bash
gcc server.c -o server -pthread
```

Compile the TCP client:

```bash
gcc tcp_client.c -o tcp_client
```

Compile the UDP client:

```bash
gcc udp_client.c -o udp_client
```

## Execution

### 1. Start the Server

```bash
./server
```

The server starts:
- TCP lookup service on port `8080`
- UDP status service on port `8081`

### 2. Run the TCP Client

Open another terminal:

```bash
./tcp_client
```

Enter a domain name and record type when prompted.

Example:

```text
Enter domain name and record type: www.example.com A
```

To close the TCP client session:

```text
EXIT
```

### 3. Run the UDP Client

Open another terminal:

```bash
./udp_client
```

The client sends a status request and displays the server response, including the active TCP client count.

## Concepts Demonstrated

- TCP connection-oriented communication
- UDP connectionless communication
- Client-server architecture
- Multithreaded server design
- Concurrent client management
- Mutex-based synchronization
- IPv4 socket addressing
- Request-response communication

## Limitations

- The lookup service uses a predefined set of records and is **not a full implementation of the DNS protocol**.
- The server and clients currently use localhost (`127.0.0.1`) and fixed port numbers.
- Error handling and TCP message framing can be improved for more robust communication.
- The UDP status service is designed for basic server monitoring.


This project was developed as part of academic networking coursework.