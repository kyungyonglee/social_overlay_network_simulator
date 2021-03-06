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
#include <algorithm>
#include <iostream>
#include <sstream>
#include <fstream>
#include <math.h>

#include "p2p_message.h"
#include "son_util.h"

using namespace Starsky;
using namespace std;

AttrValuePair::AttrValuePair(map<int,int>* node_list) : _node_list(node_list){
  _attr_value_pair = new map<int, map<int,int>* >();
  _attr_count_index = new map<int, map<int, double>* >();
  _attr_freq_count = new map<int, vector<int>* >();
  _value_node_map = new map<int, map<int, vector<int>* >* >();
  map<int,int>::iterator node_it;
  for(node_it = _node_list->begin();node_it!= _node_list->end();node_it++){
    map<int,int>* attr_val = new map<int,int>();
    (*_attr_value_pair)[node_it->first] = attr_val;
  }
}

void AttrValuePair::AddAttribute(int attr, string file_name){
  vector<int>* zipf_value = new vector<int>();
  vector<int>* attr_count = new vector<int>();
  map<int, vector<int>* >* value_nodes_map = new map<int, vector<int>* >();
  (*_value_node_map)[attr] = value_nodes_map;
  string line;
  ifstream file(file_name.c_str());
  int sum = 0;
  if(file.is_open()){    
    int index = 1, count = -1;
    map<int, int>* table;
    while(file.good()){
      getline(file, line);
      count = atoi(line.c_str());
      sum += count;
      attr_count->push_back(count);
      for(int i = 0;i<count;i++){
        zipf_value->push_back(index);
      }
      index++;      
    }
    file.close();
  }
  int zipf_size = zipf_value->size();
//  cout << "file name = " << file_name << " attr = " <<attr << " vector size = " << zipf_size << endl;
  map<int, map<int,int>* >::iterator all_it;
  for(all_it = _attr_value_pair->begin();all_it!=_attr_value_pair->end();all_it++){
    map<int,int>* attr_map = all_it->second;
    int value = (*zipf_value)[rand()%zipf_size];
    (*attr_map)[attr] = value;
    vector<int>* node_id_vector;
    if(value_nodes_map->count(value) != 0){
      node_id_vector = (*value_nodes_map)[value];
    }else{
      node_id_vector = new vector<int>();      
      (*value_nodes_map)[value] = node_id_vector;
    }
    node_id_vector->push_back(all_it->first);
  }
  (*_attr_freq_count)[attr] = zipf_value;
  map<int, double>* map_count_index = new map<int, double>();
  int prev_value = (*zipf_value)[0], local_count=0;
  double cum_sum = 0.0;
  for(int i = 0;i<zipf_value->size();i++){
    if((*zipf_value)[i] == prev_value){
      local_count++;
    }else{
      cum_sum += ((double)local_count/(double)sum);
      map_count_index->insert(pair<int,double>(prev_value,cum_sum));
      prev_value = (*zipf_value)[i];
      local_count = 1;
    }
  }
  cum_sum += ((double)local_count/(double)sum);
  map_count_index->insert(pair<int,double>(prev_value,cum_sum));  
  (*_attr_count_index)[attr] = map_count_index;  
}

map<int, map<int,int>* >* AttrValuePair::GetAttrValuePair(){
  return _attr_value_pair;
}

map<int, vector<int>* >* AttrValuePair::GetAttrFreq(){
  return _attr_freq_count;
}

map<int, map<int, double>* >* AttrValuePair::GetAttrCount(){
  return _attr_count_index;
}

map<int, map<int, vector<int>* >* >* AttrValuePair::GetValueNodeList(){
   return _value_node_map;
}


void AttrValuePair::CheckCumuMap(){
  map<int, map<int, double>* >::iterator cit;
  for(cit = _attr_count_index->begin();cit!=_attr_count_index->end();cit++){
    cout << "begin : " << cit->first << endl;
    map<int, double>::iterator cum_it;
    double prev_value = 0.0;
    for(cum_it = cit->second->begin();cum_it!=cit->second->end();cum_it++){
      if(cum_it->second <= prev_value){
        cout << "cumulative value smaller than prior " << cum_it->second << " : " <<prev_value << endl;       
      }
      cout << cum_it->second << endl;
      prev_value = cum_it->second;
    }
  }
}

