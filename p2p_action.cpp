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

P2PAction::P2PAction(map<string, GlobalClass*>* channel) : _channel(channel){

}

bool P2PAction::Execute(){
  return false;
}

bool P2PAction::Execute(int host_id){
  return false;
}

map<string, GlobalClass*>* P2PAction::GetChannel(){
  return _channel;
}

ResourceDiscoveryAct::ResourceDiscoveryAct(map<string,GlobalClass*>* channel, map<int, map<int,int>* >* global_rt) : P2PAction(channel), _routing_table(global_rt){
  _child_actions = new multimap<int, ResourceDiscoveryAct*>();
}

ResourceDiscoveryAct::~ResourceDiscoveryAct(){
  delete _child_actions;
}

bool ResourceDiscoveryAct::Execute(int host_id){
  P2PRdQuery* query = _channel->count("query")!=0? (P2PRdQuery*)((*_channel)["query"]):NULL;
  if(query == NULL){
    return true;
  } 
  bool satisfy = Matchmaking(host_id, query);
  if(satisfy == true){
    map<int,int>* result = query->Result;
    result->insert(pair<int,int>(host_id,1));
  }
  map<int, map<int, int>* >* region_asgn = P2PMessageDist::RecursiveTreeAssign((*_routing_table)[host_id], host_id, query->AddrBegin, query->AddrEnd, query->Clockwise);
  map<int, map<int, int>* >::iterator ra_it;
  for(ra_it=region_asgn->begin();ra_it!=region_asgn->end();ra_it++){
    map<int,int>* assign = ra_it->second;
    map<string,GlobalClass*>* arguments = new map<string,GlobalClass*>();
    P2PRdQuery* child_query = new P2PRdQuery();
    CloneQueryClass(query, child_query);
    map<int,int>::iterator assign_it = assign->begin();
    child_query->AddrBegin = assign_it->first;
    child_query->AddrEnd = assign_it->second;
    arguments->insert(pair<string,GlobalClass*>("query", child_query));
    ResourceDiscoveryAct* child_act = new ResourceDiscoveryAct(arguments, _routing_table);
    child_act->Execute(ra_it->first);
    _child_actions->insert(pair<int, ResourceDiscoveryAct*>(child_query->CurHops, child_act));
    delete assign;
  }
  delete region_asgn;

  int child_count = _child_actions->size();
  Aggregate();
  FreeChildActions();
  return true;
}

bool ResourceDiscoveryAct::Aggregate(){  
  P2PRdQuery* query = _channel->count("query")!=0? (P2PRdQuery*)((*_channel)["query"]):NULL;
  map<int,int>* aggr_result = query->Result;
  multimap<int, ResourceDiscoveryAct*>::iterator rda_it;
  for(rda_it=_child_actions->begin();rda_it!=_child_actions->end();rda_it++){
    ResourceDiscoveryAct* rda = rda_it->second;
    map<string, GlobalClass*>* channel = rda->GetChannel();
    P2PRdQuery* child_result = (P2PRdQuery*)((*channel)["query"]);
    map<int,int>* result = child_result->Result;
    map<int,int>::iterator rit;
    for(rit=result->begin();rit!=result->end();rit++){
      (*aggr_result)[rit->first] = rit->second;
    }    
    query->CurHops = (rda_it->first)+1;
    if(aggr_result->size() >= query->Number){
      break;
    }
  }

  if(query->Mode != FIRST_FIT){    
    if(_child_actions->size() != 0){
      query->CurHops = (_child_actions->rbegin()->first)+1;
    }else{
      query->CurHops = 1;
    }
    return false;
  }
}

void ResourceDiscoveryAct::CloneQueryClass(P2PRdQuery* host, P2PRdQuery* copied){
  copied->Attribute = host->Attribute;
  copied->Begin = host->Begin;
  copied->End = host->End;
  copied->Clockwise = host->Clockwise;
  copied->Mode = host->Mode;
  copied->Number = host->Number;
  copied->AttributeValues = host->AttributeValues;
}
bool ResourceDiscoveryAct::FreeChildActions(){
  multimap<int, ResourceDiscoveryAct*>::iterator rda_it;
  for(rda_it=_child_actions->begin();rda_it!=_child_actions->end();rda_it++){
    ResourceDiscoveryAct* rda = rda_it->second;
    map<string, GlobalClass*>* channel = rda->GetChannel();
    map<string,GlobalClass*>::iterator cit;
    for(cit=channel->begin();cit!=channel->end();cit++){
      delete cit->second;
    }
    delete channel;
    delete rda;
  }
}
bool ResourceDiscoveryAct::Matchmaking(int host_id, P2PRdQuery* query){
  map<int, map<int,int>*>* avp = query->AttributeValues->GetAttrValuePair();
  map<int,int>* own_avp = (*avp)[host_id];
  int value = (*own_avp)[query->Attribute];
  if (value >= query->Begin && value <= query->End){
    return true;
  }
  return false;
}

