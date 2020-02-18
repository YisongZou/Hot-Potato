#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstring>
#include <iostream>

#include "helper.hpp"

using namespace std;

int main(int argc, char * argv[]) {
  if (argc != 3) {
    cout << "Syntax: player <machine_name> <port_num>\n" << endl;
    return 1;
  }

  int status;
  int socket_fd;
  struct addrinfo host_info;
  struct addrinfo * host_info_list;
  const char * hostname = argv[1];
  const char * port = argv[2];

  memset(&host_info, 0, sizeof(host_info));
  host_info.ai_family = AF_UNSPEC;
  host_info.ai_socktype = SOCK_STREAM;

  status = getaddrinfo(hostname, port, &host_info, &host_info_list);
  if (status != 0) {
    cerr << "Error: cannot get address info for host" << endl;
    cerr << "  (" << hostname << "," << port << ")" << endl;
    return -1;
  }  //if

  socket_fd = socket(host_info_list->ai_family,
                     host_info_list->ai_socktype,
                     host_info_list->ai_protocol);
  if (socket_fd == -1) {
    cerr << "Error: cannot create socket" << endl;
    cerr << "  (" << hostname << "," << port << ")" << endl;
    return -1;
  }  //if

  cout << "Connecting to " << hostname << " on port " << port << "..." << endl;

  status = connect(socket_fd, host_info_list->ai_addr, host_info_list->ai_addrlen);
  if (status == -1) {
    cerr << "Error: cannot connect to socket" << endl;
    cerr << "  (" << hostname << "," << port << ")" << endl;
    return -1;
  }  //if

  //////////////////initial information
  string player_id;
  string num_player;
  char buffer[512];
  recv(socket_fd, buffer, 512, 0);
  buffer[511] = 0;
  //  cout << "Player received: " << buffer << endl;
  string initial(buffer);
  initial_parser(player_id, num_player, initial);
  cout << "Connected as player " << player_id << " out of " << num_player
       << " total players " << endl;
  ///////////////////neighbor information
  string left_fd;
  string right_fd;
  memset(buffer, '\0', sizeof(buffer));
  recv(socket_fd, buffer, 512, 0);
  string neighbor(buffer);
  neighbor_parser(left_fd, right_fd, neighbor);
  cout << left_fd << "\n" << right_fd;

  const char * message = "Ready!";
  send(socket_fd, message, strlen(message), 0);

  freeaddrinfo(host_info_list);
  close(socket_fd);

  return 0;
}