map<int, map<int, multimap<int,int>*>*>* AttrValuePair::UpdateToSuperpeers(map<int,int>* superpeer_map){
  map<int, map<int, multimap<int,int>*>*>* superpeer_assign = new map<int, map<int, multimap<int,int>*>*>();  
  map<int,int>::iterator spm_it;   //host_node/superpeer
  for(spm_it=superpeer_map->begin();spm_it!=superpeer_map->end();spm_it++){
    map<int, multimap<int,int>*>* sp_cache;  //attribute/value/host_node
    if(superpeer_assign->count(spm_it->second) != 0){
      sp_cache = (*superpeer_assign)[spm_it->second];
    }else{
      sp_cache = new map<int, multimap<int,int>*>();
      (*superpeer_assign)[spm_it->second] = sp_cache;
    }
    map<int,int>* av_pair = (*_attr_value_pair)[spm_it->first];  //attribute/value
    map<int,int>::iterator avp_it;
    for(avp_it = av_pair->begin();avp_it!=av_pair->end();avp_it++){
      multimap<int,int>* value_node;  //value/host_node
      if(sp_cache->count(avp_it->first)!=0){
        value_node = (*sp_cache)[avp_it->first];
      }else{
        value_node = new multimap<int,int>();
        (*sp_cache)[avp_it->first] = value_node;
      }
      value_node->insert(pair<int,int>(avp_it->second, spm_it->first));
    }
  }
  return superpeer_assign;
}

int AttrValuePair::GenerateNewValue(int attribute){
  vector<int>* zipf_value = (*_attr_freq_count)[attribute];
  return (*zipf_value)[rand()%zipf_value->size()];
}


DynamicAttribute::DynamicAttribute(int mean_attr_change_time, int update_period):_mean_attr_dynamic_time(mean_attr_change_time), _attr_update_period(update_period){
  _poisson_dist = std::tr1::poisson_distribution<int>(_mean_attr_dynamic_time);
  ChangeUpdatePeriod(update_period);
//  _rand_eng.seed(time(NULL));
}
void DynamicAttribute::ChangeUpdatePeriod(int new_update_period){
  if(new_update_period <= 0){
    _attr_update_period = 1;
  }else{
    _attr_update_period = new_update_period;
  }
}
bool DynamicAttribute::CheckIfChanged(int propagation_time){  
  int change_cycle = _poisson_dist(_rand_eng);
//  cout << change_cycle << endl;
  int age = rand()%_attr_update_period;
  return (age+propagation_time > change_cycle ? true:false);
}

P2PResInfoDhtUpdate::P2PResInfoDhtUpdate(map<int,int>* node_list): AttrValuePair(node_list){
  _dht_res_info = new map<int, map<int, vector<int>*>* >();
}

P2PResInfoDhtUpdate::~P2PResInfoDhtUpdate(){
  map<int, map<int, vector<int>*>* >::iterator dri_it;
  for(dri_it = _dht_res_info->begin();dri_it!=_dht_res_info->end();dri_it++){
    map<int, vector<int>*>* attr_assignments = dri_it->second;
    if(attr_assignments != NULL){
      map<int, vector<int>*>::iterator aa_it;
      for(aa_it = attr_assignments->begin();aa_it!=attr_assignments->end();aa_it++){
        if(aa_it->second != NULL){
          delete aa_it->second;
        }
      }
      delete attr_assignments;
    }
  }
  delete _dht_res_info;
}

void P2PResInfoDhtUpdate::UpdateDht(){
  map<int, map<int,int>* >::iterator navp_it;
  for(navp_it=_attr_value_pair->begin();navp_it!=_attr_value_pair->end();navp_it++){
    map<int,int>* avp = navp_it->second;
    map<int,int>::iterator avp_it;
    for(avp_it=avp->begin();avp_it!=avp->end();avp_it++){
      int host_node = DetermineNode(avp_it->first, avp_it->second);
      map<int, vector<int>* >* value_node;
      if(_dht_res_info->count(host_node)!=0){
        value_node = (*_dht_res_info)[host_node];
      }else{
        value_node = new map<int, vector<int>* >();
      }
      vector<int>* nodes;
      if(value_node->count(avp_it->first) != 0){
        nodes = (*value_node)[avp_it->first];
      }else{
        nodes = new vector<int>();
      }
      nodes->push_back(navp_it->first);
      (*value_node)[avp_it->first] = nodes;
      (*_dht_res_info)[host_node] = value_node;
    }
  }
}

