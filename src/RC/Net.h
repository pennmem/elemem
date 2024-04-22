/////////////////////////////////////////////////////////////////////
//
// RC Library, (c) 2011-2015, Ryan A. Colyer
// Distributed under the Boost Software License, v1.0. (LICENSE.txt)
//
/// \file Net.h
/// Provides basic cross-platform socket networking, with support for
/// both blocking and non-blocking sending and receiving.
/////////////////////////////////////////////////////////////////////

#ifndef RC_NET_H
#define RC_NET_H

#include "Types.h"
#include "Errors.h"
#include "File.h"
#include "RStr.h"
#include "Data1D.h"
#include "APtr.h"
#include <string.h>
#ifdef WIN32
#ifndef _WIN32_WINNT
/// @cond UNDOC
#define _WIN32_WINNT 0x0501  // XP and later only
/// @endcond
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <sys/socket.h>
#include <netdb.h>
#endif
#include <sys/types.h>

/// \def RC_SockType
/// The standard socket type used in socket calls for a given system.
/// \def RC_InvSock
/// The value returned for an invalid socket.
/// \def RC_SockErr
/// The value returned for a socket error.
/// \def RC_CloseSock
/// The function for manually closing sockets.
/// \def RC_WouldBlock
/// The return value for a socket call which would block.
/// \def RC_SockErrorAt
/// The variable or function call for obtaining the last error.
/// \def RC_SockReturn
/// The return type for send and recv calls.
#ifdef WIN32
#define RC_SockType SOCKET
#define RC_InvSock INVALID_SOCKET
#define RC_SockErr SOCKET_ERROR
#define RC_CloseSock closesocket
#define RC_WouldBlock WSAEWOULDBLOCK
#define RC_SockErrorAt WSAGetLastError()
#define RC_SockReturn int
#else
#define RC_SockType int
#define RC_InvSock -1
#define RC_SockErr -1
#define RC_CloseSock close
#define RC_WouldBlock EWOULDBLOCK
#define RC_SockErrorAt errno
#define RC_SockReturn ssize_t
#endif


namespace RC {
  /// A portable socket interface for reading and writing to an open socket.
  /** This class shares its data in a reference-counted manner upon
   *  assignment or copy construction, and automatically closes the socket
   *  as the last instance goes out of scope.
   */
  class Sock {
    protected:
    /// @cond PROTECTED

    class SockHelper {
      public:

      inline SockHelper(RC_SockType new_sock=RC_InvSock)
        : socket(new_sock),
          buf (4096),
          index(0),
          full_to(0) {
      }

      inline ~SockHelper() {
        Close();
      }

      inline void Close() {
        RC_SockType loc_soc;
#ifdef CPP11
        loc_soc = socket.exchange(RC_InvSock);
#else
        loc_soc = socket;
        socket = RC_InvSock;
#endif
        if (loc_soc != RC_InvSock) {
          RC_CloseSock(loc_soc);
        }
      }


      inline bool DataReady(bool do_block = true) const {
        fd_set readset;
        FD_ZERO(&readset);
        FD_SET(socket, &readset);
        struct timeval *timeptr = NULL;
        struct timeval timeout;
        if (!do_block) {
          timeout.tv_sec = 0;
          timeout.tv_usec = 0;
          timeptr = &timeout;
        }
        int result = select(socket+1, &readset, NULL, NULL, timeptr);
        return (result > 0);
      }


      inline bool CanSend(bool do_block = true) const {
        fd_set writeset;
        FD_ZERO(&writeset);
        FD_SET(socket, &writeset);
        struct timeval *timeptr = NULL;
        struct timeval timeout;
        if (!do_block) {
          timeout.tv_sec = 0;
          timeout.tv_usec = 0;
          timeptr = &timeout;
        }
        int result = select(socket+1, NULL, &writeset, NULL, timeptr);
        return (result > 0);
      }


