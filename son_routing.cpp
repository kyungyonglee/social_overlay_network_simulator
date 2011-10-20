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

#include <map>
#include <vector>
#include "son_routing.h"

using namespace std;
using namespace Starsky;

SonRouting::SonRouting(int net_size, int friend_select_method){
  switch(friend_select_method){
    case 1:
      _son_friend = new SonNNFriendSelect(net_size);
      break;
    case 2:
      _son_friend = new SonRandomFriendSelect(net_size);
      break;
    default:
      break;
  }
  _routing_table =  new map<int, map<int, int>* >();
  _freq_map = _son_friend->GetFreqMap();
  _friends_map = _son_friend->GetFriendsMap();
  
  map<int, map<int, int>* >::iterator friend_it;
  for(friend_it = _friends_map->begin();friend_it != _friends_map->end();friend_it++){
    _routing_table->insert(pair<int, map<int, int>* >(friend_it->first, new map<int, int>()));
  }

  _fwd_route_tbl = new map<int, multimap<int, int>* >();
  _fwd_uniq_chk_tbl = new map<int, multimap<int, int>* >();
  _fwd_freq_tbl = new map<int, multimap<int, int>* >();
//  _max_routing_table_entry = 0x7fffffff;
  _max_routing_table_entry = 100;
}

SonRouting::SonRouting(SonFriendSelect* friends_net):_son_friend(friends_net){
  _routing_table =  new map<int, map<int, int>* >();
  _freq_map = _son_friend->GetFreqMap();
  _friends_map = _son_friend->GetFriendsMap();  
  map<int, map<int, int>* >::iterator friend_it;
  for(friend_it = _friends_map->begin();friend_it != _friends_map->end();friend_it++){
    _routing_table->insert(pair<int, map<int, int>* >(friend_it->first, new map<int, int>()));
  }  
//  _max_routing_table_entry = 0x7fffffff;
  _max_routing_table_entry = 100;
  _fwd_route_tbl = new map<int, multimap<int, int>* >();
  _fwd_uniq_chk_tbl = new map<int, multimap<int, int>* >();
  _fwd_freq_tbl = new map<int, multimap<int, int>* >();  
}

bool SonRouting::MultimapKeyValueExist(multimap<int,int>* input_mm, int key, int value){
  if (NULL == input_mm){
    return true;
  }
  multimap<int,int>::iterator it = input_mm->find(key);
  for(;it!=input_mm->end();it++){
    if(it->first == key){
      if(it->second == value){
        return true;
      }
    }else{
      return false;
    }
  }
  return false;
}
bool SonRouting::AddForwardingPath(int source, int target, int gateway){
  multimap<int,int>* src_fwd_route = _fwd_route_tbl->count(source) != 0 ? (*_fwd_route_tbl)[source]:new multimap<int, int>();
  multimap<int,int>* dst_fwd_route = _fwd_route_tbl->count(target) != 0 ? (*_fwd_route_tbl)[target]:new multimap<int, int>();
  multimap<int,int>* src_fwd_uniq = _fwd_uniq_chk_tbl->count(source) != 0 ? (*_fwd_uniq_chk_tbl)[source]:new multimap<int, int>();
  multimap<int,int>* dst_fwd_uniq = _fwd_uniq_chk_tbl->count(target) != 0 ? (*_fwd_uniq_chk_tbl)[target]:new multimap<int, int>();
  multimap<int,int>* gateway_fwd = _fwd_freq_tbl->count(gateway) != 0 ? (*_fwd_freq_tbl)[gateway]:new multimap<int, int>();
    
  if(false == MultimapKeyValueExist(src_fwd_route, target, gateway)){
    src_fwd_route->insert(pair<int,int>(target, gateway));
  }
  if(false == MultimapKeyValueExist(dst_fwd_route, source, gateway)){
    dst_fwd_route->insert(pair<int,int>(source, gateway));
  }
  if(false == MultimapKeyValueExist(src_fwd_uniq, gateway, target)){
    src_fwd_uniq->insert(pair<int,int>(gateway, target));
  }
  if(false == MultimapKeyValueExist(dst_fwd_uniq, gateway, source)){  
    dst_fwd_uniq->insert(pair<int,int>(gateway, source));
  }
  int prior = source > target ? target : source;
  int latter = source > target ? source : target;
  if(false == MultimapKeyValueExist(gateway_fwd, prior, latter)){  
    gateway_fwd->insert(pair<int,int>(prior, latter));
  }
  
  if( NULL != src_fwd_route){
    (*_fwd_route_tbl)[source] = src_fwd_route;
  }
  if(NULL!=dst_fwd_route){
    (*_fwd_route_tbl)[target] = dst_fwd_route;
  }
  if(NULL != src_fwd_uniq){
    (*_fwd_uniq_chk_tbl)[source] = src_fwd_uniq;
  }
  if (NULL != dst_fwd_uniq){
    (*_fwd_uniq_chk_tbl)[target] = dst_fwd_uniq;
  }
  if(NULL != gateway_fwd){
    (*_fwd_freq_tbl)[gateway] = gateway_fwd;  
  }
  return true;
}