void P2PResInfoDhtUpdate::CheckDhtValidity(){
  map<int, map<int, vector<int>* >* >::iterator dht_it;
  for(dht_it = _dht_res_info->begin();dht_it!=_dht_res_info->end();dht_it++){
    map<int, vector<int>* >::iterator vn_it;
    for(vn_it = dht_it->second->begin();vn_it!=dht_it->second->end();vn_it++){
      vector<int>* nodes = vn_it->second;
//      cout << "reaponsible size = " << nodes->size() << endl;
      for(int i=0;i<nodes->size();i++){
        map<int,int>* avp = (*_attr_value_pair)[((*nodes)[i])];
        int value = (*avp)[vn_it->first];
        map<int, double>* attr_dist = (*_attr_count_index)[vn_it->first];
        map<int, double>::iterator addr_it = attr_dist->find(value);  
//        cout << dht_it->first << " : " << addr_it->first << " : " << addr_it->second << endl;
        int begin_addr;
        int end_addr = (int)((addr_it->second)*RAND_MAX);
        if(addr_it == attr_dist->begin()){
          begin_addr = 0;
        }else{
          addr_it--;
          begin_addr = (int)((addr_it->second)*RAND_MAX);
        }
        if(dht_it->first < begin_addr || dht_it->first > end_addr){
          map<int,int>::iterator cand_it = _node_list->lower_bound(begin_addr);
          cand_it--;
          if(cand_it->first != dht_it->first){               
            cout << dht_it->first << " : " << addr_it->first << " : " << addr_it->second << endl;
            cout << "A wrong dht assignment " << dht_it->first << " begin = " << begin_addr << " end = =" << end_addr << endl;
            return;
          }
        }
      }
    }
  }
  cout << "All entries are assigned corrrectly" << endl;
}

int P2PResInfoDhtUpdate::DetermineNode(int attribute, int value){
  map<int, double>* attr_dist = (*_attr_count_index)[attribute];
  map<int, double>::iterator addr_it = attr_dist->find(value);
  int begin_addr, end_addr;  
  end_addr = (int)((addr_it->second)*RAND_MAX);
  if(addr_it == attr_dist->begin()){
    begin_addr = 0;
  }else{
    addr_it--;
    begin_addr = (int)((addr_it->second)*RAND_MAX);
  }
  int target_addr = (end_addr-begin_addr==0) ? end_addr : begin_addr + rand()%(end_addr-begin_addr);
  map<int,int>::iterator end_it = _node_list->upper_bound(end_addr);
  map<int,int>::iterator ta_it = _node_list->lower_bound(target_addr);
  if(ta_it == end_it){
    ta_it--;
  }
  return ta_it->first;
}

map<int, map<int, vector<int>*>* >* P2PResInfoDhtUpdate::GetDhtResInfo(){
  return _dht_res_info;
}

//P2PMessageDist::P2PMessageDist(map<int, map<int,int>* >* routingt_table) : _global_rt(routingt_table){
//  _node_failures = P2PNodeFailure(NULL, 0);
//}

P2PMessageDist::P2PMessageDist(map<int, map<int,int>* >* routingt_table, P2PNodeFailure* node_failure) :_global_rt(routingt_table), _node_failures(node_failure){
}

