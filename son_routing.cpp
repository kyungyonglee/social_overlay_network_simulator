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
#include <math.h>
#include "son_routing.h"
#include "son_util.h"

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
  _max_routing_table_entry = 100000;
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
  _max_routing_table_entry = 100000;
  _fwd_route_tbl = new map<int, multimap<int, int>* >();
  _fwd_uniq_chk_tbl = new map<int, multimap<int, int>* >();
  _fwd_freq_tbl = new map<int, multimap<int, int>* >();  
}

bool SonRouting::AddForwardingPath(int source, int target, int gateway){
  cout << "add forward path" << endl;
  multimap<int,int>* src_fwd_route = _fwd_route_tbl->count(source) != 0 ? (*_fwd_route_tbl)[source]:new multimap<int, int>();
  multimap<int,int>* dst_fwd_route = _fwd_route_tbl->count(target) != 0 ? (*_fwd_route_tbl)[target]:new multimap<int, int>();
  multimap<int,int>* src_fwd_uniq = _fwd_uniq_chk_tbl->count(source) != 0 ? (*_fwd_uniq_chk_tbl)[source]:new multimap<int, int>();
  multimap<int,int>* dst_fwd_uniq = _fwd_uniq_chk_tbl->count(target) != 0 ? (*_fwd_uniq_chk_tbl)[target]:new multimap<int, int>();
  multimap<int,int>* gateway_fwd = _fwd_freq_tbl->count(gateway) != 0 ? (*_fwd_freq_tbl)[gateway]:new multimap<int, int>();
    
  if(false == SonUtil::MultimapKeyValueExist(src_fwd_route, target, gateway)){
    src_fwd_route->insert(pair<int,int>(target, gateway));
  }
  if(false == SonUtil::MultimapKeyValueExist(dst_fwd_route, source, gateway)){
    dst_fwd_route->insert(pair<int,int>(source, gateway));
  }
  if(false == SonUtil::MultimapKeyValueExist(src_fwd_uniq, gateway, target)){
    src_fwd_uniq->insert(pair<int,int>(gateway, target));
  }
  if(false == SonUtil::MultimapKeyValueExist(dst_fwd_uniq, gateway, source)){  
    dst_fwd_uniq->insert(pair<int,int>(gateway, source));
  }
  int prior = source > target ? target : source;
  int latter = source > target ? source : target;
  if(false == SonUtil::MultimapKeyValueExist(gateway_fwd, prior, latter)){  
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

bool SonRouting::DeleteForwardingPath(int source, int target, int gateway){
  multimap<int,int>* src_fwd_route = _fwd_route_tbl->count(source) != 0 ? (*_fwd_route_tbl)[source]:NULL;
  multimap<int,int>* dst_fwd_route = _fwd_route_tbl->count(target) != 0? (*_fwd_route_tbl)[target]:NULL;
  multimap<int,int>* src_fwd_uniq = _fwd_uniq_chk_tbl->count(source) != 0 ? (*_fwd_uniq_chk_tbl)[source] : NULL;
  multimap<int,int>* dst_fwd_uniq = _fwd_uniq_chk_tbl->count(target) != 0 ? (*_fwd_uniq_chk_tbl)[target]:NULL;
  multimap<int,int>* gateway_fwd = _fwd_freq_tbl->count(gateway) != 0 ? (*_fwd_freq_tbl)[gateway] : NULL;

  SonUtil::DeleteMultimapEntry(src_fwd_route, target, gateway);
  SonUtil::DeleteMultimapEntry(dst_fwd_route, source, gateway);
  SonUtil::DeleteMultimapEntry(src_fwd_uniq, gateway, target);
  SonUtil::DeleteMultimapEntry(dst_fwd_uniq, gateway, source);  

  int prior = source > target ? target : source;
  int latter = source > target ? source : target;  
  SonUtil::DeleteMultimapEntry(gateway_fwd, prior, latter);
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
    CreateEdge(friend_it->first, predecessor, NEAR_CONN);
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

bool SonRouting::CreateEdge(int host, int target, int mode){
  if((_routing_table->count(host) == 0) || (_routing_table->count(target) == 0) || host == target){
    return false;
  }
  map <int, int>* n1_table = (*_routing_table)[host];
  map <int, int>* n2_table = (*_routing_table)[target];
  if(n1_table == NULL || n2_table == NULL){
    return false;
  }
  if(n1_table->count(target) != 0 || n2_table->count(host)!= 0 || (mode == SHORTCUT && n1_table->size()>(_max_routing_table_entry*1.5))||(mode == SHORTCUT && n2_table->size()>_max_routing_table_entry*1.5)){
    return false;
  }
  (*n1_table)[target] = mode;
  (*n2_table)[host] = mode;
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

map<int, map<int, int>*>* SonRouting::GetRoutingTable(){
  return _routing_table;
}

map<int, multimap<int,int>* >* SonRouting::GetFwdRouteTable(){
  return _fwd_route_tbl;
}

bool SonRouting::EdgeExist(int n1, int n2){
  if((_routing_table->count(n1) == 0) || (_routing_table->count(n2) == 0) || n1 == n2){
    return false;
  }
  map <int, int>* n1_table = (*_routing_table)[n1];
  map <int, int>* n2_table = (*_routing_table)[n2];
  if(n1_table == NULL || n2_table == NULL){
    return false;
  }
  if(n1_table->count(n2) != 0 && n2_table->count(n1)!= 0 ){
    return true;
  }
  return false;
}

void SonRouting::CalculateFwdOverhead(){
  map<int, multimap<int, int>* >::iterator fwd_it;
  map<int, SonStatistics*> fwd_overhead_map;
  for(fwd_it=_fwd_freq_tbl->begin();fwd_it!=_fwd_freq_tbl->end();fwd_it++){
    if (((*_friends_map)[fwd_it->first]) == NULL){
      continue;
    }
    int key = ((*_friends_map)[fwd_it->first])->size();
    SonStatistics* stat_map;
    if(fwd_overhead_map.count(key) == 0){
      stat_map = new SonStatistics(key);
      fwd_overhead_map[key] = stat_map;
    }else{
      stat_map = fwd_overhead_map[key];
    }
    stat_map->UpdateStat(fwd_it->second->size());
  }
  map <int,SonStatistics*>* summarized = SonUtil::SummarizeStat(&fwd_overhead_map);
  map<int, SonStatistics*>::iterator stat_it;
  cout << endl<< endl;
  for(stat_it=summarized->begin();stat_it!=summarized->end();stat_it++){
    stat_it->second->PrintStat();
  }
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
    cout << "node : " << rt_it->first << "\t" << rt_it->second->size() << "\t" << fmap->size() << endl;
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

void SonRouting::GetRoutingTableStat(){
  map<int, map<int,int>* >::iterator rt_it;
  map<int, SonStatistics*> routing_stat;
  SonStatistics* curr_stat;
  for(rt_it = _routing_table->begin();rt_it != _routing_table->end();rt_it++){
    int key = ((*_friends_map)[rt_it->first])->size();
    if(routing_stat.count(key) == 0){
      curr_stat = new SonStatistics(key);
      routing_stat[key] = curr_stat;
    }else{
      curr_stat = routing_stat[key];
    }
    curr_stat->UpdateStat(rt_it->second->size());
  }
  map<int, SonStatistics*>* summary = SonUtil::SummarizeStat(&routing_stat);
  map<int, SonStatistics*>::iterator sit;
  cout << endl << endl;
  for(sit = summary->begin();sit!=summary->end();sit++){
    sit->second->PrintStat();
  }
  delete summary;
}

void SonRouting::CreateShortcutConn(){
  int sc_number = (log(_friends_map->size()))/2, add_count=0, net_size=_friends_map->size(), exp=log10(RAND_MAX);
  map<int, map<int,int>* >::iterator fr_it;
  int weird_count=0, weird_selection=0;
  for(fr_it=_friends_map->begin();fr_it != _friends_map->end();fr_it++){
    add_count = 0;
    int host_id = fr_it->first, consecutive_fail = 0;
    map<int,int>* host_friends = fr_it->second;
    while(add_count < sc_number&& consecutive_fail < 20){
      float fr = (float)rand()/(float)RAND_MAX;
      int k = (int)(pow(10,(exp-(1-fr)*log10(net_size))));
      k &= 0x7fffffff;
      unsigned int temp = (unsigned int)host_id + (unsigned int)k;
      int shortcut_target_addr = temp%0x7fffffff;
      int actual_sc = SonUtil::GetClosestNode(_friends_map, shortcut_target_addr);
      if(CreateEdge(host_id, actual_sc, SHORTCUT) == false){
        consecutive_fail++;
      }else{
        add_count++;
        consecutive_fail = 0;
      }
    }
  }
}

SonPrivateOverlayRouting::SonPrivateOverlayRouting(int net_size, int friend_select_method):SonRouting(net_size, friend_select_method){
  _num_forwarding = 0;
  _threshhold = 100;
  _num_po_near_conn = 0;
  _shortcut_failed = 0;
  _joinable_po_map = new map<int, map<int, int>* >();
  _score_joinable_po_map = new map<int, multimap<int, int>* >();
  _po_create_nodes = new map<int,int>();
}

SonPrivateOverlayRouting::SonPrivateOverlayRouting(SonFriendSelect* friends_net):SonRouting(friends_net){
  _threshhold = 100;
  _num_forwarding = 0;
  _num_po_near_conn = 0;
  _shortcut_failed = 0;
  _joinable_po_map = new map<int, map<int, int>* >();
  _score_joinable_po_map = new map<int, multimap<int, int>* >();
  _po_create_nodes = new map<int,int>();
}

map<int, map<int, int>* >* SonPrivateOverlayRouting::BuildRoutingTable(){
 // GetLimitedPoStat();
//  return NULL;
  BuildRing();
  CreateShortcutConn();
  BuildJoinablePoMap();
//  PrintRoutingTable();
//  return NULL;
  map<int, map<int, int>* >::iterator friend_it;
  for(friend_it = _friends_map->begin();friend_it != _friends_map->end();friend_it++){
    if(NULL == friend_it->second) continue;
    if (_po_create_nodes->count(friend_it->first) == 0){
      CreateOneToOneConns(friend_it->first);
    }
  }
  map<int, map<int, int>* >::reverse_iterator friend_rit;
  for(friend_rit = _friends_map->rbegin();friend_rit != _friends_map->rend();friend_rit++){
    if(NULL == friend_rit->second) continue;
    if (_po_create_nodes->count(friend_rit->first) != 0){
      CreatePrivateOverlay(friend_rit->first);
    }
  }
/*  
  for(friend_rit = _friends_map->rbegin();friend_rit != _friends_map->rend();friend_rit++){
    if(NULL == friend_rit->second){
      continue;
    }
    if (_po_create_nodes->count(friend_it->first) != 0){
      TrimPonConn(friend_rit->first);
    }
  }
*/  
//  CalculateFwdOverhead();
//  GetPrivateOverlayJoinStat();
//  Check();
//  PrintRoutingTable();
  return _routing_table;
}

int SonPrivateOverlayRouting::GetPrivateOverlayJoinStat(){
  map<int, map<int,int>* >::iterator fit;
  map<int, SonStatistics*> poj_stat;
  for(fit = _friends_map->begin();fit != _friends_map->end();fit++){
    if(fit->second == NULL){
      continue;
    }
    int key = fit->second->size();
    SonStatistics* stat_map;
    if(poj_stat.count(key) == 0){
      stat_map = new SonStatistics(key);
      poj_stat[key] = stat_map;
    }else{
      stat_map = poj_stat[key];
    }
    map<int, int>::iterator foaf_it;
    int value = 0;
    for(foaf_it = fit->second->begin();foaf_it != fit->second->end();foaf_it++){
      if(_po_create_nodes->count(foaf_it->first) != 0){
//      if(((*_friends_map)[foaf_it->first]) != NULL && ((*_friends_map)[foaf_it->first])->size() > _threshhold){
        value++;
      }
    }
    stat_map->UpdateStat(value);    
  }  
  map <int,SonStatistics*>* summarized = SonUtil::SummarizeStat(&poj_stat);
  map<int, SonStatistics*>::iterator stat_it;
  cout << endl<< endl;
  for(stat_it=summarized->begin();stat_it!=summarized->end();stat_it++){
    stat_it->second->PrintStat();
  }
}

map<int, map<int, int>* >* SonPrivateOverlayRouting::BuildJoinablePoMap(){
  map<int, map<int,int>* >::iterator fit;
  int join_threshhold = 100;
  for(fit = _friends_map->begin();fit != _friends_map->end();fit++){
    if(fit->second == NULL || fit->second->size() < join_threshhold){
      continue;
    }
    map<int,int>* joinable_po_map = new map<int,int>();
    multimap<int,int>* freq_po_map = new multimap<int,int>();
    joinable_po_map->insert(pair<int,int>(fit->first, 0x7fffffff));
    freq_po_map->insert(pair<int,int>(0x7fffffff, fit->first));
    map<int,int>::iterator foaf_it;
    for(foaf_it = fit->second->begin();foaf_it != fit->second->end();foaf_it++){
      map<int,int>* foaf_ft = _friends_map->count(foaf_it->first) != 0 ? (*_friends_map)[foaf_it->first] : NULL;
      if(foaf_ft == NULL) continue;
      if(foaf_ft->size() >= _threshhold){
        joinable_po_map->insert(pair<int,int>(foaf_it->first, foaf_ft->size()));
        freq_po_map->insert(pair<int,int>(foaf_ft->size(), foaf_it->first));
        if(joinable_po_map->size() > join_threshhold){
          int remove_id = freq_po_map->begin()->second;
          freq_po_map->erase(freq_po_map->begin());
          joinable_po_map->erase(remove_id);
        }
      }
    }
    _joinable_po_map->insert(pair<int, map<int,int>* >(fit->first, joinable_po_map));
    _score_joinable_po_map->insert(pair<int, multimap<int,int>* >(fit->first, freq_po_map));
  }
  DeterminePoCreateNodes();
  
  return _joinable_po_map;
}

void SonPrivateOverlayRouting::GetLimitedPoStat(){
  map<int, SonStatistics*> limited_po_stat;
  int joinable_threshold = 100;
  map<int, SonStatistics*> per_entry_stat;
  map<int, map<int,int>* >::iterator fit;
  
  BuildJoinablePoMap();
  for(fit = _friends_map->begin();fit != _friends_map->end();fit++){
    if(fit->second == NULL || fit->second->size() < _threshhold){
      continue;
    }
    int no_join_count = 0;
    map<int,int>::iterator foaf_it;
    for(foaf_it = fit->second->begin();foaf_it != fit->second->end();foaf_it++){
      map<int,int>* foaf_ft = _friends_map->count(foaf_it->first) != 0 ? (*_friends_map)[foaf_it->first] : NULL;
      if(foaf_ft == NULL || foaf_ft->size() < _threshhold) continue;
      map<int,int>* fr_po_map = _joinable_po_map->count(foaf_it->first) != 0 ? (*_joinable_po_map)[foaf_it->first]:NULL;
      if(fr_po_map == NULL || fr_po_map->size() < joinable_threshold) continue;
      if(fr_po_map->count(fit->first) == 0){
        no_join_count++;
      }
    }
    int key = fit->second->size();
    SonStatistics* stat_map;
    if(per_entry_stat.count(key) == 0){
      stat_map = new SonStatistics(key);
      per_entry_stat[key] = stat_map;
    }else{
      stat_map = per_entry_stat[key];
    }
    stat_map->UpdateStat(no_join_count);
  }
  map <int,SonStatistics*>* summarized = SonUtil::SummarizeStat(&per_entry_stat);
  map<int, SonStatistics*>::iterator stat_it;
  cout << endl<< endl;
  for(stat_it=summarized->begin();stat_it!=summarized->end();stat_it++){
    stat_it->second->PrintStat();
  }  
}

map<int,int>* SonPrivateOverlayRouting::DeterminePoCreateNodes(){
  map<int, map<int,int>* >::iterator fit;
  int over_threshold = 0;
  for(fit = _friends_map->begin();fit != _friends_map->end();fit++){
    if(fit->second == NULL || fit->second->size() < _threshhold){
      continue;
    }else{
      over_threshold++;
    }
    map<int,int>::iterator foaf_it;
    int join_cand = 0;
    for(foaf_it=fit->second->begin();foaf_it != fit->second->end() && join_cand < _threshhold;foaf_it++){
      if(_friends_map->count(foaf_it->first) != 0){
        if(((*_friends_map)[foaf_it->first])->size() < _threshhold){
          join_cand++;
        }else if(_joinable_po_map->count(foaf_it->first) > 0){
          if(((*_joinable_po_map)[foaf_it->first])->count(fit->first) > 0){
            join_cand++;
          }
        }else{
          join_cand++;
        }
      }
    }
    
    if(join_cand >= _threshhold){
      _po_create_nodes->insert(pair<int,int>(fit->first, 1));
    }
  }
  cout << "overthreshold = " << over_threshold << " possible po creation = " << _po_create_nodes->size() << endl;
  return _po_create_nodes;
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
    if(_po_create_nodes->count(friend_it->first) == 0){
      map<int, int>* ff_table = friend_it->second;
      map<int, int>::iterator ff_it;
      for(ff_it = ff_table->begin();ff_it != ff_table->end();ff_it++){
        if(_po_create_nodes->count(ff_it->first) == 0){
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
  multimap<int, int>::reverse_iterator sfl_it;
  multimap<int, int>* self_freq_map = (*_freq_map)[host_id];  // hostid's friends list
  if (NULL == self_freq_map){
    return;
  }
  int add_count=0, target_add = 30;
  for(sfl_it = self_freq_map->rbegin();sfl_it != self_freq_map->rend() && add_count<target_add ;sfl_it++){
    if(_po_create_nodes->count(sfl_it->second) == 0){
      if (CreateEdge(host_id, sfl_it->second, DIRECT_FRIEND) == true){
        add_count++;
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
    map<int, int>* fr_table = (*_routing_table)[ft_it->first];   //routing table of host id friends
    multimap<int, int>* f_freq_table = (*_freq_map)[ft_it->first];   //frequency map of host id friends
    if(NULL == fr_table || NULL == f_freq_table){
      continue;
    }    

    if(f_freq_table->size() >= _threshhold && _joinable_po_map->count(ft_it->first) != 0){
      map<int,int>* join_po_map = (*_joinable_po_map)[ft_it->first];
      if(join_po_map->count(host_id) == 0){
        continue;
      }
    }
    _num_po_near_conn++;
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
//      shortcut_added = RandomShortcutCreate(f_freq_table, self_f_table, fr_table, ft_it->first);
//      shortcut_added = MostCommonScCreate(f_freq_table, self_f_table, fr_table, ft_it->first);
//      shortcut_added = LeastCommonScCreate(f_freq_table, self_f_table, fr_table, ft_it->first);
      shortcut_added = LeastNearConnScCreate(f_freq_table, self_f_table, fr_table, ft_it->first, predecessor);
      if(false == shortcut_added){
        _shortcut_failed++;
      }else{
        _shortcut_failed--;
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
          if(predecessor_rt->size() > _max_routing_table_entry || fr_table->size() > _max_routing_table_entry){ // either routing tables full
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
        CreateEdge(ft_it->first, predecessor, PO_NEAR);
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
        RemoveEdge(ft_it->first, predecessor);
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
  int trim_succeed = 0, trimmable = 0, run_count=0, check_count=0, good=0, no_good = 0;
  map<int, int>* self_routing = (*_routing_table)[host_id];
  multimap<int, int>* self_freq_table = (*_freq_map)[host_id];
  multimap<int, int>::reverse_iterator freq_table_rit = self_freq_table->rbegin();
  while(self_routing->size() > 2*_max_routing_table_entry && freq_table_rit != self_freq_table->rend()){
    map<int, int>* cand_fmap = (*_friends_map)[freq_table_rit->second];
    map<int, int>* friend_rtable = (*_routing_table)[freq_table_rit->second];
    if(_po_create_nodes->count(freq_table_rit->first) == 0|| friend_rtable == NULL){
      freq_table_rit++;
      continue;
    }
    run_count++;
    map<int, int>::iterator fmap_it = cand_fmap->find(host_id);
    map<int, int>::iterator fmap_it_p1= cand_fmap->find(host_id);
    fmap_it_p1++;
    int successor = (fmap_it_p1 != cand_fmap->end()) ? fmap_it_p1->first : cand_fmap->begin()->first;
    int predecessor = (fmap_it != cand_fmap->begin()) ? (--fmap_it)->first : cand_fmap->rbegin()->first;
    check_count++;
    bool succ_trimmable = false, succ_trimmed = false;
    if(IsTrimable(host_id, successor) == true){
      succ_trimmable = true;
      vector<int>* gw_cand_list = RouteExistAmongFriends(self_routing, cand_fmap, (*_routing_table)[successor]);
      if(gw_cand_list->size() != 0){
        RemoveEdge(host_id, successor);        
        DeleteForwardingPath(host_id, successor);          
        CreateEdge(successor, predecessor, PO_NEAR);
        succ_trimmed = true;
      }
      for(unsigned int i=0;i<gw_cand_list->size();i++){
        AddForwardingPath(host_id, successor, (*gw_cand_list)[i]);
      }
      delete gw_cand_list;
    }
    
    bool pred_trimmable = false, pred_trimmed = false;
    if(IsTrimable(host_id, predecessor) == true){
      pred_trimmable = true;
      vector<int>* gw_cand_list = RouteExistAmongFriends(self_routing, cand_fmap, (*_routing_table)[predecessor]);
      if(gw_cand_list->size() != 0){
        RemoveEdge(host_id, predecessor);            
        DeleteForwardingPath(host_id, predecessor);          
        CreateEdge(successor, predecessor, PO_NEAR);
        pred_trimmed = true;
      }
      for(unsigned int i=0;i<gw_cand_list->size();i++){
        AddForwardingPath(host_id, predecessor, (*gw_cand_list)[i]);
      }
      delete gw_cand_list;
    }      

    if(pred_trimmable == true && pred_trimmed == false && self_routing->count(predecessor)!= 0){
      map<int,int>* pred_rt = (*_routing_table)[predecessor];
      if (pred_rt->size() < 2*_max_routing_table_entry){
        map<int,int>::iterator srit;
        int min_table_size = 0x7fffffff, candidate = 0;
        for(srit = self_routing->begin();srit != self_routing->end();srit++){
          if(cand_fmap->count(srit->first) != 0 && ((*_routing_table)[srit->first])->size() < 2*_max_routing_table_entry){
            if((((*_routing_table)[srit->first])->size() < min_table_size) && srit->first != predecessor){
              min_table_size = ((*_routing_table)[srit->first])->size();
              candidate = srit->first;
            }
          }
        }
        if ((true == CreateEdge(predecessor,candidate,PO_NEAR)) && (EdgeExist(predecessor, candidate) == true)){            
          RemoveEdge(host_id, predecessor);            
          DeleteForwardingPath(host_id, predecessor);          
          AddForwardingPath(host_id, predecessor, candidate);
        }
      }
    }

    if(succ_trimmable == true && succ_trimmed == false && self_routing->count(successor)!= 0){
      map<int,int>* succ_rt = (*_routing_table)[successor];
      if (succ_rt->size() < 2*_max_routing_table_entry){
        map<int,int>::iterator srit;
        int min_table_size = 0x7fffffff, candidate = 0;
        for(srit = self_routing->begin();srit != self_routing->end();srit++){
          if(cand_fmap->count(srit->first) != 0 && ((*_routing_table)[srit->first])->size() < 2*_max_routing_table_entry){
            if((((*_routing_table)[srit->first])->size() < min_table_size) && (srit->first != successor)){
              min_table_size = ((*_routing_table)[srit->first])->size();
              candidate = srit->first;
            }
          }
        }
        if((true == CreateEdge(successor, candidate, PO_NEAR)) || (EdgeExist(successor, candidate) == true)){
          RemoveEdge(host_id, successor);            
          DeleteForwardingPath(host_id, successor);          
          AddForwardingPath(host_id, successor, candidate);
        }
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

bool SonPrivateOverlayRouting::IsTrimable(int source, int target){
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


map<int, map<int,int>* >* SonPrivateOverlayRouting::GetJoinablePoMap(){
  return _joinable_po_map;
}

map<int,int>* SonPrivateOverlayRouting::GetPoCreateNodes(){
  return _po_create_nodes;
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
//    TrimConnections(friend_it->first);
  }  
  return NULL;
}

void SonKCoverageRouting::AddKConnections(int target_node_id){
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
  
  DealWithNonKCovered(target_node_id);
}

void SonKCoverageRouting::DealWithNonKCovered(int host_addr){
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
      multimap<int, int>::iterator tf_it;
      for (tf_it = target_freq->begin();tf_it!=target_freq->end() && add_num > 0 ;tf_it++){
        if(CreateEdge(host_addr, tf_it->second, NON_FRIENDS) == true){
          UpdateRemainConn(host_addr, tf_it->second, true);
          add_num--;
        }
      }
    }
  }
}

void SonKCoverageRouting::UpdateRemainConn(int n1, int n2, bool deduct){
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

//trim unnecessary friend/non-friend connections
void SonKCoverageRouting::TrimConnections(int host_addr){  
  map<int, int>* self_friend_table = (*_friends_map)[host_addr];
  map<int, int>* self_rtable = (*_routing_table)[host_addr];
  if(NULL == self_friend_table || NULL == self_rtable){
    return;
  }  
  map<int, int>::iterator fr_it;
  for(fr_it = self_friend_table->begin();fr_it != self_friend_table->end(); fr_it++){
    //trim unnecessary friend/non-friend connections
    if(fr_it->second > 0 || self_rtable->count(fr_it->first) == 0){
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
        RemoveEdge(host_addr, fr_it->first);
        UpdateRemainConn(host_addr, fr_it->first, false);      
      }
    }
  }
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