bool SonRouting::DeleteMultimapEntry(multimap<int, int>* input_mm, int key, int value){
  if (NULL != input_mm){
    pair<multimap<int,int>::iterator, multimap<int,int>::iterator> range = input_mm->equal_range(key);  
    multimap<int,int>::iterator it;
    for(it=range.first;it!=range.second;++it){
      if(it->second == value){
        input_mm->erase(it);  
        break;
      }
    }  
  }
  return false;
}
bool SonRouting::DeleteForwardingPath(int source, int target, int gateway){
  multimap<int,int>* src_fwd_route = _fwd_route_tbl->count(source) != 0 ? (*_fwd_route_tbl)[source]:NULL;
  multimap<int,int>* dst_fwd_route = _fwd_route_tbl->count(target) != 0? (*_fwd_route_tbl)[target]:NULL;
  multimap<int,int>* src_fwd_uniq = _fwd_uniq_chk_tbl->count(source) != 0 ? (*_fwd_uniq_chk_tbl)[source] : NULL;
  multimap<int,int>* dst_fwd_uniq = _fwd_uniq_chk_tbl->count(target) != 0 ? (*_fwd_uniq_chk_tbl)[target]:NULL;
  multimap<int,int>* gateway_fwd = _fwd_freq_tbl->count(gateway) != 0 ? (*_fwd_freq_tbl)[gateway] : NULL;

  DeleteMultimapEntry(src_fwd_route, target, gateway);
  DeleteMultimapEntry(dst_fwd_route, source, gateway);
  DeleteMultimapEntry(src_fwd_uniq, gateway, target);
  DeleteMultimapEntry(dst_fwd_uniq, gateway, source);  

  int prior = source > target ? target : source;
  int latter = source > target ? source : target;  
  DeleteMultimapEntry(gateway_fwd, prior, latter);
  return true;
}

bool SonRouting::DeleteForwardingPath(int source, int target){
  multimap<int,int>* src_fwd_uniq = _fwd_uniq_chk_tbl->count(source) != 0 ?(*_fwd_uniq_chk_tbl)[source] : NULL;
  if(src_fwd_uniq == NULL){
    return true;
  }
  pair<multimap<int,int>::iterator,multimap<int,int>::iterator> range = src_fwd_uniq->equal_range(target);
  multimap<int,int>::iterator it;
  vector<int> delete_target;
  for(it=range.first;it!=range.second;++it){
    delete_target.push_back(it->second);
  }
  for(unsigned int i=0;i<delete_target.size();i++){    
    DeleteForwardingPath(source, delete_target[i], target);
  }
  return true;
}


void SonRouting::BuildRing(){
  map<int, map<int, int>* >::iterator friend_it;
  int predecessor = _friends_map->rbegin()->first;
  for(friend_it = _friends_map->begin();friend_it != _friends_map->end();friend_it++){
    if (CreateEdge(friend_it->first, predecessor, NEAR_CONN) == true){
      UpdateRemainConn(friend_it->first, predecessor, true);
    }
    predecessor = friend_it->first;
  }
}

bool SonRouting::Check(){
  map<int, map<int, int>* >::iterator friend_it;
  int predecessor = _friends_map->rbegin()->first;
  friend_it = _friends_map->begin();
  int begin_addr = friend_it->first;
  friend_it++;
  int next_addr = friend_it->first;
  int run_index = 0;
  cout << "predecessor = " << predecessor << endl;
  while(begin_addr != next_addr){
    run_index++;
    map<int, int>* routing_table = (*_routing_table)[next_addr];
    if(routing_table->size() != 2){
      cout << "routing table size is not 2" << endl;
      return false;
    }

    map<int, int>::iterator rt_it;
    
    if (next_addr == predecessor && routing_table->count(begin_addr) != 0){
      next_addr = begin_addr;
      for(rt_it = routing_table->begin();rt_it != routing_table->end();rt_it++){
        cout << rt_it->first << endl;
      }      
      break;
    }
    for(rt_it = routing_table->begin();rt_it != routing_table->end();rt_it++){
      if(rt_it->first > next_addr){
        next_addr = rt_it->first;
        break;
      }
    }
  }
  cout << "run index = " << run_index << " next_addr = " << next_addr << endl;  
  return true;
}

bool SonRouting::CreateEdge(int n1, int n2, int mode){
  if((_routing_table->count(n1) == 0) || (_routing_table->count(n2) == 0) || n1 == n2){
    return false;
  }
  map <int, int>* n1_table = (*_routing_table)[n1];
  map <int, int>* n2_table = (*_routing_table)[n2];
  if(n1_table == NULL || n2_table == NULL){
    return false;
  }
  if(n1_table->count(n2) != 0 || n2_table->count(n1)!= 0 || (mode != PO_NEAR && n1_table->size()>_max_routing_table_entry)||(mode != PO_NEAR && n2_table->size()>_max_routing_table_entry)){
    return false;
  }
  (*n1_table)[n2] = mode;
  (*n2_table)[n1] = mode;
  return true;
}

bool SonRouting::RemoveEdge(int n1, int n2){
  map <int, int>* n1_table = (*_routing_table)[n1];
  map <int, int>* n2_table = (*_routing_table)[n2];
  if(n1_table == NULL || n2_table == NULL){
    return false;
  }
  if(n1_table->count(n2) != 0){
    n1_table->erase(n2);
  }
  if(n2_table->count(n1) != 0){
    n2_table->erase(n1);
  }  
  return true;
}