int P2PMessageDist::P2PTreeMulticast(int source, int begin_addr, int end_addr, P2PAction* actions, bool clockwise){
  bool within_range = SonUtil::WithinRange(source, begin_addr, end_addr);
  map<int,int>* host_rt = (*_global_rt)[source];
  int max_depth = 0;
  if(_node_failures->CheckIfFailed(source) == true){
    return 0;
  }
  if(within_range){
    host_rt->insert(pair<int,int>(source,1));
    map<int, map<int, int>* >* region_alloc = AllocateRegions(host_rt, begin_addr, end_addr, source, clockwise);
    host_rt->erase(source);
    map<int, map<int, int>* >::iterator ra_it;
    for(ra_it = region_alloc->begin();ra_it!=region_alloc->end();ra_it++){
      map<int,int>::iterator each_alloc_it = ra_it->second->begin();
      int cur_depth = P2PTreeMulticast(ra_it->first, each_alloc_it->first, each_alloc_it->second, actions, clockwise);
      delete ra_it->second;
      max_depth = max_depth < cur_depth ? cur_depth : max_depth;
    }
    delete region_alloc;
    actions->Execute(source);
  }else{
    map<int,int>::iterator rt_it;
    int next_node = -1;
    for(rt_it=host_rt->begin();rt_it!=host_rt->end();rt_it++){
      if(SonUtil::WithinRange(rt_it->first, begin_addr, end_addr) == true){
        next_node = rt_it->first;
        break;
      }
    }
    if(next_node < 0){
      next_node = SonUtil::GetClosestNode(source, host_rt, begin_addr);
    }
    if(next_node >= 0){
      int cur_depth = P2PTreeMulticast(next_node, begin_addr, end_addr, actions, clockwise);
      max_depth = max_depth < cur_depth ? cur_depth : max_depth;
    }
  }
  return (max_depth+1);
}

map<int, map<int, int>* >* P2PMessageDist::RecursiveTreeAssign(map<int,int>* host_rt, int source, int begin_addr, int end_addr, bool clockwise){
  bool within_range = SonUtil::WithinRange(source, begin_addr, end_addr);
  if(within_range){
    host_rt->insert(pair<int,int>(source,1));
    map<int, map<int, int>* >* region_alloc = AllocateRegions(host_rt, begin_addr, end_addr, source, clockwise);
    host_rt->erase(source);
    return region_alloc;  //should be freed for out and in memory, host_id/begin_addr/end_addr
  }else{
    return NULL;
  }
}

int P2PMessageDist::P2PGreedyRouting(int source, int target, P2PAction* actions){
  if(source == target){
    if (actions != NULL){
      actions->Execute(target);
    }
    return 0;
  }
  int hops=0, next_node = SonUtil::GetClosestNode(source, (*_global_rt)[source], target);
  if(next_node > 0){
    hops = P2PGreedyRouting(next_node ,target, actions);
  }
  return hops+1;
}

int P2PMessageDist::P2PSequentialCrawling(int host_id, int begin_addr, int end_addr, P2PAction* actions){
  map<int,map<int,int>* >::iterator start_it = _global_rt->upper_bound(begin_addr);
  start_it--;
  if(begin_addr == 0){
    start_it = _global_rt->begin();
  }
  int visited_nodes = P2PGreedyRouting(host_id,start_it->first, NULL);
  while(start_it->first < end_addr && start_it!=_global_rt->end()){
    if(_node_failures->CheckIfFailed(start_it->first) == true){
      return visited_nodes;
    }    
    visited_nodes++;
    if(actions != NULL){
      if (true == actions->Execute(start_it->first)){   //Execute return true means we have to stop sequential crawling.
        visited_nodes += P2PGreedyRouting(start_it->first, host_id, NULL);
        return visited_nodes;
      }
    }
    start_it++;
  }
  return visited_nodes;
}

