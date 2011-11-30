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
#include <math.h>
#include "p2p_network.h"
#include "son_util.h"

using namespace Starsky;
using namespace std;

P2PNetwork::P2PNetwork(int net_size) : _net_size(net_size){
  _node_list = new map<int, int>();    
  _routing_table = new map<int, map<int,int>* >();
  CreateNodeIDs();
}

void P2PNetwork::CreateNodeIDs(){
  for(int i=0;i<_net_size;i++){
    int id = rand();
    while(_node_list->count(id) != 0){
      id = rand();
    }
    _node_list->insert(pair<int,int>(id, 1));
    map<int,int>* each_rt = new map<int,int>();
    (*_routing_table)[id] = each_rt;
  }
}


bool P2PNetwork::CreateEdge(int host, int target, int mode){
  if((_routing_table->count(host) == 0) || (_routing_table->count(target) == 0) || host == target){
    return false;
  }
  map <int, int>* n1_table = (*_routing_table)[host];
  map <int, int>* n2_table = (*_routing_table)[target];
  if(n1_table == NULL || n2_table == NULL){
    return false;
  }
  if(n1_table->count(target) != 0 || n2_table->count(host)!= 0 ){
    return false;
  }
  (*n1_table)[target] = mode;
  (*n2_table)[host] = mode;
  return true;
}

map<int,int>* P2PNetwork::GetNodeLists(){
  return _node_list;
}

map<int, map<int,int>* >*  P2PNetwork::GetRoutingTable(){
  return _routing_table;
}

void P2PNetwork::PrintRoutingTable(){
  map<int, map<int,int>*>::iterator rt_it;
  for(rt_it = _routing_table->begin();rt_it!=_routing_table->end();rt_it++){
    cout << rt_it->first << endl;
    map<int,int>::iterator ele_it;
    for(ele_it = rt_it->second->begin();ele_it!=rt_it->second->end();ele_it++){
      cout << ele_it->first <<"\t";
    }
    cout << endl;
  }
}

void P2PNetwork::CreateNetwork(){}
void P2PNetwork::CheckConnection(){}

SymphonyP2P::SymphonyP2P(int net_size, int shortcut_num) : P2PNetwork(net_size), _num_shortcut(shortcut_num){
  CreateNetwork();
}

void SymphonyP2P::CreateNetwork(){
  FormRing();
  CreateShortcuts();
}

void SymphonyP2P::FormRing(){
  map<int, int>::iterator node_it;
  int predecessor = _node_list->rbegin()->first;
  for(node_it = _node_list->begin();node_it != _node_list->end();node_it++){
    CreateEdge(node_it->first, predecessor, NEAR_CONN);
    predecessor = node_it->first;
  }
}  

void SymphonyP2P::CreateShortcuts(){
  int add_count=0, e10=log10(RAND_MAX);
  map<int, int>::iterator node_it;
  for(node_it=_node_list->begin();node_it != _node_list->end();node_it++){
    add_count = 0;
    int host_id = node_it->first, consecutive_fail = 0;
    while(add_count < _num_shortcut && consecutive_fail < 20){
      float fr = (float)rand()/(float)RAND_MAX;
      int k = (int)(pow(10,(e10-(1-fr)*log10(_net_size))));
      k &= 0x7fffffff;
      unsigned int temp = (unsigned int)host_id + (unsigned int)k;
      int shortcut_target_addr = temp%0x7fffffff;
      int actual_sc = SonUtil::GetClosestNode(host_id, _node_list, shortcut_target_addr);
      if(CreateEdge(host_id, actual_sc, SHORTCUT) == false){
        consecutive_fail++;
      }else{
        add_count++;
        consecutive_fail = 0;
      }
    }
  }
}

void SymphonyP2P::CheckConnection(){
  map<int, int>::iterator nm_it;
  int pred_addr = _node_list->rbegin()->first;
  bool has_succ_conn = true;
  for(nm_it = _node_list->begin();nm_it != _node_list->end();nm_it++){
    int this_node = nm_it->first;
    map<int,int>* rt = (*_routing_table)[this_node];
    if(rt->count(pred_addr) == 0){
      cout << "ring is not formed" << endl;
      return;
    }
    cout << rt->size() << endl;
    pred_addr = this_node;
  }
  cout << "ring is formed correctly" << endl;

}

UnstructuredP2P::UnstructuredP2P(int net_size, int conn_num): P2PNetwork(net_size), _conn_num(conn_num){
  CreateNetwork();
}

void UnstructuredP2P::CreateNetwork(){
  map<int,int>::iterator node_it;
  map<int,int>::iterator node_advance_it;
  for(node_it=_node_list->begin();node_it!=_node_list->end();node_it++){
    node_advance_it = node_it;
    int rotate_index = 0;
    for(int i=0;i<_conn_num;i++){
      node_advance_it++;
      if(node_advance_it == _node_list->end()){
        node_advance_it = _node_list->begin();
      }
      CreateEdge(node_it->first, node_advance_it->first, UNSTRUCTURED);
    }
  }
}

void UnstructuredP2P::CheckConnection(){
  map<int, map<int,int>* >::iterator rt_it;
  for(rt_it = _routing_table->begin();rt_it != _routing_table->end();rt_it++){
    if(rt_it->second->size() != (2*_conn_num)){
      cout << "unstructured connection is not correct : size : " << rt_it->second->size()  <<  endl;
      return;
    }
  }
  cout << "unstructured connection is formed correctly" << endl;
}

SuperPeerP2P::SuperPeerP2P(int net_size, int super_peer_num, int sp_conn_num) : UnstructuredP2P(super_peer_num, sp_conn_num){
  _superpeer_map = new map<int,int>();
  _non_sp_nodes = new P2PNetwork(net_size);
  DetermineSuperpeer();
}

SuperPeerP2P::SuperPeerP2P(int super_peer_num,int sp_conn_num, P2PNetwork* base_network) : UnstructuredP2P(super_peer_num, sp_conn_num){
  _superpeer_map = new map<int,int>();
  _non_sp_nodes = base_network;
  DetermineSuperpeer();
}
map<int,int>* SuperPeerP2P::GetSuperpeerMap(){
  return _superpeer_map;
}

void SuperPeerP2P::DetermineSuperpeer(){
  map<int,int>* non_sp_nodes = _non_sp_nodes->GetNodeLists();
  map<int,int>::iterator nsp_it;
  map<int, vector<int> > temp_count;
  for(nsp_it=non_sp_nodes->begin();nsp_it!=non_sp_nodes->end();nsp_it++){
    map<int,int>::iterator ub_it = _node_list->upper_bound(nsp_it->first);
    if(ub_it == _node_list->end()){
      ub_it--;
    }
    _superpeer_map->insert(pair<int,int>(nsp_it->first, ub_it->first));
    (temp_count[ub_it->first]).push_back(nsp_it->first);
  }
  map<int, vector<int> >::iterator smap_it;
  for(smap_it=temp_count.begin();smap_it!=temp_count.end();smap_it++){
    cout << smap_it->first << " : size = " << smap_it->second.size() << endl;
  }
}

map<int,int>* SuperPeerP2P::GetNodeLists(){
  return _non_sp_nodes->GetNodeLists();
}