void SonRouting::AddKConnections(int target_node_id){
  map<int, int>* self_friend_table = (*_friends_map)[target_node_id];
  map<int, int>* self_rtable = (*_routing_table)[target_node_id];
  multimap<int, int>* self_ftable = (*_freq_map)[target_node_id];
  if(NULL == self_friend_table || NULL == self_rtable || NULL == self_ftable){
    return;
  }
  multimap<int,int>::reverse_iterator sf_ri;
  for(sf_ri = self_ftable->rbegin();sf_ri != self_ftable->rend(); sf_ri++){
    if(self_rtable->count(sf_ri->second) != 0){
      continue;
    }
    map<int, int>* frtable = (*_routing_table)[sf_ri->second];
    if (NULL == frtable){
      continue;
    }
    if((*self_friend_table)[sf_ri->second] > 0) {
      if (CreateEdge(target_node_id, sf_ri->second, FRIENDS) == true){
        UpdateRemainConn(target_node_id, sf_ri->second, true);
        continue;
      }
    }
    map<int, int>* ff_map = (*_friends_map)[sf_ri->second];
    map<int, int>::iterator rt_it;
    for(rt_it = ff_map->begin();rt_it!=ff_map->end();rt_it++){
      if(self_friend_table->count(rt_it->first) != 0 && (*self_friend_table)[rt_it->first] > 0){
        if(CreateEdge(target_node_id, sf_ri->second, FRIENDS) == true){
          UpdateRemainConn(target_node_id, sf_ri->second, true);    
          break;
        }
      }
    }
  }
  
//    for(fr_it = self_friend_table->begin();fr_it != self_friend_table->end();fr_it++){
//      cout << "before trimming : " <<fr_it->first << " : " << fr_it->second << endl;
//    }
  DealWithNonKCovered(target_node_id);
//    for(fr_it = self_friend_table->begin();fr_it != self_friend_table->end();fr_it++){
//      cout << "after  trimming : " <<fr_it->first << " : " << fr_it->second << endl;
//    }    
//  delete self_ftable;
}

void SonRouting::DealWithNonKCovered(int host_addr){
  map<int, int>* self_friend_table = (*_friends_map)[host_addr];
  if(NULL == self_friend_table){
    return;
  }
  map<int, int>::iterator fr_it;
  for(fr_it = self_friend_table->begin();fr_it!=self_friend_table->end();fr_it++){
    if(fr_it->second > 0){
      int add_num = fr_it->second;
      multimap<int, int>* target_freq = (*_freq_map)[fr_it->first];
      if (NULL == target_freq){
        continue;
      }
      multimap<int, int>::reverse_iterator tf_it;
      for (tf_it = target_freq->rbegin();tf_it!=target_freq->rend() && add_num > 0 ;tf_it++){
        if(CreateEdge(host_addr, tf_it->second, NON_FRIENDS) == true){
          UpdateRemainConn(host_addr, tf_it->second, true);
          add_num--;
        }
      }
    }
  }
}

//trim unnecessary friend/non-friend connections
void SonRouting::TrimConnections(int host_addr){  
  map<int, int>* self_friend_table = (*_friends_map)[host_addr];
  map<int, int>* self_rtable = (*_routing_table)[host_addr];
  if(NULL == self_friend_table || NULL == self_rtable){
    return;
  }  
  map<int, int>::iterator fr_it;
  for(fr_it = self_friend_table->begin();fr_it != self_friend_table->end(); fr_it++){
    //trim unnecessary friend/non-friend connections
    if(fr_it->second > 0 || self_rtable->count(fr_it->first) == 0 || (self_rtable->count(fr_it->first) != 0 && ((*self_rtable)[fr_it->first] == SHORTCUT || (*self_rtable)[fr_it->first] == NEAR_CONN))){
      continue;
    }
    map<int, int>* ff_table = (*_friends_map)[fr_it->first];
    if(NULL != ff_table){
      map<int, int>::iterator self_ftable_it;
      bool remove = true;
      for(self_ftable_it = self_friend_table->begin();self_ftable_it !=self_friend_table->end();self_ftable_it++){
        if(ff_table->count(self_ftable_it->first) != 0 || ff_table->count(host_addr) != 0){
          if(self_ftable_it->second >= 0 || (ff_table->count(self_ftable_it->first) != 0 && (*ff_table)[self_ftable_it->first] >= 0)){
            remove = false;
            break;
          }
        }
      }        
      if(remove == true){
        
        if((fr_it->first==1064432087 &&host_addr ==834375185) || ((fr_it->first==834375185 && host_addr ==1064432087))){
          cout << "in a trim connection : " << __FILE__ << " : " <<__LINE__ << endl;
        }        
        RemoveEdge(host_addr, fr_it->first);
        UpdateRemainConn(host_addr, fr_it->first, false);      
      }
    }
  }
}

void SonRouting::UpdateRemainConn(int n1, int n2, bool deduct){
  map<int, int>*  n1_friends = (*_friends_map)[n1];
  map<int, int>*  n2_friends = (*_friends_map)[n2];
  int delta = deduct == true ? -1 : 1;
  if(NULL == n1_friends || NULL == n2_friends){
    return;
  }
  if(n1_friends->count(n2) != 0){
    int remain = (*n1_friends)[n2];
    (*n1_friends)[n2] = remain + delta;
    remain = (*n2_friends)[n1];
    (*n2_friends)[n1] = remain + delta;
  }
  map<int, int>::iterator ftable_it;
  for (ftable_it = n1_friends->begin();ftable_it!=n1_friends->end();ftable_it++){
    if(n2_friends->count(ftable_it->first) != 0){ // friends of friends
      int remain = ftable_it->second;
      (*n1_friends)[ftable_it->first] = remain + delta;
      remain = (*n2_friends)[ftable_it->first];
      (*n2_friends)[ftable_it->first] = remain + delta;      
    }
  }
}

