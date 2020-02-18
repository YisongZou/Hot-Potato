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
  string left_ip;
  string right_ip;
  memset(buffer, '\0', sizeof(buffer));  //Reset buffer
  cout << "Here!";
  recv(socket_fd, buffer, 512, 0);
  string neighbor(buffer);
  neighbor_parser(left_ip, right_ip, neighbor);
  cout << left_ip << "\n" << right_ip;

  const char * message = "Ready!";
  send(socket_fd, message, strlen(message), 0);

  ///////////////As the server of the right player
  int status_server;
  int server_fd;
  struct addrinfo server_info;
  struct addrinfo * server_info_list;
  const char * server_hostname = "0.0.0.0";
  int server_port_num = atoi(player_id.c_str()) + atoi(port) + 1;
  const char * server_port = to_string(server_port_num).c_str();

  memset(&server_info, 0, sizeof(server_info));

  server_info.ai_family = AF_UNSPEC;
  server_info.ai_socktype = SOCK_STREAM;
  server_info.ai_flags = AI_PASSIVE;

  status_server =
      getaddrinfo(server_hostname, server_port, &server_info, &server_info_list);
  if (status_server != 0) {
    cerr << "Error: cannot get address info for host" << endl;
    cerr << "  (" << server_hostname << "," << server_port << ")" << endl;
    return -1;
  }  //if

  server_fd = socket(server_info_list->ai_family,
                     server_info_list->ai_socktype,
                     server_info_list->ai_protocol);
  if (server_fd == -1) {
    cerr << "Error: cannot create socket" << endl;
    cerr << "  (" << server_hostname << "," << server_port << ")" << endl;
    return -1;
  }  //if

  int server_yes = 1;
  status_server =
      setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &server_yes, sizeof(int));
  status_server =
      bind(server_fd, server_info_list->ai_addr, server_info_list->ai_addrlen);
  if (status == -1) {
    cerr << "Error: cannot bind socket" << endl;
    cerr << "  (" << server_hostname << "," << server_port << ")" << endl;
    return -1;
  }  //if

  status_server = listen(server_fd, 100);
  if (status == -1) {
    cerr << "Error: cannot listen on socket" << endl;
    cerr << "  (" << server_hostname << "," << server_port << ")" << endl;
    return -1;
  }  //if

  cout << "Waiting for connection on port " << server_port << endl;
  struct sockaddr_storage socket_addr;
  socklen_t socket_addr_len = sizeof(socket_addr);
  int client_connection_fd;
  client_connection_fd =
      accept(server_fd, (struct sockaddr *)&socket_addr, &socket_addr_len);
  if (client_connection_fd == -1) {
    cerr << "Error: cannot accept connection on socket" << endl;
    return -1;
  }  //if

  memset(buffer, '\0', sizeof(buffer));
  recv(client_connection_fd, buffer, 512, 0);
  //buffer[511] = 0;

  cout << "Server received: " << buffer << endl;

  freeaddrinfo(server_info_list);
  close(server_fd);

  //////////////As the client of the left player
  int client_status;
  int client_fd;
  struct addrinfo client_info;
  struct addrinfo * client_info_list;
  const char * client_hostname =
      "127.0.0.1";  ///////////////////////////////////need to modify
  int client_port_num = 0;
  if (atoi(player_id.c_str()) == atoi(num_player.c_str()) - 1) {
    client_port_num = 1 + atoi(port);
  }
  else {
    client_port_num = atoi(player_id.c_str()) + 2 + atoi(port);
  }
  const char * client_port = to_string(client_port_num).c_str();

  memset(&client_info, 0, sizeof(client_info));
  client_info.ai_family = AF_UNSPEC;
  client_info.ai_socktype = SOCK_STREAM;

  client_status =
      getaddrinfo(client_hostname, client_port, &client_info, &client_info_list);
  if (client_status != 0) {
    cerr << "Error: cannot get address info for host" << endl;
    cerr << "  (" << client_hostname << "," << client_port << ")" << endl;
    return -1;
  }  //if

  client_fd = socket(client_info_list->ai_family,
                     client_info_list->ai_socktype,
                     client_info_list->ai_protocol);
  if (client_fd == -1) {
    cerr << "Error: cannot create socket" << endl;
    cerr << "  (" << client_hostname << "," << client_port << ")" << endl;
    return -1;
  }  //if

  cout << "Connecting to " << client_hostname << " on port " << client_port << "..."
       << endl;

  client_status =
      connect(client_fd, client_info_list->ai_addr, client_info_list->ai_addrlen);
  if (client_status == -1) {
    cerr << "Error: cannot connect to socket" << endl;
    cerr << "  (" << client_hostname << "," << client_port << ")" << endl;
    return -1;
  }  //if

  const char * message_left = "hi there!";
  send(socket_fd, message_left, strlen(message), 0);

  freeaddrinfo(host_info_list);
  close(socket_fd);

  //////////////////////////////////////////////
  char host_name[128];
  if (gethostname(host_name, sizeof(host_name)) == -1) {
    cerr << "Error: cannot gethostname" << endl;
    return -1;
  }
  cout << "hostname:" << host_name << endl;
  freeaddrinfo(host_info_list);
  close(socket_fd);

  return 0;
}