int P2PMessageDist::P2PFlooding(int src_id, int host_id, int ttl, map<int,int>* visited_nodes, P2PAction* actions){  //src=message source, host=msg receiver
  bool matching, renew_ttl = false, prev_ttl_is_zero=false;  
  if(_node_failures->CheckIfFailed(host_id) == true){
    return 0;
  }
  
  if(visited_nodes != NULL){
    if(visited_nodes->count(host_id) != 0){
      if(((*visited_nodes)[host_id]) < ttl ){
        if((*visited_nodes)[host_id] == 0){
          prev_ttl_is_zero = true;
        }
        (*visited_nodes)[host_id] = ttl;
        renew_ttl = true;
      }else{
//        cout << "already gotten message src=" << src_id << " host id = " << host_id <<endl;
        return 0;
      }
    }else{
      (*visited_nodes)[host_id] = ttl;
    }
  }
  if(actions != NULL && renew_ttl == false){
    matching = actions->Execute(host_id);  //Execute return true : terminate flooding 
  }  
  if(matching == true){
    return 0;
  }
  
  map<int,int>* host_rt = (*_global_rt)[host_id];
  map<int,int>::iterator hrt_it;
  int total_messages = 0;
  if(ttl > 0){
    for(hrt_it=host_rt->begin();hrt_it!=host_rt->end();hrt_it++){
      if(hrt_it->first != src_id){
        int local_msg_count = 0;
//        cout << "before sending to chile node : src = "<< src_id << " host id = " << host_id << " total message = " << total_messages << endl;
        local_msg_count = P2PFlooding(host_id, hrt_it->first, ttl-1, visited_nodes, actions);
//        cout << "return from child nodes: src = " << src_id << " host id = " << host_id << " total message = " <<local_msg_count << endl;
        total_messages += local_msg_count;
        if(renew_ttl == false || prev_ttl_is_zero==true){
          total_messages++;
        }
      }
    }
  }
//  cout << "return to parent node src = " << src_id << " host id = " << host_id << " total message = "<< total_messages << endl;
  return total_messages;
}

map<int, map<int, int>* >* P2PMessageDist::AllocateRegions(map<int,int>* routing_table, int begin_addr, int end_addr, int exclude, bool clockwise){
  map<int,int> candidates;
  map<int,int>::iterator rt_it;
  for(rt_it = routing_table->begin();rt_it != routing_table->end();rt_it++){
    if(SonUtil::WithinRange(rt_it->first, begin_addr, end_addr) == true){
      candidates[SonUtil::GetClockwiseDistance(begin_addr, rt_it->first)] = rt_it->first;
    }
  }
  map<int, map<int,int>* >* allocation = new map<int, map<int,int>* >();
  map<int,int>::iterator next_node_it = candidates.begin();
  int ba = begin_addr, ea = end_addr;
  for(rt_it = candidates.begin();rt_it!=candidates.end();rt_it++){
    next_node_it++;
    if(rt_it->second == exclude){
      ba = rt_it->second+1;
      continue;
    }
    map<int,int>* each_alloc = new map<int,int>();
    if(next_node_it != candidates.end()){
      if(clockwise){
        ea = next_node_it->second -1;
      }else{
        ea = rt_it->second;
      }
    }else{   
      ea = end_addr;
    }
    (*each_alloc)[ba] = ea;
    (*allocation)[rt_it->second] = each_alloc;
    ba = ea+1;
  }
  return allocation;
}

P2PRdQuery::P2PRdQuery(AttrValuePair* attribute_values, int mode, bool clockwise, int max_target_num) : Mode(mode), AttributeValues(attribute_values), Clockwise(clockwise){
  map<int, vector<int>* >* attr_freq_count = AttributeValues->GetAttrFreq();
  Attribute = rand()%attr_freq_count->size();
  vector<int>* attr_freq = (*attr_freq_count)[Attribute];
  int t1 = (*attr_freq)[rand()%attr_freq->size()];
  int t2 = (*attr_freq)[rand()%attr_freq->size()];
  Begin = t1 < t2 ? t1 : t2;
  End = t1 < t2 ? t2: t1;
  DistanceFromRoot = 0;
  Number = (rand() % max_target_num);
//  Number = (rand() % (max_target_num/2));
//  Number += (max_target_num/2);
  Number = Number == 0 ? 1 : Number;
  Result = new map<int,int>();
  NumQueriedNodes = 0;
  Initialize();
  if(mode == OPTIMAL || mode == FIRST_FIT){
    AddrBegin = 0;
    AddrEnd = 0x7fffffff;
  }else if(mode == DHT){
    DetermineDhtQueryRange();
  }
  MaxTargetNum = max_target_num;  
}

P2PRdQuery::P2PRdQuery(){
  Result = new map<int,int>();
  CurHops = 0;
  DistanceFromRoot = 0;
  TotalMsgs = 0;
  NumQueriedNodes = 0;
  Initialize();
}

P2PRdQuery::~P2PRdQuery(){
  delete Result;
}