map<int, map<int, int>*>* SonRouting::GetRoutingTable(){
  return _routing_table;
}

bool SonRouting::IsTrimable(int source, int target){
  multimap<int, int>* src_uniq_fwd = (*_fwd_uniq_chk_tbl)[source];
  multimap<int, int>* dst_uniq_fwd = (*_fwd_uniq_chk_tbl)[target];
  if(NULL != src_uniq_fwd && src_uniq_fwd->count(target) != 0){
    pair<multimap<int,int>::iterator,multimap<int,int>::iterator> range = src_uniq_fwd->equal_range(target);
    map<int,int>::iterator it;
    for(it=range.first;it!=range.second;++it){
      multimap<int, int>* src_fwd_route = (*_fwd_route_tbl)[source];
      if(src_fwd_route != NULL && src_fwd_route->count(it->second) < 2){  //should not trim
        return false;
      }
    }
  }
  if(dst_uniq_fwd != NULL && dst_uniq_fwd->count(source) != 0){
    pair<multimap<int,int>::iterator,multimap<int,int>::iterator> range = dst_uniq_fwd->equal_range(source);
    map<int,int>::iterator it;
    for(it=range.first;it!=range.second;++it){
      multimap<int, int>* tgt_fwd_route = (*_fwd_route_tbl)[target];
      if(tgt_fwd_route != NULL && tgt_fwd_route->count(it->second) < 2){  //should not trim
        return false;
      }
    }
  }

  return true;
}

map<int, multimap<int,int>* >* SonRouting::GetFwdRouteTable(){
  return _fwd_route_tbl;
}
void SonRouting::PrintRoutingTable(){
  int spacing = 10;
  map<int, map<int, int>* >::iterator rt_it;
  map<int, vector<int>* > routing_table_size_hist;
  unsigned long total_routing_table_entry = 0, near_conn_count = 0, shortcut_count = 0, friends_conn = 0, no_friends_conn=0, po_near_conn = 0;
  cout << "routing table size = " << _routing_table->size() << " friend map size = " << _friends_map->size() << endl;
  unsigned int max_friends = 0, non_kcovered = 0;
  for(rt_it = _routing_table->begin();rt_it != _routing_table->end();rt_it++){
    map<int, int>* fmap = (*_friends_map)[rt_it->first];
    if(NULL == fmap){
      continue;
    }
    max_friends = fmap->size() > max_friends ? fmap->size() : max_friends;
    int key = ((fmap->size()/spacing)+1)*spacing;
    vector<int>* num_edge_vector;
    if(routing_table_size_hist.count(key) == 0){
      num_edge_vector = new vector<int>();
    }else{
      num_edge_vector = routing_table_size_hist[key];
    }
    num_edge_vector->push_back(rt_it->second->size());
    routing_table_size_hist[key] = num_edge_vector;
    total_routing_table_entry += rt_it->second->size();
//    cout << "node : " << rt_it->first << "\t" << rt_it->second->size() << "\t" << fmap->size() << endl;
    map<int, int>::iterator fit;
//    if(fmap->size() < 50){
      for(fit=fmap->begin();fit!=fmap->end();fit++){
        if (fit->second > 0 && ((*_friends_map)[fit->first])->size() < 50){
//          cout << fit->first << " : " << fit->second << endl;
          non_kcovered++;
        }
      }
//    }
    map<int, int>::iterator each_rit;
    if(fmap->size() == 345 || fmap->size() == 438 || fmap->size() == 2162 || fmap->size() > 2000){
      int sn=0, nn=0, fn=0,nfn=0, ponc=0;
      for(each_rit = rt_it->second->begin();each_rit!=rt_it->second->end();each_rit++){
        if(each_rit->second == NEAR_CONN){
          nn++;
        }else if(each_rit->second == SHORTCUT){
          sn++;
        }else if(each_rit->second == FRIENDS){
          fn++;
        }else if(each_rit->second == NON_FRIENDS){
          nfn++;
        }else if(each_rit->second == PO_NEAR){
          ponc++;
        }
      }
      cout << fmap->size() << " max node id : " << rt_it->first <<" status : sn  "<< sn << " : nn " <<nn<<" : fn " <<fn<<" : nfn " <<nfn << " : ponc " << ponc << endl;
    }

    for(each_rit = rt_it->second->begin();each_rit!=rt_it->second->end();each_rit++){
      if(each_rit->second == NEAR_CONN){
        near_conn_count++;
      }else if(each_rit->second == SHORTCUT){
        shortcut_count++;
      }else if(each_rit->second == FRIENDS){
        friends_conn++;
      }else if(each_rit->second == NON_FRIENDS){
        no_friends_conn++;
      }else if(each_rit->second == PO_NEAR){
        po_near_conn++;
      }
    }
  }
  cout << "total routing table entry = " << total_routing_table_entry << " near connection = " << near_conn_count << " shortcut = " << shortcut_count << " friends = " << friends_conn << " no friends conn = " << no_friends_conn << "private overlay near conn = " << po_near_conn << endl;
  map<int, vector<int>* >::iterator ech_it;
  for(ech_it = routing_table_size_hist.begin();ech_it != routing_table_size_hist.end();ech_it++){
    unsigned long sum = 0;
    vector<int>* ec_vector = ech_it->second;
    for(unsigned int i=0;i<ec_vector->size();i++){
      sum += (*ec_vector)[i];
    }
    cout << ech_it->first << "\t" << ec_vector->size() << "\t" << (sum/ec_vector->size()) << endl;
  }
  cout << "max_friends : " << max_friends << endl;  
  cout << "non k-covered : " << non_kcovered << endl;
  cout << "total number of private overlay conn = " << _num_po_near_conn << " number of forward conn = " << _num_forwarding << endl;
}