DhtResDiscAct::DhtResDiscAct(map<string,GlobalClass*>* channel, map<int, map<int, vector<int>*>* >* dht_res_info) : P2PAction(channel), _dht_res_info(dht_res_info){

}

bool DhtResDiscAct::Execute(int host_id){
  P2PRdQuery* query = _channel->count("query")!=0? (P2PRdQuery*)((*_channel)["query"]):NULL;
  if(query == NULL){
    return true;
  } 
  if(query->Result->size() >= query->Number){
    return true;
  }
  map<int, map<int,int>* >* attr_val_pairs = query->AttributeValues->GetAttrValuePair();
  map<int, vector<int>*>* host_dht = _dht_res_info->count(host_id)!= 0? (*_dht_res_info)[host_id]:NULL ;
  if(host_dht != NULL){
    if(host_dht->count(query->Attribute) != 0){
      vector<int>* candidate = (*host_dht)[query->Attribute];
      for(int i=0;i<candidate->size();i++){
        map<int,int>* local_pair = (*attr_val_pairs)[(*candidate)[i]];
        if(((*local_pair)[query->Attribute] >= query->Begin) && ((*local_pair)[query->Attribute] <= query->End)){
          query->Result->insert(pair<int,int>((*candidate)[i], host_id));
        }
      }
    }
  }
  if(query->Result->size() >= query->Number){
    return true;
  }else{
    return false;
  }
}

SingleResDiscAct::SingleResDiscAct(map<string, GlobalClass*>* channel):P2PAction(channel){
}

bool SingleResDiscAct::Execute(int host_id){
  P2PRdQuery* query = _channel->count("query")!=0? (P2PRdQuery*)((*_channel)["query"]):NULL;
  if(query != NULL){
    if(ResourceDiscoveryAct::Matchmaking(host_id, query) == true){
      query->Result->insert(pair<int,int>(host_id,1));
      return true;
    }
  }
  return false;
}

SuperpeerResDiscAct::SuperpeerResDiscAct(map<string, GlobalClass*>* channel, map<int, map<int, multimap<int,int>*>*>* spav_asgn):P2PAction(channel), _sp_av_assignment(spav_asgn){

}

bool SuperpeerResDiscAct::Execute(int host_id){
  bool matching = false;
  P2PRdQuery* query = _channel->count("query")!=0? (P2PRdQuery*)((*_channel)["query"]):NULL;
  map<int, multimap<int,int>*>* avn = _sp_av_assignment->count(host_id)!=0?(*_sp_av_assignment)[host_id]:NULL;
  if(query != NULL && avn != NULL){
    multimap<int,int>* value_node = avn->count(query->Attribute)!=0?(*avn)[query->Attribute]:NULL;
    if(value_node != NULL){
      pair<multimap<int,int>::iterator, multimap<int,int>::iterator> range;
      multimap<int,int>::iterator vn_it;
      for(int q=query->Begin;q<=query->End;q++){
        range = value_node->equal_range(q);
        for(vn_it = range.first;vn_it!=range.second;vn_it++){
          query->Result->insert(pair<int,int>(vn_it->second,1));
          matching = true;
        }
      }      
    }
  }
  return matching;
}

P2PNodeCountAct::P2PNodeCountAct(map<string,GlobalClass*>* channel) : P2PAction(channel){
}

bool P2PNodeCountAct::Execute(){
  P2PStat* stat;
  if(_channel->count("result") == 0){
    stat = new P2PStat();
  }else{
    stat = (P2PStat*)(*_channel)["result"];
  }
  stat->TotalNodes = stat->TotalNodes + 1;
  (*_channel)["result"] = stat;
  return true;
}

bool P2PNodeCountAct::Execute(int host_id){
  Execute();
}