bool P2PRdQuery::CheckResultCorrectness(P2PNodeFailure* failed_nodes, map<int, map<int, ResDiscResult*>* >& stat){
  int fail_node_count = 0, stat_key;
//  bool stat_fill = stat.size() != 0 ? true : false;
  bool stat_fill = true;
  int real_node_count = 0;
  if(Result->size() >= Number && stat_fill == false){
//    cout << "discovered all required nodes : "<<endl;
  }else{
    int total_nodes_num = AttributeValues->GetAttrValuePair()->size();
    real_node_count = GetRealSatisfyingNodes(failed_nodes);
    if((Result->size() != real_node_count) && (Result->size() < Number)){
//      double queried_region = AddrEnd > AddrBegin ? (AddrEnd-AddrBegin)/(double)RAND_MAX : ((RAND_MAX-AddrBegin) + AddrEnd)/(double)RAND_MAX;
//      cout << "Query no complete result size: " << Result->size() << " target number = " << Number<< " Real satisfying: " << real_node_count << " attribute = " <<Attribute << " begin = " << Begin << " end = " << End << " Begion addr = " << AddrBegin << " End addr = " <<AddrEnd << "queried region = " << queried_region <<endl;
    }
    stat_key = ceil((real_node_count/(double)total_nodes_num)*10) *10;
  }

  map<int,int>::iterator result_it;
  map<int, map<int,int>* >* av_pair = AttributeValues->GetAttrValuePair();
  long total_result_age = 0;
  for(result_it=Result->begin();result_it!=Result->end();result_it++){
    total_result_age += result_it->second;
    fail_node_count = failed_nodes->CheckIfFailed(result_it->first) == true ? fail_node_count+1 : fail_node_count;
    map<int,int>* host_av = (*av_pair)[result_it->first];
    if((*host_av)[Attribute] < Begin || (*host_av)[Attribute] >End){
      cout << "incorrect result returned : " <<result_it->first << " Attribute = " << Attribute << " Value = " <<(*host_av)[Attribute]<< " Begin: " << Begin << " End: " << End<<endl;
    }
  }
/*  
  if(fail_node_count != 0){
    cout << "the query result is not up-to-date with stale result  = " << fail_node_count << endl;
    if((Result->size() >= Number) && ((Result->size() - fail_node_count) < Number)){
      cout << "The query is not satisfied due to the stale informaion" << endl;
    }
  }
*/  
  long avg_res_age = Result->size() != 0 ? total_result_age/Result->size() : 0;
  if (stat_key == 0){
    return false;
  }
  if(stat_fill == true){
    map<int, ResDiscResult*>* local_stat;
    if(stat.count(stat_key) != 0){
      local_stat = stat[stat_key];
    }else{
      local_stat = new map<int, ResDiscResult*>();
      stat[stat_key] = local_stat;
    }
    
    ResDiscResult* entry;
    int target_num_key = Number/(MaxTargetNum/3);
    if(local_stat->count(target_num_key) != 0){
      entry = (*local_stat)[target_num_key];
    }else{
      entry = new ResDiscResult();
      (*local_stat)[target_num_key] = entry;
    }
    
    entry->Count = entry->Count + 1;
    entry->Hops = entry->Hops + CurHops;
    entry->MaxHops = entry->MaxHops > CurHops ? entry->MaxHops : CurHops;
    entry->MinHops = entry->MinHops < CurHops ? entry->MinHops : CurHops;
    entry->TotalMessages = entry->TotalMessages + TotalMsgs;
    entry->MaxMessages = entry->MaxMessages > TotalMsgs ? entry->MaxMessages : TotalMsgs;
    entry->MinMessages = entry->MinMessages < TotalMsgs ? entry->MinMessages : TotalMsgs;
    int temp = min(Number,(int)(Result->size() - fail_node_count));
    double incompleteness =  (real_node_count - (Result->size()-fail_node_count))*(Number - temp)/ (double)(real_node_count*Number);
    double completeness = max(temp/(double)real_node_count, temp/(double)Number);
    entry->InCompleteness = entry->InCompleteness + incompleteness;
    entry->Completeness = entry->Completeness + completeness;
    entry->AvgResultAge = entry->AvgResultAge + avg_res_age;
    if(Result->size() != 0){
      entry->FalseResult = entry->FalseResult + (fail_node_count/(double)Result->size());
    }
    entry->TotalQueriedNodes = entry->TotalQueriedNodes + NumQueriedNodes;
  }
  return true;
}

