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


#ifndef starsky__p2pmessage
#define starsky__p2pmessage

#include <tr1/random>
#include <stdio.h>
#include <iostream>
#include <map>
#include <stdlib.h>
#include <vector>
#include "son_util.h"
#include "p2p_action.h"
#include "p2p_global_data.h"


using namespace std;
namespace Starsky {
  enum P2PRdQueryMode{FIRST_FIT, SUB_REGION, OPTIMAL, DHT, FLOODING, NODE_COUNT};
  class AttrValuePair{
    public:      
      AttrValuePair(map<int,int>* node_list);
      void AddAttribute(int attr, string file_name);
      map<int, map<int,int>* >* GetAttrValuePair();
      map<int, vector<int>* >* GetAttrFreq();         
      map<int, map<int, double>* >* GetAttrCount();       
      map<int, map<int, vector<int>* >* >* GetValueNodeList();
      map<int, map<int, multimap<int,int>*>*>* UpdateToSuperpeers(map<int,int>* superpeer_map);  //superpeer_id/attribute_id/value/host_id
      void CheckCumuMap();
    protected:      
      map<int,int>* _node_list;
      map<int, map<int,int>* >* _attr_value_pair;   //node_id and it assignment
      map<int, map<int, double>* >* _attr_count_index; // to count fraction of each value, attr/value/cumulative dist
      map<int, vector<int>* >* _attr_freq_count;  //a vector for attr frequency, attribute/a list of value cumulatively
      map<int, map<int, vector<int>* >* >* _value_node_map; // to check the result correctness attribue/value/node list
  };

  class DynamicAttribute{
    public:
      DynamicAttribute(int mean_attr_change_time, int update_period);
      void ChangeUpdatePeriod(int new_update_period);
      bool CheckIfChanged(int propagation_time);
    protected:
      std::tr1::ranlux64_base_01 _rand_eng;
      std::tr1::poisson_distribution<int> _poisson_dist;
      int _mean_attr_dynamic_time; //in seconds
      int _attr_update_period; //in seconds
  };

  class P2PResInfoDhtUpdate : public AttrValuePair{
    public:
      P2PResInfoDhtUpdate(map<int,int>* node_list);
      ~P2PResInfoDhtUpdate();
      void UpdateDht();      
      void CheckDhtValidity();
      map<int, map<int, vector<int>*>* >* GetDhtResInfo();
    protected:      
      int DetermineNode(int attribute, int value);
      map<int, map<int, vector<int>*>* >* _dht_res_info;  //node id/attribute id/node lists
//      map<int, map<int, int>* >* _cand_node_count;
  };
  class P2PMessageDist{
    public:
//      P2PMessageDist(map<int, map<int,int>* >* routingt_table);
      P2PMessageDist(map<int, map<int,int>* >* routingt_table, P2PNodeFailure& node_failure);
      int P2PTreeMulticast(int source, int begin_addr, int end_addr, P2PAction* actions, bool clockwise);
      int P2PGreedyRouting(int source, int target, P2PAction* actions);
      int P2PSequentialCrawling(int host_id, int begin_addr, int end_addr, P2PAction* actions);      
      int P2PFlooding(int src_id, int host_id, int ttl, map<int,int>* visited_nodes, P2PAction* actions);
      static map<int, map<int, int>* >* RecursiveTreeAssign(map<int,int>* host_rt, int source, int begin_addr, int end_addr, bool clockwise);
    protected:      
      static map<int, map<int, int>* >* AllocateRegions(map<int,int>* routing_table, int begin_addr, int end_addr, int exclude, bool clockwise);
      map<int, map<int,int>* >* _global_rt;
      P2PNodeFailure _node_failures;
  };

  class P2PRdQuery : public GlobalClass{
    public:
      P2PRdQuery(AttrValuePair* attribute_values, int mode, bool clockwise, int max_target_num);
      P2PRdQuery();
      ~P2PRdQuery();      
      bool CheckResultCorrectness(P2PNodeFailure& failed_nodes);
      bool CheckResultCorrectness(P2PNodeFailure & failed_nodes, map<int, map<int, ResDiscResult*>* >& stat);
      void DetermineDhtQueryRange();      
      void DetermineSubRegionQuery(double fraction, int host_id);    
      void DetermineAllQueryRange();
      void Initialize();
      int CheckResultValidity(int dynamic_period, int res_update_period);

      int Attribute;
      int Begin;
      int End;
      int AddrBegin;
      int AddrEnd;
      int Number;
      int Mode;
      int DistanceFromRoot;
      int CurHops;
      int TotalMsgs;
      int MaxTargetNum;
      bool Clockwise;
      map<int, int>* Result; //key=node id, value = time
      AttrValuePair* AttributeValues; 
  };  
}
#endif



