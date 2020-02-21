#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstring>
#include <iostream>

#include "helper.hpp"
#include "potato.hpp"
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

  //  cout << "Connecting to " << hostname << " on port " << port << "..." << endl;

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
  recv(socket_fd, buffer, 512, 0);
  string neighbor(buffer);
  neighbor_parser(left_ip, right_ip, neighbor);

  const char * message = "Ready!";
  send(socket_fd, message, strlen(message), 0);

  int client_connection_fd;
  int status_server;
  int server_fd;
  struct addrinfo server_info;
  struct addrinfo * server_info_list;
  const char * server_hostname = NULL;
  int server_port_num = atoi(player_id.c_str()) + atoi(port) + 1;
  const char * server_port = to_string(server_port_num).c_str();
  ///////////////As the server of the right player
  //The players except for player 0 establishes as clients first, the player 0 establishes first as server  and  start to shape the ring
  if (player_id == "0") {
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

    //cout << "Waiting for connection on port " << server_port << endl;
    struct sockaddr_storage socket_addr;
    socklen_t socket_addr_len = sizeof(socket_addr);
    //int client_connection_fd;
    client_connection_fd =
        accept(server_fd, (struct sockaddr *)&socket_addr, &socket_addr_len);
    if (client_connection_fd == -1) {
      cerr << "Error: cannot accept connection on socket" << endl;
      return -1;
    }  //if

  }

  memset(buffer, '\0', sizeof(buffer));  //Reset buffer
  recv(socket_fd, buffer, 512, 0);
  //  cout << buffer << endl;
  //////////////As the client of the left player
  int client_status;
  int client_fd;
  struct addrinfo client_info;
  struct addrinfo * client_info_list;
  const char * client_hostname = left_ip.c_str();
  int client_port_num = 0;
  if (atoi(player_id.c_str()) == 0) {
    client_port_num = atoi(port) + atoi(num_player.c_str());
  }
  else {
    client_port_num = atoi(player_id.c_str()) + atoi(port);
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

  // cout << "Connecting to " << client_hostname << " on port " << client_port << "..."<< endl;

  client_status =
      connect(client_fd, client_info_list->ai_addr, client_info_list->ai_addrlen);
  if (client_status == -1) {
    cerr << "Error: cannot connect to socket" << endl;
    cerr << "  (" << client_hostname << "," << client_port << ")" << endl;
    return -1;
  }  //if

  ///////////////The other players except for player0  establish as server later
  if (player_id != "0") {
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

    //    cout << "Waiting for connection on port " << server_port << endl;
    struct sockaddr_storage socket_addr;
    socklen_t socket_addr_len = sizeof(socket_addr);
    //int client_connection_fd;
    client_connection_fd =
        accept(server_fd, (struct sockaddr *)&socket_addr, &socket_addr_len);
    if (client_connection_fd == -1) {
      cerr << "Error: cannot accept connection on socket" << endl;
      return -1;
    }  //if

  }

  potato temp_potato;
  temp_potato.count = 0;
  temp_potato.hops = 0;
  memset(temp_potato.ip, '\0', sizeof(temp_potato.ip));
  recv(socket_fd, &temp_potato, sizeof(temp_potato), 0);
  temp_potato.ip[temp_potato.count] = player_id[0];
  //  cout << "Count" <<temp_potato.count <<  endl;
  temp_potato.count++;

  //If Get the real potato, send it
  if(temp_potato.hops == 0){
    cout << "I’m it"<<endl;
    send(socket_fd, &temp_potato, sizeof(temp_potato), 0);
  }
  else if (temp_potato.hops != -1) {
    temp_potato.hops--;
    srand((unsigned int)time(NULL) + atoi(player_id.c_str()));
    int random = rand() % (2);
    //cout << "Random:" << random << endl;
    if (random == 0) {
      if(player_id != "0"){
	cout << "Sending potato to " << atoi(player_id.c_str()) -1 << endl;
      }
      else{
	cout << "Sending potato to " << atoi(num_player.c_str()) -1 << endl;
      }
      send(client_fd, &temp_potato, sizeof(temp_potato), 0);
      //cout << "left"<<endl;
    }
    else {
      if(atoi(player_id.c_str()) != atoi(num_player.c_str()) - 1){
        cout << "Sending potato to " << atoi(player_id.c_str())  + 1 << endl;
      }
      else{
        cout << "Sending potato to " << 0 << endl;
      }
      send(client_connection_fd, &temp_potato, sizeof(temp_potato), 0);
      // cout << "right"<<endl;
    }
  }

  
 
  ///////////////////// //If did not get the potato, start selecting
  int n = client_connection_fd + 1;
  if (client_fd > client_connection_fd) {
    n = client_fd + 1;
  }
  srand((unsigned int)time(NULL) + atoi(player_id.c_str()));
  int rv;
  while (1) {
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(client_connection_fd, &readfds);
    FD_SET(client_fd, &readfds);
    FD_SET(socket_fd, &readfds);
    rv = select(n, &readfds, NULL, NULL, NULL);

    if (rv == -1) {
      cerr << "Error in select" << endl;
      return -1;
      // error occurred in select()
    }
    else {
      //End the game
      if (FD_ISSET(socket_fd, &readfds)) {
	//      cout<<"Game ends"<<endl;
      break;
      }
      // one or both of the descriptors have data
      //Potato from right client
     else if (FD_ISSET(client_connection_fd, &readfds)) {
	if (FD_ISSET(socket_fd, &readfds)) {
	  //  cout<<"Game ends"<<endl;
        break;
      }
	potato new_potato;
	new_potato.count = 0;
	new_potato.hops = 0;
	memset(new_potato.ip, '\0', sizeof(new_potato.ip));
	recv(client_connection_fd, &new_potato, sizeof(new_potato), 0);
	new_potato.count++;
	new_potato.ip[new_potato.count -1 ] = player_id[0];
	new_potato.hops--;
        if (new_potato.hops == 0) {
          //End the game
	  cout << "I’m it"<<endl;
          send(socket_fd, &new_potato, sizeof(new_potato), 0);
	}
        else {
          //Continue sending
          int random = rand() % (2);
          if (random == 0) {
	    if(player_id != "0"){
        cout << "Sending potato to " << atoi(player_id.c_str()) -1 << endl;
      }
      else{
        cout << "Sending potato to " << atoi(num_player.c_str()) -1 << endl;
      }
            send(client_fd, &new_potato, sizeof(new_potato), 0);
	    continue;
	  }
          else {
	    if(atoi(player_id.c_str()) != atoi(num_player.c_str()) - 1){
        cout << "Sending potato to " << atoi(player_id.c_str())  + 1 << endl;
      }
      else{
        cout << "Sending potato to " << 0 << endl;
      }
            send(client_connection_fd, &new_potato, sizeof(new_potato), 0);
	    continue;
	  }
        }
      }
      //Potato from server
     else if (FD_ISSET(client_fd, &readfds)) {
	if (FD_ISSET(socket_fd, &readfds)) {
	  //cout<<"Game ends"<<endl;
        break;
      }
	potato new_potato;
	new_potato.count = 0;
	new_potato.hops = 0;
	memset(new_potato.ip, '\0', sizeof(new_potato.ip));
	recv(client_fd, &new_potato, sizeof(new_potato), 0);
	new_potato.count++;	
	new_potato.ip[new_potato.count-1] = player_id[0];
	new_potato.hops--;

        if (new_potato.hops == 0) {
          //End the game
	  cout << "I’m it"<<endl;
	  send(socket_fd, &new_potato, sizeof(new_potato), 0);
	  break;
	}
        else {
          //Continue sending
          int random = rand() % (2);
          if (random == 0) {
	    if(player_id != "0"){
        cout << "Sending potato to " << atoi(player_id.c_str()) -1 << endl;
      }
      else{
        cout << "Sending potato to " << atoi(num_player.c_str()) -1 << endl;
      }
            send(client_fd, &new_potato, sizeof(new_potato), 0);
	    continue;
	  }
          else {
	    if(atoi(player_id.c_str()) != atoi(num_player.c_str()) - 1){
        cout << "Sending potato to " << atoi(player_id.c_str())  + 1 << endl;
      }
      else{
        cout << "Sending potato to " << 0 << endl;
      }
            send(client_connection_fd, &new_potato, sizeof(new_potato), 0);
	    continue;
	  }
        }
      }
    }
  }

  freeaddrinfo(server_info_list);
  close(server_fd);
  freeaddrinfo(host_info_list);
  close(socket_fd);
  freeaddrinfo(client_info_list);
  close(client_fd);
  return 0;
}