      inline void ReadMore(size_t amnt_wanted = -1, bool do_block = true) {
        size_t amnt_left = full_to - index;
        if (amnt_wanted < amnt_left) {
          return;
        }

        bool got_data = false;
        do {
          if (do_block) {
            DataReady();
          }

          RC_SockReturn bytes_recv;

          if (amnt_wanted == size_t(-1)) {
            amnt_wanted = buf.size();
          }

          if (amnt_wanted > buf.size()) {
            buf.Resize(amnt_wanted);
          }

          if (index + amnt_wanted > buf.size()) {
            buf.CopyData(0, index, amnt_left);
            index = 0;
            full_to = amnt_left;
            amnt_wanted = amnt_wanted - amnt_left;
          }

          bytes_recv = recv(socket, reinterpret_cast<char*>(buf.Raw()) + 
            full_to, amnt_wanted, 0);

          if (bytes_recv == RC_SockErr) {
            if (RC_SockErrorAt == RC_WouldBlock) {
              continue;
            }
            Throw_RC_Type(Net, "Socket receive error");
          }

          got_data = true;
          full_to += bytes_recv;

        } while (do_block && !got_data);
      }


      size_t SendData(const char* arr, size_t len, bool do_block = true) {
        const char *arr_current = arr;
        size_t len_left = len;

        do {
          if (do_block) {
            CanSend();
          }

          RC_SockReturn bytes_sent;
          bytes_sent = send(socket, arr, len, 0);

          if (bytes_sent == RC_SockErr) {
            if (RC_SockErrorAt == RC_WouldBlock) {
              continue;
            }
            Throw_RC_Type(Net, "Socket send error");
          }

          arr_current += bytes_sent;
          len_left -= bytes_sent;
        } while (do_block && len_left > 0);

        return len;
      }

#ifdef CPP11
      std::atomic<RC_SockType> socket;
#else
      RC_SockType socket;
#endif

      Data1D<u8> buf;
      size_t index;
      size_t full_to;
      RStr remote_addr;
      RStr remote_port;
    };

    APtr<SockHelper> helper;

    /// @endcond

    public:

    /// Encapsulate a new socket, receiving the base socket type of the
    /// system (int or SOCKET).
    /** Use Connect or Listener::Accept to initialize this.
     */
    inline Sock(RC_SockType new_sock=RC_InvSock) {
      helper = new SockHelper(new_sock);

      if (new_sock != RC_InvSock) {
        bool is_non_blocking = false;
        // Set non-blocking.
#ifdef WIN32
        unsigned long mode = 1;
        if (ioctlsocket(new_sock, FIONBIO, &mode) == NO_ERROR) {
          is_non_blocking = true;
        }
#else
        int flags = fcntl(new_sock, F_GETFL, 0);
        if (flags != -1) {
          if (fcntl(new_sock, F_SETFL, flags | O_NONBLOCK) != -1) {
            is_non_blocking = true;
          }
        }
#endif
        if (!is_non_blocking) {
          Throw_RC_Type(Net, "Could not set socket non-blocking!");
        }
      }

    }

    /// Manually close the socket.
    /** Note this is handled automatically when all copies of the Sock go
     *  out of scope.)
     */
    inline void Close() {
      helper->Close();
    }

    /// For record-keeping, set the remote address and port.
    inline void SetRemote(RStr remote_addr, RStr remote_port) {
      helper->remote_addr = remote_addr;
      helper->remote_port = remote_port;
    }

    /// Get the remote address this socket is connected to.
    inline RStr GetRemoteAddr() const {
      return helper->remote_addr;
    }

    /// Get the remote port this socket is connected to.
    inline RStr GetRemotePort() const {
      return helper->remote_port;
    }

    /// Returns true if data is ready for reading.
    /** If block_until_ready is true this will wait until data is available
     *  or an error occurs, such as the socket closing or a signal being
     *  received.  (Note that rare kernel level events can cause data to no
     *  longer be available after it was reported as available.)
     */
    inline bool DataReady(bool block_until_ready = false) const {
      return helper->DataReady(block_until_ready);
    }

    /// Returns true if the socket is ready to send data.
    /** If block_until_ready is true this will wait until data can be sent
     *  or an error occurs, such as the socket closing or a signal being
     *  received.
     */
    inline bool CanSend(bool block_until_ready = false) const {
      return helper->CanSend(block_until_ready);
    }

#ifdef unix
    /// On unix-based systems, return a FileRW coresponding to this socket.
    /** Throws ErrorMsgNet if it cannot convert the socket.
     */
    FileRW ToFileRW() {
      FileRW frw;

      FILE* sfp = fdopen(helper->socket, "a+");
      if (sfp == NULL) {
        Throw_RC_Type(Net, "fdopen failed");
      }

      frw = FileRW(sfp);
      frw.SetFilename(helper->remote_addr + " " + helper->remote_port);
      return frw;
    }
#endif