SonPrivateOverlayRouting::SonPrivateOverlayRouting(int net_size, int friend_select_method):SonRouting(net_size, friend_select_method){
  _num_forwarding = 0;
  _threshhold = 100;
  _num_po_near_conn = 0;
  _shortcut_failed = 0;
}

SonPrivateOverlayRouting::SonPrivateOverlayRouting(SonFriendSelect* friends_net):SonRouting(friends_net){
  _threshhold = 100;
  _num_forwarding = 0;
  _num_po_near_conn = 0;
  _shortcut_failed = 0;
}

map<int, map<int, int>* >* SonPrivateOverlayRouting::BuildRoutingTable(){
  BuildRing();
  map<int, map<int, int>* >::iterator friend_it;
  for(friend_it = _friends_map->begin();friend_it != _friends_map->end();friend_it++){
    if(NULL == friend_it->second){
      continue;
    }
    if (friend_it->second->size() < _threshhold){
      CreateOneToOneConns(friend_it->first);
    }
  }
  map<int, map<int, int>* >::reverse_iterator friend_rit;
  for(friend_rit = _friends_map->rbegin();friend_rit != _friends_map->rend();friend_rit++){
    if(NULL == friend_rit->second){
      continue;
    }
    if (friend_rit->second->size() < _threshhold){
    }else{
      CreatePrivateOverlay(friend_rit->first);
    }
  }

for(friend_rit = _friends_map->rbegin();friend_rit != _friends_map->rend();friend_rit++){
  if(NULL == friend_rit->second){
    continue;
  }
  if (friend_rit->second->size() < _threshhold){
  }else{
    TrimPonConn(friend_rit->first);
  }
}
  
//  Check();
  PrintRoutingTable();
  cout << "shortcut failed : " << _shortcut_failed << endl;
  return NULL;
  for(friend_it = _friends_map->begin();friend_it != _friends_map->end();friend_it++){
    if(NULL == friend_it->second){
      continue;
    }
    if (friend_it->second->size() < _threshhold){
      SonRouting::TrimConnections(friend_it->first);
    }else{
      TrimConnections(friend_it->first);
    }
  }
  PrintRoutingTable();
  _num_forwarding = 0;
  _num_po_near_conn = 0;
  for(friend_it = _friends_map->begin();friend_it != _friends_map->end();friend_it++){
    if(NULL == friend_it->second){
      continue;
    }
    if (friend_it->second->size() < _threshhold){
      CreateOneToOneConns(friend_it->first);
    }else{
      CreatePrivateOverlay(friend_it->first);
    }
  }
  PrintRoutingTable();
  return NULL;
}

int SonPrivateOverlayRouting::GetGatewayNode(int source, int dst){
  map<int, int>* src_rt = _routing_table->count(source) == 0 ? NULL : (*_routing_table)[source];
  map<int, int>* dst_friend = _friends_map->count(dst) == 0 ? NULL : (*_friends_map)[dst];
  if (src_rt == NULL || dst_friend == NULL){
    return -1;
  }

  map<int, int>::iterator rt_it;
  for(rt_it = src_rt->begin();rt_it != src_rt->end();rt_it++){
    if(dst_friend->count(rt_it->first) != 0){
      return rt_it->first;
    }
  }
  return -1;
}

bool SonPrivateOverlayRouting::Check(){
  map<int, map<int, int>* >::iterator friend_it;
  for(friend_it = _friends_map->begin();friend_it != _friends_map->end();friend_it++){
    map<int, int>* self_rtable = (*_routing_table)[friend_it->first];
    unsigned int count_hit = 0;
    if(friend_it->second->size() < _threshhold){
      map<int, int>* ff_table = friend_it->second;
      map<int, int>::iterator ff_it;
      for(ff_it = ff_table->begin();ff_it != ff_table->end();ff_it++){
        if(_friends_map->count(ff_it->first) != 0 && ((*_friends_map)[ff_it->first])->size() < _threshhold){
          count_hit++;
          if(self_rtable->count(ff_it->first) == 0){
            cout << "a node that should be in here does not in a routing table" << endl;
            return false;
          }
        }
      }
    }
    if(self_rtable->size() < count_hit || (self_rtable->size() > (friend_it->second->size() + 2))){
      cout << "weird routing table size : routing table size = " << self_rtable->size()  << " count hit = " << count_hit << " friend number = " << friend_it->second->size() << endl;
      map<int,int>::iterator srt_it;
      cout << friend_it->first << endl;
      for(srt_it = self_rtable->begin();srt_it != self_rtable->end(); srt_it++){
        if(friend_it->second->count(srt_it->first) == 0){
          cout << srt_it->first << " : " << srt_it->second << endl;
        }
      }
      return false;
    }
  }  
  return true;
}
void SonPrivateOverlayRouting::CreateOneToOneConns(int host_id){
  map<int, int>::iterator sfl_it;
  map<int, int>* self_f_table = (*_friends_map)[host_id];  // hostid's friends list
  if (NULL == self_f_table){
    return;
  }
  for(sfl_it = self_f_table->begin();sfl_it != self_f_table->end();sfl_it++){
    if(_friends_map->count(sfl_it->first) != 0){
      map<int, int>* ff_table = (*_friends_map)[sfl_it->first];
      if(ff_table->size() < _threshhold){        
        CreateEdge(host_id, sfl_it->first, SHORTCUT);
      }
    }
  }
}

