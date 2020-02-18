#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstring>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

void initial_parser(string & player_id, string & num_player, string & initial) {
  size_t position = initial.find("Player No:");
  size_t i = position;
  while (i < initial.size()) {
    if (initial[i] != ':') {
      i++;
    }
    else {
      i++;
      break;
    }
  }
  while (i < initial.size()) {
    if (initial[i] != ',') {
      player_id += initial[i];
      i++;
    }
    else {
      i++;
      break;
    }
  }
  while (i < initial.size()) {
    if (initial[i] != ' ') {
      num_player += initial[i];
      i++;
    }
    else {
      break;
    }
  }
}

void neighbor_parser(string & left_fd, string & right_fd, string & neighbor) {
  size_t position = neighbor.find("Neighbors:");
  size_t i = position;
  while (i < neighbor.size()) {
    if (neighbor[i] != ':') {
      i++;
    }
    else {
      i++;
      break;
    }
  }
  while (i < neighbor.size()) {
    if (neighbor[i] != ',') {
      left_fd += neighbor[i];
      i++;
    }
    else {
      i++;
      break;
    }
  }
  while (i < neighbor.size()) {
    if (neighbor[i] != ' ') {
      right_fd += neighbor[i];
      i++;
    }
    else {
      break;
    }
  }
}