bool P2PRdQuery::CheckResultCorrectness(P2PNodeFailure* failed_nodes){
  map<int, map<int, ResDiscResult*>* > result;
  CheckResultCorrectness(failed_nodes, result);
}

void P2PRdQuery::DetermineDhtQueryRange(){
  map<int, map<int, double>* >*  attr_count = AttributeValues->GetAttrCount();
  map<int, double>* attr_dist = (*attr_count)[Attribute];
  map<int, double>::iterator addr_it = attr_dist->find(Begin);
  if(addr_it == attr_dist->begin()){
    AddrBegin = 0;
  }else{
    addr_it--;
    AddrBegin = (int)((addr_it->second)*RAND_MAX);
  }
  addr_it = attr_dist->find(End);
  AddrEnd = (int)((addr_it->second)*RAND_MAX);
}

void P2PRdQuery::DetermineAllQueryRange(){
  AddrBegin = 0;
  AddrEnd = RAND_MAX;
}

void P2PRdQuery::DetermineSubRegionQuery(double fraction, int host_id){
  map<int, map<int, double>* >*  attr_count = AttributeValues->GetAttrCount();
  map<int, double>* attr_dist = (*attr_count)[Attribute];
  map<int, double>::iterator begin_addr_it = attr_dist->find(Begin);
  map<int, double>::iterator end_addr_it = attr_dist->find(End);  
  double begin_cum = begin_addr_it == attr_dist->begin() ? 0.0 : (--begin_addr_it)->second;
  double frac_region = end_addr_it->second - begin_cum;
  int expected_nodes = frac_region * AttributeValues->GetAttrValuePair()->size();
  int os_addr_space = (min(1.0, (Number/(double)expected_nodes)*fraction) * RAND_MAX)/2;
  AddrEnd = host_id + os_addr_space < 0 ? os_addr_space - (RAND_MAX-host_id) : host_id+os_addr_space;
  AddrBegin = host_id - os_addr_space < 0 ? RAND_MAX - (os_addr_space-host_id) : host_id - os_addr_space;
}

void P2PRdQuery::Initialize(){
  Result->clear();
  CurHops = 0;
  TotalMsgs = 0;
  AddrBegin = -1;
  AddrEnd = -1;
  TotalSatisfyingNode = 0;
  NumQueriedNodes = 0;
}
int P2PRdQuery::CheckResultValidity(int dynamic_period, int res_update_period){
  DynamicAttribute da_tester = DynamicAttribute(dynamic_period, res_update_period);
  map<int,int>::iterator res_it;
  int latency = 1, change_num=0;
  for(res_it=Result->begin();res_it!=Result->end();res_it++){
    change_num = da_tester.CheckIfChanged((res_it->second)*latency)==true?change_num+1:change_num;
  }
  return change_num;
}

int P2PRdQuery::GetRealSatisfyingNodes(P2PNodeFailure* failed_nodes){
  map<int, map<int,vector<int>* >* >* value_nodes_map = AttributeValues->GetValueNodeList();
  map<int, vector<int>* >* vnmap = (*value_nodes_map)[Attribute];
  int real_node_count = 0;
  for(int i=Begin;i<=End;i++){
    if(vnmap->count(i) != 0){
      vector<int>* node_list = (*vnmap)[i];
      for(int k=0;k<node_list->size();k++){
        if(failed_nodes->CheckIfFailed((*node_list)[k]) == false){
          real_node_count++;
        }
      }
    }
  }
  return real_node_count;
}

double P2PRdQuery::GetResultCoverage(){
  if(TotalSatisfyingNode >= Number){
    return ((double)TotalSatisfyingNode/(double)Number);
  }else{
    P2PNodeFailure* failed_nodes = new P2PNodeFailure();
    int real_satisfying_nodes = GetRealSatisfyingNodes(failed_nodes);
    int denom = real_satisfying_nodes < Number ? real_satisfying_nodes : Number;
    delete failed_nodes;
    return ((double)TotalSatisfyingNode/(double)denom);
  }
}