void SonPrivateOverlayRouting::CreatePrivateOverlay(int host_id){
  map<int, int>* self_f_table = (*_friends_map)[host_id];  // hostid's friends list
  if (NULL == self_f_table){
    return;
  }
  self_f_table->insert(pair<int, int>(host_id, 3));
  map<int, int>::iterator ft_it;
  int predecessor = self_f_table->rbegin()->first;
  for(ft_it = self_f_table->begin();ft_it != self_f_table->end(); ft_it++){
    _num_po_near_conn++;
    map<int, int>* fr_table = (*_routing_table)[ft_it->first];   //routing table of host id friends
    multimap<int, int>* f_freq_table = (*_freq_map)[ft_it->first];   //frequency map of host id friends
    if(NULL == fr_table || NULL == f_freq_table){
      continue;
    }    

    bool shortcut_added = false, shortcut_need = true;    
    multimap<int, int>::reverse_iterator frq_it; //frequency iterator of hostid's friens's
    for(frq_it = f_freq_table->rbegin();frq_it != f_freq_table->rend();frq_it++){  //check if we need to add a new shortcut connection
      shortcut_added = false;
      shortcut_need = true;
      if(self_f_table->count(frq_it->second) != 0 && fr_table->count(frq_it->second) != 0  && (*fr_table)[frq_it->second] != PO_NEAR){    
        shortcut_need = false;
        break;
      }
    }

    if(shortcut_need == true){
//      shortcut_added = LeastCommonScCreate(f_freq_table, self_f_table, fr_table, ft_it->first);
      shortcut_added = LeastNearConnScCreate(f_freq_table, self_f_table, fr_table, ft_it->first, predecessor);
      if(false == shortcut_added){
        _shortcut_failed++;
      }
    }

    if(fr_table->count(predecessor) == 0){    //successor and predecessor connection
      map<int, int>* predecessor_rt = (*_routing_table)[predecessor];
      map<int, int>::iterator prt_it;
      bool create_near_conn = true;
      if (predecessor_rt != NULL){
        if(predecessor_rt->count(ft_it->first) != 0){
          create_near_conn = false;
        }else{        
          if(predecessor_rt->size() > _max_routing_table_entry || fr_table->size() > _max_routing_table_entry){
            for(prt_it=predecessor_rt->begin();prt_it != predecessor_rt->end();prt_it++){
              if(fr_table->count(prt_it->first) != 0 && self_f_table->count(prt_it->first) != 0){  //there is a forward way
                AddForwardingPath(ft_it->first, predecessor, prt_it->first);
                create_near_conn = false;
              }
            }
          }
        }
      }
      if(create_near_conn == true){
        if (CreateEdge(ft_it->first, predecessor, PO_NEAR) == true){
          UpdateRemainConn(ft_it->first, predecessor, true);
        }
      }
    }
    predecessor = ft_it->first;
  }
  self_f_table->erase(host_id);
}

bool SonPrivateOverlayRouting::MostCommonScCreate(multimap<int, int>* f_freq_table, map<int, int>* host_node_ft, map<int, int>* friend_rt, int friend_id){
  multimap<int, int>::reverse_iterator frq_it;
  for(frq_it = f_freq_table->rbegin();frq_it != f_freq_table->rend();frq_it++){  //create one shortcut connection
    if(host_node_ft->count(frq_it->second) != 0 && friend_rt->count(frq_it->second) == 0){    
      if (CreateEdge(friend_id, frq_it->second, SHORTCUT) == true){
        UpdateRemainConn(friend_id, frq_it->second, true);
        return true;
      }
    }
  }
  return false;
}


bool SonPrivateOverlayRouting::LeastCommonScCreate(multimap<int, int>* f_freq_table, map<int, int>* host_node_ft, map<int, int>* friend_rt, int friend_id){
  multimap<int, int>::iterator frq_it;
  for(frq_it = f_freq_table->begin();frq_it != f_freq_table->end();frq_it++){  //create one shortcut connection
    if(host_node_ft->count(frq_it->second) != 0 && friend_rt->count(frq_it->second) == 0){    
        if(CreateEdge(friend_id, frq_it->second, SHORTCUT) == true){
          UpdateRemainConn(friend_id, frq_it->second, true);
          return true;
        }
    }
  }
  return false;
}

bool SonPrivateOverlayRouting::LeastNearConnScCreate(multimap<int, int>* f_freq_table, map<int, int>* host_node_ft, map<int, int>* fr_table, int friend_id, int predecessor){
  multimap<int, int>::iterator frq_it;
  for(frq_it= f_freq_table->begin();frq_it != f_freq_table->end();frq_it++){  //create one shortcut connection
    if(host_node_ft->count(frq_it->second) != 0){
        map<int, int>* cand_rt = (*_routing_table)[frq_it->second];
        if(cand_rt->count(predecessor) != 0 && fr_table->count(frq_it->second) == 0){
          if(true == CreateEdge(friend_id, frq_it->second, SHORTCUT)){
            UpdateRemainConn(friend_id, frq_it->second, true);
            return true;
          }
        }
    }
  }
  return false;
}