    /// Return the raw encapsulated socket.
    inline RC_SockType Raw() const {
      return helper->socket;
    }


    /// Receive data from the socket into buf, up to the size of buf.
    /** @param buf The buffer into which data is received.
     *  @param do_block If true, wait until some data is available.
     *  @return The number of elements of buf received.
     */
    template<class T>
    inline size_t Recv(Data1D<T> buf, bool do_block = true) {

      size_t amnt_to_read = buf.size()*buf.TypeSize();

      helper->ReadMore(amnt_to_read, do_block);

      size_t elem_in_helper = (helper->full_to - helper->index)
                              / buf.TypeSize();
      size_t elem_to_copy =
        buf.size() < elem_in_helper ? buf.size() : elem_in_helper;

      size_t bytes_to_copy = elem_to_copy * buf.TypeSize();

      buf.CopyFrom(helper->buf, helper->index, bytes_to_copy);
      helper->index += bytes_to_copy;

      return elem_to_copy;
    }

    /// Receive characters into str until a newline or the end of data.
    /** @param str The string into which a line is received.
     *  @param crop_newline If true, removes "\r\n" and "\n" from the end.
     *  @param do_block If true, keeps reading until newline or the socket
     *  closes.  If false, reads until newline or no data is available.
     *  @return The number of bytes received, including null characters and
     *  newlines not added to str.
     */
    inline size_t Recv(RStr& str, bool crop_newline = true,
                       bool do_block = true) {
      Data1D<char> arr;
      arr.Reserve(4096);

      size_t count = 0;

      while (1) {
        if (helper->full_to == helper->index) {
          helper->ReadMore(-1, do_block);
          if (helper->full_to == 0) {
            break;
          }
        }
        
        char c = helper->buf[helper->index];
        ++(helper->index);
        count++;

        if (c == '\0') {
          break;
        }
        if (c == '\n') {
          if (!crop_newline) {
            arr += c;
          }
          break;
        }

        arr += c;
      }

      if (crop_newline) {
        while (arr.size() > 0 && arr[arr.size()-1] == '\r') {
          arr.Resize(arr.size()-1);
        }
      }

      arr += '\0';
      str = arr.Raw();

      return count;
    }


    /// Sends the contents of buf through the socket.
    /** @param buf The buffer containing the data to send.
     *  @param do_block If true, sends all of buf.  If false, sends as much of
     *  buf as can be sent.
     *  @return The amount of data sent.
     */
    template<class T>
    inline size_t Send(Data1D<T> buf, bool do_block = true) {
      return helper->SendData(buf.Raw(), buf.Size()*buf.TypeSize(), do_block);
    }


    /// Sends the contents of str through the socket.
    /** @param str The string containing the data to send.
     *  @param do_block If true, sends all of str.  If false, sends as much of
     *  str as can be sent.
     *  @return The amount of data sent.
     */
    inline size_t Send(RStr str, bool do_block = true) {
      return helper->SendData(str.c_str(), str.size(), do_block);
    }
  };


  /// Provides both client and server sides of blocking TCP connections.
  class Net {
    public:

    /// Automatically called by Connect and Listener.
    /** The first time this is called, it sets SIGPIPE to ignore so the
     *  program doesn't terminate while writing to a closed socket.  Call
     *  this once manually beforehand if you want to set your own custom
     *  handler for SIGPIPE.
     */
    static inline void InitializeSockets() {
      static RC_THREAD_LOCAL bool already_run = false;
      if (! already_run) {
#ifdef WIN32
        WSADATA wsa_data;
        int err;
        err = WSAStartup(0x0202, &wsa_data);
        if (err != 0) {
          Throw_RC_Type(Net, "WSAStartup error");
        }
#else
        struct sigaction sa;
        memset (&sa, 0, sizeof(sa));
        sa.sa_handler = SIG_IGN;
        sigaction(SIGPIPE, &sa, NULL);
#endif

        already_run = true;
      }
    }


