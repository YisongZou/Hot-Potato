#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstring>
#include <iostream>

#include "helper.hpp"
using namespace std;

void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}


int main(int argc, char * argv[]) {
  //Check if the number of command line arguments is correct
  if (argc != 4) {
    cerr << "Wrong number of command line arguments\n Usage: ringmaster <port_num> "
            "<num_players> <num_hops>\n";
    exit(EXIT_FAILURE);
  }

  int num_players = atoi(argv[2]);
  int num_hops = atoi(argv[3]);

  if (num_players <= 1 || num_hops < 0 || num_hops > 512) {
    cerr << "Wrong num_players or wrong num_hops, num_players must be greater than 1 and "
            "num_hops must be greater than or equal to zero and less than or equal to "
            "512\n";
    exit(EXIT_FAILURE);
  }

  // Initial message
  cout << "Potato Ringmaster\n";
  cout << "Players = " << num_players << endl;
  cout << "Hops = " << num_hops << endl;

  vector<int> player_fd(num_players, 0);  //Used to store all the player_fd
  vector<string> player_ip(num_players, " ");
  int status;
  int master_fd;
  struct addrinfo host_info;
  struct addrinfo * host_info_list;
  const char * hostname = "0.0.0.0";  //bind to address 0.0.0.0
  const char * port = argv[1];

  memset(&host_info, 0, sizeof(host_info));

  host_info.ai_family = AF_UNSPEC;
  host_info.ai_socktype = SOCK_STREAM;
  host_info.ai_flags = AI_PASSIVE;

  status = getaddrinfo(hostname, port, &host_info, &host_info_list);
  if (status != 0) {
    cerr << "Error: cannot get address info for ringmaster" << endl;
    cerr << "  (" << hostname << "," << port << ")" << endl;
    return -1;
  }  //if

  master_fd = socket(host_info_list->ai_family,
                     host_info_list->ai_socktype,
                     host_info_list->ai_protocol);
  if (master_fd == -1) {
    cerr << "Error: cannot create socket" << endl;
    cerr << "  (" << hostname << "," << port << ")" << endl;
    return -1;
  }  //if

  int yes = 1;
  status = setsockopt(master_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
  status = bind(master_fd, host_info_list->ai_addr, host_info_list->ai_addrlen);
  if (status == -1) {
    cerr << "Error: cannot bind socket" << endl;
    cerr << "  (" << hostname << "," << port << ")" << endl;
    return -1;
  }  //if

  status = listen(master_fd, 100);
  if (status == -1) {
    cerr << "Error: cannot listen on socket" << endl;
    cerr << "  (" << hostname << "," << port << ")" << endl;
    return -1;
  }  //if

  //  cout << "Waiting for connection on port " << port << endl;
 
  //Creat the players and send them information
  for (int i = 0; i < num_players; i++) {
    struct sockaddr_storage socket_addr;
    socklen_t socket_addr_len = sizeof(socket_addr);
    player_fd[i] = accept(master_fd, (struct sockaddr *)&socket_addr, &socket_addr_len);
    if (player_fd[i] == -1) {
      cerr << "Error: cannot accept connection on socket" << endl;
      return -1;
    }  //if
    if (socket_addr.ss_family == AF_INET) {
      char ip4[INET_ADDRSTRLEN];
      inet_ntop(AF_INET, get_in_addr((struct sockaddr * )& socket_addr), ip4, INET_ADDRSTRLEN);
      player_ip[i] = ip4;
    }
    else {
      char ip6[INET6_ADDRSTRLEN];
     inet_ntop(AF_INET6, get_in_addr((struct sockaddr * )& socket_addr), ip6, INET6_ADDRSTRLEN);
      player_ip[i] = ip6;
    }
    cout << "/" << player_ip[i] << "/" << endl;
    stringstream temp;
    temp << "Player No:";
    temp << i;
    temp << ",";
    temp << num_players;
    const char * message = temp.str().c_str();

    send(player_fd[i], message, strlen(message), 0);
  }

  usleep(1);
  //send neighbor information
  for (int j = 0; j < num_players; j++) {
    string left_ip;
    string right_ip;
    if (j == 0) {
      left_ip = player_ip[num_players - 1];
      right_ip = player_ip[j + 1];
    }
    else if (j == num_players - 1) {
      left_ip = player_ip[j - 1];
      right_ip = player_ip[0];
    }
    else {
      left_ip = player_ip[j - 1];
      right_ip = player_ip[j + 1];
    }
    string temp = "Neighbors:" + left_ip + "," + right_ip;
    const char * message = temp.c_str();
    send(player_fd[j], message, strlen(message), 0);
    char buffer[512];
    recv(player_fd[j], buffer, 512, 0);
    buffer[511] = 0;
    if (buffer[0] == 'R') {
      cout << "Player ";
      cout << j;
      cout << " is ready to play" << endl;
    }
  }

  freeaddrinfo(host_info_list);
  close(master_fd);

  return 0;
}