bool SonPrivateOverlayRouting::RandomShortcutCreate(multimap<int, int>* f_freq_table, map<int, int>* host_node_ft, map<int, int>* fr_table, int friend_id){
  bool shortcut_added = false;
  int loop_index = 0;
  multimap<int, int>::reverse_iterator frq_it;
  while(shortcut_added == false && loop_index < 10){
    frq_it= f_freq_table->rbegin(); //frequency iterator of hostid's friens's
    advance(frq_it, (rand() % f_freq_table->size()));
    if(host_node_ft->count(frq_it->second) != 0 && fr_table->count(frq_it->second) == 0){    
        if(true == CreateEdge(friend_id, frq_it->second, SHORTCUT)){
          UpdateRemainConn(friend_id, frq_it->second, true);
          return true;
        }
    }
    loop_index++;
  }
  return false;
}

void SonPrivateOverlayRouting::TrimConnections(int host_id){
  map<int, int>* self_f_table = (*_friends_map)[host_id];  // hostid's friends list
  if (NULL == self_f_table){
    return;
  }
  self_f_table->insert(pair<int, int>(host_id, 3));
  map<int, int>::iterator ft_it;
  int predecessor = self_f_table->rbegin()->first;
  for(ft_it = self_f_table->begin();ft_it != self_f_table->end(); ft_it++){
    map<int, int>* fr_table = (*_routing_table)[ft_it->first];   //routing table of host id friends
    multimap<int, int>* f_freq_table = (*_freq_map)[ft_it->first];   //frequency map of host id friends
    if(NULL == fr_table || NULL == f_freq_table){
      continue;
    }    
    if(fr_table->count(predecessor) != 0){    //successor and predecessor connection
      map<int, int>* predecessor_rt = (*_routing_table)[predecessor];
      map<int, int>::iterator prt_it;
      bool needed = true;
      for(prt_it=predecessor_rt->begin();prt_it != predecessor_rt->end();prt_it++){
        if(fr_table->count(prt_it->first) != 0 && self_f_table->count(prt_it->first) != 0){
          needed = false;
          break;
        }
      }
      if(needed == false){
        if((predecessor==1064432087 && ft_it->first ==834375185) || ((predecessor==834375185 && ft_it->first ==1064432087))){
          cout << "in a trim connection : " << __FILE__ << " : " <<__LINE__ << endl;
        }                
        RemoveEdge(ft_it->first, predecessor);
        UpdateRemainConn(ft_it->first, predecessor, false);
      }
    }
    predecessor = ft_it->first;
  }
  self_f_table->erase(host_id);  
}

bool SonPrivateOverlayRouting::TrimPonConn(int host_id){
  if(_routing_table->count(host_id) < 0 || _freq_map->count(host_id) < 0){
    return false;
  }
  map<int, int>* self_routing = (*_routing_table)[host_id];
  multimap<int, int>* self_freq_table = (*_freq_map)[host_id];
  multimap<int, int>::reverse_iterator freq_table_rit = self_freq_table->rbegin();
  while(self_routing->size() > 2*_max_routing_table_entry && freq_table_rit != self_freq_table->rend()){
    map<int, int>* cand_fmap = (*_friends_map)[freq_table_rit->second];
    map<int, int>* friend_rtable = (*_routing_table)[freq_table_rit->second];
    if(cand_fmap->size() < _threshhold || friend_rtable == NULL){
      freq_table_rit++;
      continue;
    }
    map<int, int>::iterator fmap_it = cand_fmap->find(host_id);
    map<int, int>::iterator fmap_it_p1= fmap_it++;
    int successor = (fmap_it_p1 != cand_fmap->end()) ? (++fmap_it)->first : cand_fmap->begin()->first;
    fmap_it = cand_fmap->find(host_id);
    int predecessor = (fmap_it != cand_fmap->begin()) ? (--fmap_it)->first : cand_fmap->rbegin()->first;
    if(self_routing->count(successor) != 0 && (*self_routing)[successor] == PO_NEAR && self_routing->count(predecessor) != 0 && (*self_routing)[predecessor] == PO_NEAR){
      if(IsTrimable(host_id, successor) == true){
        vector<int>* gw_cand_list = RouteExistAmongFriends(self_routing, cand_fmap, (*_routing_table)[successor]);
        if(gw_cand_list->size() != 0){
          RemoveEdge(host_id, successor);        
          DeleteForwardingPath(host_id, successor);          
          CreateEdge(successor, predecessor, PO_NEAR);
        }
        for(unsigned int i=0;i<gw_cand_list->size();i++){
          AddForwardingPath(host_id, successor, (*gw_cand_list)[i]);
        }
        delete gw_cand_list;
      }
      if(IsTrimable(host_id, predecessor) == true){
        vector<int>* gw_cand_list = RouteExistAmongFriends(self_routing, cand_fmap, (*_routing_table)[predecessor]);
        if(gw_cand_list->size() != 0){
          RemoveEdge(host_id, predecessor);            
          DeleteForwardingPath(host_id, predecessor);          
          CreateEdge(successor, predecessor, PO_NEAR);
        }
        for(unsigned int i=0;i<gw_cand_list->size();i++){
          AddForwardingPath(host_id, predecessor, (*gw_cand_list)[i]);
        }
        delete gw_cand_list;
      }      
    }
    freq_table_rit++;
  }
  return true;
}