    /// Opens a TCP connection to host:port.
    /** Will throw ErrorMsgNet upon failure to connect if quiet_fail is
     *  false.
     *  @return True if the connect succeeded.
     */
    static inline bool Connect(Sock &connection, const RStr& host,
                               const RStr& port, bool quiet_fail=true) {
      FileRW retval;
      struct addrinfo hints;
      struct addrinfo *ai_list, *addr;
      RC_SockType sfd = RC_InvSock;

      InitializeSockets();
      connection.Close();

      memset(&hints, 0, sizeof(hints));
      hints.ai_family = AF_UNSPEC;  // Support IPv4 && IPv6
      hints.ai_socktype = SOCK_STREAM;  // For TCP

      int aierr = getaddrinfo(host.c_str(), port.c_str(), &hints, &ai_list);
      if (aierr != 0) {
        if (! quiet_fail) {
          Throw_RC_Type(Net, RStr(gai_strerror(aierr)).c_str());
        }
        return false;
      }
      
      for (addr = ai_list; addr != NULL; addr = addr->ai_next) {
        sfd = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);

        if (sfd == RC_InvSock) {
          continue;
        }

        if (connect(sfd, addr->ai_addr, addr->ai_addrlen) != RC_SockErr) {
          break;  // Connected.
        }

        RC_CloseSock(sfd);
      }

      freeaddrinfo(ai_list);

      // If could not find one
      if (addr == NULL) {
        if (! quiet_fail) {
          Throw_RC_Error("Connection failed");
        }
        return false;
      }

      connection = Sock(sfd);
      connection.SetRemote(host, port);

      return true;
    }


    /// Listens to the specified port for incoming TCP connections.
    class Listener {
      protected:
      /// @cond PROTECTED
      RC_SockType sockfd;

      /// @endcond
      public:

      /// Begin listening to the specified port for incoming TCP connections.
      /** Use Accept to accept a connection.
       */
      inline Listener(const RStr& port) {
        sockfd = RC_InvSock;

        struct addrinfo hints;
        struct addrinfo *ai_list, *addr;
        RC_SockType sfd;

        InitializeSockets();

        memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_UNSPEC;  // Support IPv4 && IPv6
        hints.ai_socktype = SOCK_STREAM;  // For TCP
        hints.ai_flags = AI_PASSIVE;  // Listen on all interfaces

        int aierr = getaddrinfo(NULL, port.c_str(), &hints, &ai_list);
        if (aierr != 0) {
          Throw_RC_Type(Net, RStr(gai_strerror(aierr)).c_str());
        }
        
        for (addr = ai_list; addr != NULL; addr = addr->ai_next) {
          sfd = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
          if (sfd == RC_InvSock) {
            continue;
          }

          if (::bind(sfd, addr->ai_addr, addr->ai_addrlen) == 0) {
            break;  // Bound.
          }

          close(sfd);
        }

        // If could not find one
        if (addr == NULL) {
          Throw_RC_Type(Net, "Could not bind");
        }

        freeaddrinfo(ai_list);

        if (listen(sfd, 128) == RC_SockErr) {
          Throw_RC_Type(Net, "Port in use");
        }

        sockfd = sfd;
      }


      /// If open, this closes the port being listened to.
      ~Listener() {
        if (sockfd != RC_InvSock) {
          close(sockfd);
        }
      }


      /// Provides the raw socket descriptor being listened to.
      RC_SockType Raw() const {
        return sockfd;
      }

      /// Accept an incoming connection, with read/write access through the
      /// FileRW connection.
      /** This throws an ErrorMsgNet on error if quiet_fail is false.
       *  @return False if an error occurred.
       */
      inline bool Accept(Sock &connection, bool quiet_fail=true) {
        RC_SockType acc_sock;
        struct sockaddr_in6 addr;
        socklen_t len = sizeof(addr);

        connection.Close();

        acc_sock = accept(sockfd, reinterpret_cast<sockaddr*>(&addr), &len);

        if (acc_sock == RC_InvSock) {
          if (! quiet_fail) {
            Throw_RC_Type(Net, "Could not accept connection");
          }
          return false;
        }

        connection = Sock(acc_sock);

        char host[2048];
        char port[2048];
        if (getnameinfo(reinterpret_cast<sockaddr*>(&addr), sizeof(addr), host,
              2048, port, 2048, NI_NUMERICHOST | NI_NUMERICSERV) == 0) {
          connection.SetRemote(host, port);
        }

        return true;
      }
    };

  };
}


#endif // RC_NET_H

