1. **How does the remote client determine when a command's output is fully received from the server, and what techniques can be used to handle partial reads or ensure complete message transmission?**

   >The client knows that a command’s output is complete when it encounters a special marker—often the EOF character (`0x04` in many networked shells) or some defined terminator. Since TCP is a stream protocol, you can’t rely on a single `recv()` call to read an entire message. Instead, the client typically loops, reading data in chunks, until the terminating character is found. Partial reads are handled by continuing to read from the socket and appending the data to a buffer. The protocol itself must define:
   >- **A fixed terminator** (like EOF) that the server sends after each command’s output.
   >- **Or a length prefix** that specifies how many bytes to expect.
   >
   >This ensures the client knows when to stop reading and that the entire message has been received.

2. **This week's lecture on TCP explains that it is a reliable stream protocol rather than a message-oriented one. Since TCP does not preserve message boundaries, how should a networked shell protocol define and detect the beginning and end of a command sent over a TCP connection? What challenges arise if this is not handled correctly?**

   >Because TCP sends data as a continuous stream of bytes, a networked shell protocol must define clear boundaries. Common techniques include:
   >- **Including a length field** in each message, so the receiver knows how many bytes to read.
   >- **Using a special delimiter** (like `\0` or `\n`) that signifies the end of one command or output block.
   >
   >If you fail to handle boundaries correctly, partial commands may be read together, or one command’s data may inadvertently merge with another’s. This can lead to garbled data, misinterpreted commands, or synchronization issues between client and server.

3. **Describe the general differences between stateful and stateless protocols.**

   >- **Stateful Protocols**: The server and/or client keeps track of the session’s current status or context. Each request or message may depend on prior interactions, and both sides preserve that state to handle subsequent communications correctly. Examples include FTP or certain TCP sessions where the connection is maintained.
   >- **Stateless Protocols**: The server (and sometimes the client) treats each request independently. It does not store prior context, so each message must contain all necessary info to process it. Examples include HTTP/1.0 (in its simplest form) or certain RESTful APIs.

4. **Our lecture this week stated that UDP is "unreliable". If that is the case, why would we ever use it?**

   >Even though UDP doesn’t guarantee delivery or order, it is often chosen because:
   >- **Low overhead**: There’s no connection setup or retransmission mechanism by default, which lowers latency.
   >- **Speed**: It can be faster for real-time applications like streaming, VoIP, or gaming, where occasional dropped packets are less critical than delay.
   >- **Custom Reliability**: Applications can implement their own logic for handling lost or out-of-order packets if needed, providing more flexibility than TCP in certain scenarios.

5. **What interface/abstraction is provided by the operating system to enable applications to use network communications?**

   >The OS provides **sockets** as the primary abstraction. Through socket APIs (e.g., `socket()`, `connect()`, `bind()`, `listen()`, `accept()`), applications can create endpoints for data exchange over various protocols (TCP, UDP, etc.). This socket interface hides much of the lower-level details (e.g., Ethernet framing, routing) and presents a unified mechanism for sending and receiving data.