vector<int>* SonPrivateOverlayRouting::RouteExistAmongFriends(map<int, int>* routing_table, map<int, int>* friends_list, map<int,int>* target_rt){
  map<int, int>::iterator sr_it;
  vector<int>* cand_list = new vector<int>();
  for(sr_it = routing_table->begin();sr_it != routing_table->end();sr_it++){
    if(target_rt->count(sr_it->first)!= 0 && friends_list->count(sr_it->first) != 0){
      cand_list->push_back(sr_it->first);
    }
  }
  return cand_list;
}

SonKCoverageRouting::SonKCoverageRouting(SonFriendSelect* friends_net):SonRouting(friends_net){
}

SonKCoverageRouting::SonKCoverageRouting(int net_size, int friend_select_method):SonRouting(net_size, friend_select_method){
}

map<int, map<int, int>* >* SonKCoverageRouting::BuildRoutingTable(){
  map<int, map<int, int>* >::iterator friend_it;
  for(friend_it = _friends_map->begin();friend_it != _friends_map->end();friend_it++){
    _routing_table->insert(pair<int, map<int, int>* >(friend_it->first, new map<int, int>()));
  }
  for(friend_it = _friends_map->begin();friend_it != _friends_map->end();friend_it++){
    AddKConnections(friend_it->first);
  }
  for(friend_it = _friends_map->begin();friend_it != _friends_map->end();friend_it++){
    TrimConnections(friend_it->first);
  }  
  return NULL;
}

SonClusterRouting::SonClusterRouting(int net_size, int friend_select_method):SonRouting(net_size, friend_select_method){
}

SonClusterRouting::SonClusterRouting(SonFriendSelect* friends_net):SonRouting(friends_net){
}

void SonClusterRouting::CheckNoCommonFriends(){
  map<int, map<int, int>* >::iterator friend_it;
  for(friend_it = _friends_map->begin();friend_it != _friends_map->end();friend_it++){
    CheckNoCommonFriends(friend_it->first);
  }  
}
map<int, map<int, int>* >* SonClusterRouting::BuildRoutingTable(){
//  CheckNoCommonFriends();
  map<int, map<int, int>* >::iterator friend_it;
  for(friend_it = _friends_map->begin();friend_it != _friends_map->end();friend_it++){
    CheckClusterCompleteness(friend_it->first);
  }
}
void SonClusterRouting::CheckNoCommonFriends(int host_id){
  multimap<int, int>* self_freq_map = (*_freq_map)[host_id];
  if (NULL == self_freq_map){
    return;
  }
  if(self_freq_map->size() < 50){
    return;
  }
  cout << "root node = " << host_id<< " number of friends = " <<self_freq_map->size() << endl;
  multimap<int, int>::iterator sfmap_it;
  for(sfmap_it = self_freq_map->begin();sfmap_it != self_freq_map->end();sfmap_it++){
    if(sfmap_it->first == 0){
      map<int, int>* ffmap = (*_friends_map)[sfmap_it->second];
      if (NULL == ffmap){
        continue;
      }
      cout << sfmap_it->second << " number of neighbor = " << ffmap->size() << endl;
    }else{
      break;
    }
  }
}

void SonClusterRouting::CheckClusterCompleteness(int host_id){
  map<int, int>* self_friend_map = (*_friends_map)[host_id];
  if (NULL == self_friend_map){
    return;
  }
  map<int, int> complete_covered_list;
  map<int, int>::iterator sfm_it;
  for(sfm_it = self_friend_map->begin(); sfm_it != self_friend_map->end(); sfm_it++){
    map<int, int>* ff_list = (*_friends_map)[sfm_it->first];
    if(NULL == ff_list){
      continue;
    }
    bool complete = true;
    vector<int>* cluster = CreateCluster(self_friend_map, ff_list);
    if (NULL == cluster){
      return ;
    }else if(cluster->size() == 0){
      delete cluster;
      return;
    }
    for(unsigned int i = 0; i < cluster->size();i++){
      map<int, int>* cf_map = (*_friends_map)[(*cluster)[i]];
      if (cf_map == NULL){
        complete = false;
        break;
      }else{
        for(unsigned int j=0;j<cluster->size();j++){
          if(cf_map->count((*cluster)[j]) == 0 && (i != j)){
            complete = false;
            break;
          }
        }
      }
      if (complete == false){
        break;
      }
    }
    if (complete == true){
      for(unsigned int i=0;i<cluster->size();i++){
        complete_covered_list[(*cluster)[i]] = 1;
      }
    }
    delete cluster;
  }
  if (self_friend_map->size() > 50)
  cout << host_id << " :  number of friends = " << self_friend_map->size() << " : complete covered = " << complete_covered_list.size() << endl;
}

vector<int>* SonClusterRouting::CreateCluster(map<int, int>* self_friend_map, map<int, int>* ff_list){
  map<int, int>* loop_list = ff_list->size() > self_friend_map->size()  ? self_friend_map : ff_list;
  map<int, int>* check_list = ff_list->size() > self_friend_map->size()  ? ff_list : self_friend_map;
  if (loop_list == NULL || check_list == NULL){
    return NULL;
  }
  map<int, int>::iterator check_it;
  vector<int>* cluster = new vector<int>();
  for(check_it = loop_list->begin();check_it != loop_list->end();check_it++){
    if(check_list->count(check_it->first) != 0){
      cluster->push_back(check_it->first);
    }
  }
  return cluster;
}

