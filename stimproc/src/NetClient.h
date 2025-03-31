#ifndef NETCLIENT_H
#define NETCLIENT_H

#ifdef WIN32
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0501  // XP and later only
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <sys/socket.h>
#include <netdb.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#endif
#include <atomic>
#include <cstdint>
#include <iostream>
#include <sys/types.h>
#include <string.h>
#include <string>
#include <vector>
#include <memory>

#ifndef WIN32
#if defined(_WIN32) || defined(__WIN32__)
#define WIN32
#endif
#endif

/// \def Net_SockType
/// The standard socket type used in socket calls for a given system.
/// \def Net_InvSock
/// The value returned for an invalid socket.
/// \def Net_SockErr
/// The value returned for a socket error.
/// \def Net_CloseSock
/// The function for manually closing sockets.
/// \def Net_WouldBlock
/// The return value for a socket call which would block.
/// \def Net_SockErrorAt
/// The variable or function call for obtaining the last error.
/// \def Net_SockReturn
/// The return type for send and recv calls.
#ifdef WIN32
#define Net_SockType SOCKET
#define Net_InvSock INVALID_SOCKET
#define Net_SockErr SOCKET_ERROR
#define Net_CloseSock closesocket
#define Net_WouldBlock WSAEWOULDBLOCK
#define Net_SockErrorAt WSAGetLastError()
#define Net_SockReturn int
#else
#define Net_SockType int
#define Net_InvSock -1
#define Net_SockErr -1
#define Net_CloseSock close
#define Net_WouldBlock EWOULDBLOCK
#define Net_SockErrorAt errno
#define Net_SockReturn ssize_t
#endif


namespace SP {
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

      inline SockHelper(Net_SockType new_sock=Net_InvSock)
        : socket(new_sock),
          buf(4096),
          index(0),
          full_to(0) {
      }

      inline ~SockHelper() {
        Close();
      }

      inline void Close() {
        Net_SockType loc_soc;
        loc_soc = socket.exchange(Net_InvSock);
        if (loc_soc != Net_InvSock) {
          Net_CloseSock(loc_soc);
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

          Net_SockReturn bytes_recv;

          if (amnt_wanted == size_t(-1)) {
            amnt_wanted = buf.size();
          }

          if (amnt_wanted > buf.size()) {
            buf.resize(amnt_wanted);
          }

          if (index + amnt_wanted > buf.size()) {
            std::copy(buf.begin()+index, buf.begin()+index+amnt_left,
                      buf.begin());
            index = 0;
            full_to = amnt_left;
            amnt_wanted = amnt_wanted - amnt_left;
          }

          bytes_recv = recv(socket, reinterpret_cast<char*>(buf.data()) +
            full_to, amnt_wanted, 0);

          if (bytes_recv == Net_SockErr) {
            if (Net_SockErrorAt == Net_WouldBlock) {
              continue;
            }
            throw std::runtime_error("Socket receive error");
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

          Net_SockReturn bytes_sent;
          bytes_sent = send(socket, arr_current, len, 0);

          if (bytes_sent == Net_SockErr) {
            if (Net_SockErrorAt == Net_WouldBlock) {
              continue;
            }
            throw std::runtime_error("Socket send error");
          }

          arr_current += bytes_sent;
          len_left -= bytes_sent;
        } while (do_block && len_left > 0);

        return len;
      }

      std::atomic<Net_SockType> socket;

      std::vector<uint8_t> buf;
      size_t index;
      size_t full_to;
    };

    std::shared_ptr<SockHelper> helper;

    /// @endcond

    public:

    /// Encapsulate a new socket, receiving the base socket type of the
    /// system (int or SOCKET).
    /** Use Connect to initialize this.
     */
    inline Sock(Net_SockType new_sock=Net_InvSock) {
      helper = std::make_shared<SockHelper>(new_sock);

      if (new_sock != Net_InvSock) {
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
          throw std::runtime_error("Could not set socket non-blocking!");
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

    /// Return the raw encapsulated socket.
    inline Net_SockType Raw() const {
      return helper->socket;
    }


    /// Receive characters into str until a newline or the end of data.
    /** @param str The string into which a line is received.
     *  @param crop_newline If true, removes "\r\n" and "\n" from the end.
     *  @param do_block If true, keeps reading until newline or the socket
     *  closes.  If false, reads until newline or no data is available.
     *  @return The number of bytes received, including null characters and
     *  newlines not added to str.
     */
    inline size_t Recv(std::string& str, bool crop_newline = true,
                       bool do_block = true) {
      std::string inp;
      inp.reserve(4096);

      size_t count = 0;

      while (1) {
        if (helper->full_to == helper->index) {
          helper->ReadMore(-1, do_block);
          if (helper->full_to == 0) {
            break;
          }
        }

        char c = helper->buf.at(helper->index);
        ++(helper->index);
        count++;

        if (c == '\0') {
          break;
        }
        if (c == '\n') {
          if (!crop_newline) {
            inp += c;
          }
          break;
        }

        inp += c;
      }

      if (crop_newline) {
        while (inp.size() > 0 && inp[inp.size()-1] == '\r') {
          inp.resize(inp.size()-1);
        }
      }

      str = inp;
      std::cout << "Recv: " << str << std::endl;  // Testing

      return count;
    }


    /// Sends the contents of str through the socket with a newline added.
    /** @param str The string containing the data to send.
     *  @param do_block If true, sends all of str.  If false, sends as much of
     *  str as can be sent.
     *  @return The amount of data sent.
     */
    inline size_t Send(std::string str, bool do_block = true,
        bool add_newline = true) {
      if (add_newline) {
        str += "\n";
      }
      std::cout << "Send: " << str;  // Testing
      if (add_newline) {             // Testing
        std::cout.flush();           // Testing
      }                              // Testing
      else {                         // Testing
        std::cout << std::endl;      // Testing
      }                              // Testing
      return helper->SendData(str.c_str(), str.size(), do_block);
    }
  };


  /// Provides client side of blocking TCP connections.
  class Net {
    public:

    /// Automatically called by Connect.
    /** The first time this is called, it sets SIGPIPE to ignore so the
     *  program doesn't terminate while writing to a closed socket.  Call
     *  this once manually beforehand if you want to set your own custom
     *  handler for SIGPIPE.
     */
    static inline void InitializeSockets() {
      static thread_local bool already_run = false;
      if (! already_run) {
#ifdef WIN32
        WSADATA wsa_data;
        int err;
        err = WSAStartup(0x0202, &wsa_data);
        if (err != 0) {
          throw std::runtime_error("WSAStartup error");
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
    static inline bool Connect(Sock &connection, const std::string& host,
        const std::string& port, bool quiet_fail=false) {
      struct addrinfo hints;
      struct addrinfo *ai_list, *addr;
      Net_SockType sfd = Net_InvSock;

      InitializeSockets();
      connection.Close();

      memset(&hints, 0, sizeof(hints));
      hints.ai_family = AF_UNSPEC;  // Support IPv4 && IPv6
      hints.ai_socktype = SOCK_STREAM;  // For TCP

      int aierr = getaddrinfo(host.c_str(), port.c_str(), &hints, &ai_list);
      if (aierr != 0) {
        if (! quiet_fail) {
          throw std::runtime_error(gai_strerror(aierr));
        }
        return false;
      }

      for (addr = ai_list; addr != NULL; addr = addr->ai_next) {
        sfd = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);

        if (sfd == Net_InvSock) {
          continue;
        }

        if (connect(sfd, addr->ai_addr, addr->ai_addrlen) != Net_SockErr) {
          break;  // Connected.
        }

        Net_CloseSock(sfd);
      }

      freeaddrinfo(ai_list);

      // If could not find one
      if (addr == NULL) {
        if (! quiet_fail) {
          throw std::runtime_error("Connection failed");
        }
        return false;
      }

      connection = Sock(sfd);

      return true;
    }
  };
}


#endif

