/*
This program is part of Netmodeler, a library for graph and network
modeling and simulation.
Copyright (C) 2011  University of Florida
Copyright (C) 2011  Kyungyong Lee (klee@acis.ufl.edu), University of Florida

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

//#include <stdio.h>
#include <iostream>
#include "p2p_action.h"
#include "son_util.h"
#include "p2p_message.h"

using namespace Starsky;
using namespace std;

GlobalClass::GlobalClass(){
}

P2PRdResponse::P2PRdResponse(){
  Response = new map<int,int>();
}

P2PRdResponse::~P2PRdResponse(){
  delete Response;
}


P2PStat::P2PStat(){
  TotalNodes = 0;
  MaxDepth = 0;
}

P2PNodeFailure::P2PNodeFailure(map<int,int>* node_list, int num_failed_node):_num_failed_nodes(num_failed_node){
  if(node_list == NULL){
    return;
  }
  int net_size = node_list->size();  
  int candidate; 
  map<int,int>::iterator nl_it;
  for(int added = 0;(added<_num_failed_nodes)&&(added<net_size);){
    nl_it = node_list->begin();
    advance(nl_it, rand()%net_size);
    candidate = nl_it->first;
    if(_failed_nodes.count(candidate) == 0){
      _failed_nodes[candidate] = 1;
      added++;
    }
  }
}

bool P2PNodeFailure::AddFailedNode(int failed_node_id){
  if(_failed_nodes.count(failed_node_id) == 0){
    _failed_nodes[failed_node_id] = 1;
    return true;
  }
  return false;
}

bool P2PNodeFailure::CheckIfFailed(int test_node_id){
  return (_failed_nodes.count(test_node_id) == 0 ? false : true);
}

ResDiscResult::ResDiscResult(){
  Count= 0;
  Hops = 0;
  TotalMessages = 0;
  InCompleteness = 0.0;
  AvgResultAge = 0;
  FalseResult = 0;
}

