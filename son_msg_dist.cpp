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


#include <iostream>
#include <vector>
#include <map>

#include "son_msg_dist.h"
#include "son_routing.h"

using namespace std;
using namespace Starsky;
SonMsgDist::SonMsgDist(SonRouting* routing) : _routing_info(routing){

}

SonRouting* SonMsgDist::GetRoutingInfo(){
  return _routing_info;
}
int SonMsgDist::GreedyDeliever(int source, int dest, int cur_hops){
  map<int, map<int, int>* >* global_routing_table = _routing_info->GetRoutingTable();
  if(NULL == global_routing_table){
    return -1;
  }
  map<int, int>* self_rtable = global_routing_table->count(source) == 0 ? NULL : (*global_routing_table)[source];
  if (NULL == self_rtable){
    return -1;
  }

  if (self_rtable->count(dest) == 0){
    map<int, multimap<int,int>* >* fwd_route = _routing_info->GetFwdRouteTable();
    if(fwd_route != NULL && fwd_route->count(source) != 0){
      multimap<int,int>* own_fwd_tbl = (*fwd_route)[source];
      if(own_fwd_tbl != NULL && own_fwd_tbl->count(dest) != 0){
        pair<multimap<int,int>::iterator,multimap<int,int>::iterator> range = own_fwd_tbl->equal_range(dest);
        multimap<int,int>::iterator it = range.first;
        //        for(it=range.first;it!=range.second;++it){
        return GreedyDeliever(it->second, dest, (cur_hops+1));
  //        }
      }
    }
  }else{
    return (cur_hops+1);
  }

  int cur_dist = GetDistance(source, dest);
  map<int, int>::iterator it_upper = self_rtable->upper_bound(dest);
  map<int, int>::iterator it_upper1 = it_upper;
  int next_node_addr = 0;
  if (it_upper == self_rtable->begin() || it_upper == self_rtable->end()){
    if (GetDistance((self_rtable->begin())->first, dest) < GetDistance((self_rtable->rbegin())->first, dest)) {
      next_node_addr = (self_rtable->begin())->first;
    }else {
      next_node_addr = (self_rtable->rbegin() )->first;
    }
  }else {
    it_upper1--;
    if (GetDistance(it_upper->first, dest) < GetDistance( it_upper1->first, dest) ) {
      next_node_addr = it_upper->first;
    }else {
      next_node_addr = it_upper1->first;
    }
  }
  int new_dist = GetDistance(next_node_addr, dest);
  if(new_dist < cur_dist){
    return GreedyDeliever(next_node_addr, dest, (cur_hops+1));
  }
  return -1;
}


int SonMsgDist::FloodDeliever(int sender, map<int, int>* dist, int cur_hops){
  map<int, map<int, int>* >* global_routing_table = _routing_info->GetRoutingTable();
  if(NULL == global_routing_table){
    return 0;
  }
  map<int, int>* self_rtable = global_routing_table->count(sender) == 0 ? NULL : (*global_routing_table)[sender];
  if (NULL == self_rtable){
    return 0;
  }

  map<int, int>::iterator rt_it;
  int msg_count = 0, msg_count_child = 0, temp_count=0;
  int taken_hops = cur_hops + 1;
  for(rt_it = self_rtable->begin();rt_it != self_rtable->end();rt_it++){
    if (dist->count(rt_it->first)){
      int prev_hops = (*dist)[rt_it->first];
      if (prev_hops == -1 || taken_hops < prev_hops){
        (*dist)[rt_it->first] = taken_hops;
        temp_count = FloodDeliever(rt_it->first, dist, taken_hops);
        if(prev_hops == -1){
          msg_count_child += temp_count;
        }
      }      
      msg_count++;
    }
  }
  return (msg_count+msg_count_child);
}
int SonMsgDist::MulticastDeliever(int source, map<int, int>* dist, int cur_hops){
  return 1;
}

int SonMsgDist::GetDistance(int addr_a, int addr_b) {
    int sm, bg;
    sm = min(addr_a, addr_b);
    bg = max(addr_a, addr_b);
    return (min( (bg-sm),( (0x7fffffff) - bg + sm) ));
}

