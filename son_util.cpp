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
#include <map>
#include <vector>
#include "son_util.h"

using namespace std;
using namespace Starsky;

SonUtil::SonUtil(){}

bool SonUtil::MultimapKeyValueExist(multimap<int,int>* input_mm, int key, int value){
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

bool SonUtil::DeleteMultimapEntry(multimap<int, int>* input_mm, int key, int value){
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

int SonUtil::GetDistance(int addr_a, int addr_b) {
    int sm, bg;
    sm = min(addr_a, addr_b);
    bg = max(addr_a, addr_b);
    return (min( (bg-sm),( (0x7fffffff) - bg + sm) ));
}

int SonUtil::GetClosestNode(int src, map<int,int>* routing_table, int dest){
  if(routing_table == NULL || routing_table->size() == 0){
    return -1;
  }
  map<int, int>::iterator it_upper = routing_table->upper_bound(dest);
  map<int, int>::iterator it_upper1 = it_upper;
  int next_node_addr = 0;
  if (it_upper == routing_table->begin() || it_upper == routing_table->end()){
    if (GetDistance((routing_table->begin())->first, dest) < GetDistance((routing_table->rbegin())->first, dest)) {
      next_node_addr = (routing_table->begin())->first;
    }else {
      next_node_addr = (routing_table->rbegin() )->first;
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
  if(new_dist < GetDistance(src,dest)){
    return next_node_addr;
  }
  return -1;
}

int SonUtil::GetClosestNode(map<int, map<int,int>* >* friends_map, int dest){
  if(friends_map == NULL || friends_map->size() == 0){
    return -1;
  }
  map<int, map<int, int>* >::iterator it_upper = friends_map->upper_bound(dest);
  map<int, map<int, int>* >::iterator it_upper1 = it_upper;
  int next_node_addr = -1;
  if (it_upper == friends_map->begin() || it_upper == friends_map->end()){
    if (GetDistance((friends_map->begin())->first, dest) < GetDistance((friends_map->rbegin())->first, dest)) {
      next_node_addr = (friends_map->begin())->first;
    }else {
      next_node_addr = (friends_map->rbegin() )->first;
    }
  }else {
    it_upper1--;
    if (GetDistance(it_upper->first, dest) < GetDistance( it_upper1->first, dest) ) {
      next_node_addr = it_upper->first;
    }else {
      next_node_addr = it_upper1->first;
    }
  }
  return next_node_addr;
}


map<int, SonStatistics*>* SonUtil::SummarizeStat(map<int, SonStatistics*>* input_stat_map){
  map<int, SonStatistics*>* summary = new map<int, SonStatistics*>();
  map<int, SonStatistics*>::iterator stat_it;
  int prev_key = 0, spacing = 1000;
  SonStatistics* curr_stat;
  for(stat_it=input_stat_map->begin();stat_it!=input_stat_map->end();stat_it++){
    int curr_key = ((stat_it->first/spacing)+1)*spacing;
    if (curr_key == prev_key){
      curr_stat = (*summary)[curr_key];
    }else{
      curr_stat = new SonStatistics(curr_key);
      summary->insert(pair<int, SonStatistics*>(curr_key, curr_stat));
      prev_key = curr_key;
    }
    curr_stat->Merge(stat_it->second);
  }
  return summary;
}

map<int,int>* SonUtil::GetInteresctEntry(map<int, int>* rt1, map<int,int>* rt2){
  map<int,int>* loop_map = rt1->size() > rt2->size() ? rt2:rt1;
  map<int,int>* compare_map = rt1->size() > rt2->size()? rt1:rt2;
  map<int,int>::iterator loop_it;
  map<int,int>* ret_map = new map<int,int>();
  for(loop_it=loop_map->begin();loop_it!=loop_map->end();loop_it++){
    if(compare_map->count(loop_it->first) != 0){
      ret_map->insert(pair<int,int>(loop_it->first, 1));
    }
  }
  return ret_map;
}

SonStatistics::SonStatistics(int id) : _id(id){
  _max = 0;
  _min = 0xffffffff;
  _sum = 0;
  _count = 0;
  _deliver_fail = 0;
}

void SonStatistics::UpdateStat(int input){
  _max = input > _max ? input : _max;
  _min = input < _min ? input : _min;
  _sum += input;
  _count++;
}

void SonStatistics::UpdateStat(map<int, int>* input_result){
  map<int,int>::iterator it;
  for(it=input_result->begin();it!=input_result->end();it++){
    if(it->second > 0){
      _max = it->second > _max ? it->second : _max;
      _min = it->second < _min ? it->second : _min;
      _sum += it->second;
    }else{
      _deliver_fail++;
    }
  }
  _count++;
}

void SonStatistics::PrintStat(){
  unsigned long avg = _sum/(_count*_id-_deliver_fail);
  unsigned long count_based_avg = _sum/(_count);
  cout << _id << "\t" << count_based_avg << "\t" << avg << "\t" << _max << "\t" << _min << "\t" <<_deliver_fail << "\t" << _count<< endl;
}

void SonStatistics::Merge(SonStatistics* in_stat){
  _sum += in_stat->_sum;
  _min = _min < in_stat->_min ? _min : in_stat->_min;
  _max = _max > in_stat->_max ? _max : in_stat->_max;
  _count += in_stat->_count;
  _deliver_fail += in_stat->_deliver_fail;
}

SonCumStat::SonCumStat(int id):SonStatistics(id){
  _cum_dist_map = new map<int,int>();
}

SonCumStat::SonCumStat():SonStatistics(0){
  _cum_dist_map = new map<int,int>();
}

void SonCumStat::AddCumKey(int key){
  int old_value = 0;
  if(_cum_dist_map->count(key) != 0){
    old_value = (*_cum_dist_map)[key];
  }
  (*_cum_dist_map)[key] = old_value+1;
}

void SonCumStat::AddCumKey(map<int,int>* input_map){
  map<int,int>::iterator im_it;
  for(im_it=input_map->begin();im_it!=input_map->end();im_it++){
    AddCumKey(im_it->second);
  }
}

void SonCumStat::PrintStat(){
  map<int,int>::iterator cum_dist_it;
  for(cum_dist_it = _cum_dist_map->begin();cum_dist_it!=_cum_dist_map->end();cum_dist_it++){
    cout <<cum_dist_it->first <<"\t"<<cum_dist_it->second << endl;
  }
}
