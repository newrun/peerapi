# PeerConnect

### Table of Contents
* Constructor
 * [PeerConnect()](#classpeerconnect)
* Methods
 * [Open()](#open)
 * [Close()](#close)
 * [Connect()](#connect)
 * [Send()](#send)
* Events
 * [On("open")](#onopen)
 * [On("close")](#onclose)
 * [On("connect")](#onconnect)
 * [On("message")](#onmessage)
 * [On("writable")](#onwritable)
* Static Methods
 * [PeerConnect::Run()](#run)
 * [PeerConnect::Stop()](#stop)
* Example
 * [echo_server](#echoserver)
 * [echo_client](#echoclient)


## Include

Include following at the top of your code

```c++
#include "peerconnect.h"
```

## Class: PeerConnect

This class is a PeerConnect server and client.

## Constructor

<a name="classpeerconnect"/>
### PeerConnect()

Construct a new PeerConnect object. A `peer` is local name of peer and remote peer may connect to local peer with `peer` name.

```c++
PeerConnect( 
  const std::string peer = ""
)
```

Parameters

> **peer** [optional]
> * `peer` is a local name of peer. Remote peers may connect to local peer by `peer`.

Examples

```c++
PeerConnect pc1;
PeerConnect pc2("PEER_NAME");
PeerConnect pc3("Your@email.com");
```

## Methods

<a name="open"/>
### Open()

Initialize a peer. It is required to call `Open()` before calling other methods like Send(), Connect() and On().

```c++
void Open()
```

<a name="close"/>
### Close()

Close the remote peer connection or connection attempt, if any. If the connection is already closed, this method does nothing.

```c++
void Close(
  const std::string peer = ""
)
```

Parameters

> **peer** [optional]
> * Close a remote `peer` if peer is provide. Otherwise close all local and remote peers.

<a name="connect"/>
### Connect()

Connect to remote peer.

```c++
void Connect(
  const std::string peer
)
```

Parameters

> **peer**
> * A name of peer connect to.

<a name="send"/>
### Send()

Transmits data to the peer over p2p connection.

```c++
bool Send(
  const std::string& peer,
  const char* data,
  const size_t size,
  const bool wait = SYNC_OFF
)

bool Send(
  const string& peer,
  const string& data,
  const bool wait = SYNC_OFF
)
```

Parameters

> * peer : A name of peer receiving data
> * data : A data to send
> * size : A size of data
> * wait : SYNC_ON if synchronously send a data and SYNC_OFF if asynchronously send a data.

Constants
> * SYNC_ON : bool `true`
> * SYNC_OFF : bool `false`

## Events

<a name="onopen"/>
### On("open")

Attaches "open" event handler. A "open" event is emitted when PeerConnect is ready by `Open()` method.

```c++
pc.On("open", function_pc( std::string peer ) {
  // ...
})
```

Parameters

> peer : A name of initialized peer

<a name="onclose"/>
### On("close")

Attaches "close" event handler. A "close" event is emitted when connection is closed.

```c++
pc.On("close", function_pc( std::string peer, CloseCode code, std::string desc ) {
  // ...
})
```

Parameters

> * peer : A name of closed peer. Note that peer is one of local peer or remote peer.
> * code : A close code defined in the PeerConnect.
> * desc : A description of close reason.

Constants

```c++
enum CloseCode {
  // Success
  CLOSE_NORMAL        = 0,

  // Failure
  CLOSE_GOING_AWAY,
  CLOSE_ABNORMAL,
  CLOSE_PROTOCOL_ERROR,
  CLOSE_SIGNAL_ERROR
};
```

<a name="onconnect"/>
### On("connect")

Attaches "connect" event handler. A "connect" event is emitted when connection is established.

```c++
pc.On("connect", function_pc( std::string peer ) {
  // ...
})
```

Parameters

> * peer : A name of remote peer

<a name="onmessage"/>
### On("message")

Attaches "message" event handler. A "message" event is emitted when data is received.

```c++
pc.On("message", function_pc( std::string peer, char* data, std::size_t size ) {
  // ...
})
```

Parameters

> * peer : A name of remote peer that sent a message.
> * data : A pointer of data.
> * size : A size of data.


<a name="onwritable"/>
### On("writable")

Attaches "writable" event handler. A "writable" event is emitted when read to send data. It is useful when asynchronously (SYNC_OFF) sending a data.

```c++
pc.On("writable", function_pc( std::string peer ){
  // ...
});
```

Parameter
> * peer : A name of peer that is ready to send a data.

## Static methods

<a name="run"/>
### PeerConnect::Run()

Run PeerConnect object's event processing loop. Note that the thread quit a loop if other thread calls PeerConnect::Stop() method. 

```c++
void PeerConnect::Run()
```

<a name="stop"/>
### PeerConnect::Stop()

Stop PeerConnect object's event processing loop.

```c++
void PeerConnect::Stop()
```

## Example

<a name="echoserver"/>
* echo server

```c++
PeerConnect pc("SERVER_PEER");

pc.On("message", function_pc(std::string peer, char* data, std::size_t size) {
  // Echo message
  pc.Send(peer, data, size);
});

pc.Open();
PeerConnect::Run();
```

<a name="echoclient"/>
* echo client
```c++

PeerConnect pc;

pc.On("open", function_pc(std::string peer) {
  pc.Connect("SERVER_PEER");
});

pc.On("connect", function_pc(std::string peer) {
  pc.Send(peer, "Hello world");
});

pc.On("close", function_pc(std::string peer, CloseCode code, std::string desc) {
  PeerConnect::Stop();
});

pc.On("message", function_pc(std::string peer, char* data, std::size_t size) {
  pc.Close();
});

pc.Open();
PeerConnect::Run();
```
