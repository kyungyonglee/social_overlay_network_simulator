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
  _global_routing_table = _routing_info->GetRoutingTable();
  _msg_overhead = new map<int, SonStatistics*>();
  _fwd_route = _routing_info->GetFwdRouteTable();
  _failed_nodes = new map<int,int>();
}

SonRouting* SonMsgDist::GetRoutingInfo(){
  return _routing_info;
}
int SonMsgDist::GreedyDeliever(map<int,int>* msg_recipients, int source, int dest, int cur_hops, int* no_friend_hops){
  map<int, int>* self_rtable = _global_routing_table->count(source) == 0 ? NULL : (*_global_routing_table)[source];  
  multimap<int,int>* own_fwd_tbl = _fwd_route->count(source)!= 0 ? (*_fwd_route)[source]:NULL;
  if (NULL == self_rtable){
    return -1;
  }
  if(msg_recipients->count(source) == 0){
    (*no_friend_hops) += 1;
  }
  if(source == dest){
    return cur_hops;
  }
  if (self_rtable->count(dest) == 0){
    if(own_fwd_tbl != NULL && own_fwd_tbl->count(dest) != 0){
      pair<multimap<int,int>::iterator,multimap<int,int>::iterator> range = own_fwd_tbl->equal_range(dest);
      multimap<int,int>::iterator it = range.first;
      //        for(it=range.first;it!=range.second;++it){
      return GreedyDeliever(msg_recipients, it->second, dest, (cur_hops+1), no_friend_hops);
//        }
    }
  }else{
    return (cur_hops+1);
  }
  map<int,int> temp_added_entry;
  if(own_fwd_tbl != NULL){
    multimap<int,int>::iterator sfwd_it;
    for(sfwd_it=own_fwd_tbl->begin();sfwd_it!=own_fwd_tbl->end();sfwd_it++){
      temp_added_entry[sfwd_it->first]=1;
      self_rtable->insert(pair<int,int>(sfwd_it->first,1));
    }
  }
  int cur_dist = SonUtil::GetDistance(source, dest);
  map<int, int>::iterator it_upper = self_rtable->upper_bound(dest);
  map<int, int>::iterator it_upper1 = it_upper;
  int next_node_addr = 0;
  if (it_upper == self_rtable->begin() || it_upper == self_rtable->end()){
    if (SonUtil::GetDistance((self_rtable->begin())->first, dest) < SonUtil::GetDistance((self_rtable->rbegin())->first, dest)) {
      next_node_addr = (self_rtable->begin())->first;
    }else {
      next_node_addr = (self_rtable->rbegin() )->first;
    }
  }else {
    it_upper1--;
    if (SonUtil::GetDistance(it_upper->first, dest) < SonUtil::GetDistance( it_upper1->first, dest) ) {
      next_node_addr = it_upper->first;
    }else {
      next_node_addr = it_upper1->first;
    }
  }
  map<int,int>::iterator temp_it;
  for(temp_it=temp_added_entry.begin();temp_it!=temp_added_entry.end();temp_it++){
    self_rtable->erase(temp_it->first);
  }
  int new_dist = SonUtil::GetDistance(next_node_addr, dest);
  if(new_dist < cur_dist){
    return GreedyDeliever(msg_recipients, next_node_addr, dest, (cur_hops+1), no_friend_hops);
  }
  return -1;
}


int SonMsgDist::FloodDeliever(int sender, map<int, int>* dist, int cur_hops, int ttl){
  map<int, int>* self_rtable = _global_routing_table->count(sender) == 0 ? NULL : (*_global_routing_table)[sender];
  multimap<int, int>* fwd_table = _fwd_route->count(sender) == 0 ? NULL : (*_fwd_route)[sender];
  if (NULL == self_rtable){
    return 0;
  }

  map<int, int>::iterator rt_it;
  int msg_count = 0, msg_count_child = 0, temp_count=0;
  int taken_hops = cur_hops + 1;
  map<int,int>* temp_dist_list = new map<int,int>() ;
  
  for(rt_it = dist->begin();rt_it != dist->end();rt_it++){
    if(self_rtable->count(rt_it->first) == 0){
      temp_dist_list->insert(pair<int,int>(rt_it->first, rt_it->second));
    }
  }
  
  for(rt_it = self_rtable->begin();rt_it != self_rtable->end();rt_it++){
    if (dist->count(rt_it->first) != 0 && (taken_hops <= ttl || ttl < 0)){
      int prev_hops = (*dist)[rt_it->first];
      if (prev_hops == -1 || taken_hops < prev_hops){
        if(_failed_nodes->count(rt_it->first) != 0){
          continue;
        }        
        (*dist)[rt_it->first] = taken_hops;
 //       temp_count = FloodDeliever(rt_it->first, dist, taken_hops, ttl-1);
        temp_count = FloodDeliever(rt_it->first, temp_dist_list, taken_hops, ttl-1);
        if(prev_hops == -1){
          msg_count_child += temp_count;  // forward to its child nodes only once
        }
      }      
      msg_count++;   //try to forward to its friend
    }
  }
///*  
  map<int,int>::iterator result_it;
  for(result_it=temp_dist_list->begin();result_it!=temp_dist_list->end();result_it++){
      (*dist)[result_it->first] = result_it->second; 
  }
//*/  
  delete temp_dist_list;
  return (msg_count+msg_count_child);
}
int SonMsgDist::MulticastDeliever(int source, map<int, int>* org_recpt, map<int, int>* dist, int cur_hops){
  map<int,int>* self_routing = _global_routing_table->count(source) != 0 ? (*_global_routing_table)[source] : NULL;
  multimap<int,int>* self_fwd = _fwd_route->count(source) != 0 ? (*_fwd_route)[source] : NULL;
  map<int, map<int,int>* > node_assignment;
  int new_hops = cur_hops+1, total_msg=0;
  if(self_routing == NULL){
    return 0;
  }
  map<int,int>* intersect_list = SonUtil::GetInteresctEntry(self_routing, dist);
  multimap<int,int>::iterator fwd_it;
  if(self_fwd != NULL){
    for(fwd_it=self_fwd->begin();fwd_it!=self_fwd->end();fwd_it++){
      if(dist->count(fwd_it->first)!=0){
        (*intersect_list)[fwd_it->first] = 1;
      }
    }
  }
  map<int,int>::iterator dest_it;
  for(dest_it=dist->begin();dest_it!=dist->end();dest_it++){
    map<int,int>* per_node_assgn;
    int next_node = -1;
    if(source == dest_it->first){
      dest_it->second = cur_hops;
      continue;
    }
    if(self_routing->count(dest_it->first) != 0){
      next_node = dest_it->first;
    }else if(self_fwd != NULL && self_fwd->count(dest_it->first) != 0){
      pair<multimap<int,int>::iterator,multimap<int,int>::iterator> range = self_fwd->equal_range(dest_it->first);
      multimap<int,int>::iterator it = range.first;
      next_node = it->second;
    }else{
      next_node = SonUtil::GetClosestNode(source, intersect_list, dest_it->first);
    }
    if (next_node > 0){
      if(node_assignment.count(next_node) != 0){
        per_node_assgn = node_assignment[next_node];
      }else{
        per_node_assgn = new map<int,int>();
        node_assignment[next_node] = per_node_assgn;
      }
      per_node_assgn->insert(pair<int,int>(dest_it->first, -1));
    }else{
      if(self_fwd != NULL){
        int curr_dist = SonUtil::GetDistance(source, dest_it->first);
        multimap<int,int>::iterator sfwd_it;
        for(sfwd_it=self_fwd->begin();sfwd_it!=self_fwd->end();sfwd_it++){
          if(/*org_recpt->count(sfwd_it->first) != 0 && */SonUtil::GetDistance(sfwd_it->first, dest_it->first) < curr_dist){
            next_node = sfwd_it->first;
            break;
          }
        }
      }else{
  //      cout << source << " : " << dest_it->first << endl;
        map<int,int>::iterator sri;
        for(sri=dist->begin();sri!=dist->end();sri++){
//          cout << sri->first << endl;
        }
      }
    }
  }
  delete intersect_list;
  map<int, map<int,int>*>::iterator na_it;
  for(na_it=node_assignment.begin();na_it != node_assignment.end();na_it++){    
    if(_failed_nodes->count(na_it->first) != 0){
      continue;
    }    
    total_msg++;
    total_msg += MulticastDeliever(na_it->first, org_recpt, na_it->second, cur_hops+1);
    map<int,int>::iterator res_it;
    for(res_it = na_it->second->begin();res_it != na_it->second->end();res_it++){
      (*dist)[res_it->first] = res_it->second;
    }
    delete na_it->second;
  }
  return total_msg;
}

void SonMsgDist::UpdateMsgOverhead(int id, int total_msg){
  SonStatistics* son_stat_rt;
  if(_msg_overhead->count(id) != 0){
    son_stat_rt = (*_msg_overhead)[id];
  }else{
    son_stat_rt = new SonStatistics(id);
    (*_msg_overhead)[id] = son_stat_rt;
  }
  map<int,int>* temp = new map<int,int>();
  temp->insert(pair<int,int>(id, total_msg));
  son_stat_rt->UpdateStat(temp);
  delete temp;
}

void SonMsgDist::PrintMsgOverhead(){
  map<int, SonStatistics*>::iterator ss_it;
  map<int, SonStatistics*>* summ_result = SonUtil::SummarizeStat(_msg_overhead);
  cout << endl << endl;
  for(ss_it=summ_result->begin();ss_it != summ_result->end();ss_it++){
    ss_it->second->PrintStat();
  }  
  delete summ_result;
}

void SonMsgDist::SetCrashNodes(int target_number){
  map<int,map<int, int>* >::iterator node_it;
  while(target_number > 0){
    node_it = _global_routing_table->begin();
    advance(node_it, rand()%_global_routing_table->size());
    if(_failed_nodes->count(node_it->first) == 0){
      _failed_nodes->insert(pair<int,int>(node_it->first, 1));
      target_number--;
    }
  }
}

