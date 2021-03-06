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
#include <vector>
#include <map>
#include <stdlib.h>

#include "son_friend_select.h"

#ifndef starsky__son_routing
#define starsky__son_routing

using namespace std;

namespace Starsky {
  class SonRouting{
    public:
      enum ConnectionMode{FRIENDS, NON_FRIENDS, NEAR_CONN, SHORTCUT, FORWARDING, PO_NEAR, DIRECT_FRIEND, NUM_ROUTING_MODE};
      SonRouting(int net_size, int friend_select_method);
      SonRouting(SonFriendSelect* friends_net);
      virtual map<int, map<int, int>* >* BuildRoutingTable() = 0;
      virtual bool CreateEdge(int n1, int n2, int mode);
      virtual bool RemoveEdge(int n1, int n2);      
      virtual void PrintRoutingTable();        
      virtual bool Check();
      void BuildRing();
      void CreateShortcutConn();
      map<int, map<int, int>*>* GetRoutingTable();      
      map<int, multimap<int,int>* >* GetFwdRouteTable();
      bool EdgeExist(int n1, int n2);
      void GetRoutingTableStat();
      static int GetHarmonicDistNode(int source, map<int,int>* target);
    protected:      
      void AddConnsPerQuarter(int host_id, map<int,int>* host_friends);
      bool AddForwardingPath(int source, int target, int gateway);      
      bool DeleteForwardingPath(int source, int target, int gateway);
      bool DeleteForwardingPath(int source, int target);
      void CalculateFwdOverhead();      
      SonFriendSelect* _son_friend;
      unsigned int _max_routing_table_entry;
      unsigned int _num_forwarding;  //for statistics
      unsigned int _num_po_near_conn;  //for statistics
      unsigned int _shortcut_failed; // for statistics
      map<int, map<int, int>* >* _routing_table;
      map<int, multimap<int, int>* >* _freq_map;
      map<int, map<int, int>* >* _friends_map;
      map<int, multimap<int, int>* >* _fwd_route_tbl;  //for forward-routing : a,<c,b> = a<->b<->c
      map<int, multimap<int, int>* >* _fwd_uniq_chk_tbl;  // to check uniqueness of forwarding: a,<b,c> => if go to b, get to c
      map<int, multimap<int, int>* >* _fwd_freq_tbl;  //to check forwarding overhead: b, <a,c>
  };

  class SonKCoverageRouting : public SonRouting{
    public:
      SonKCoverageRouting(int net_size, int friend_select_method);
      SonKCoverageRouting(SonFriendSelect* friends_net);
      map<int, map<int, int>* >* BuildRoutingTable();  
    protected:      
      void AddKConnections(int target_node_id);      
      void DealWithNonKCovered(int host_addr);               
      void UpdateRemainConn(int n1, int n2, bool deduct);      
      void TrimConnections(int host_addr);      
  };

  class SonPrivateOverlayRouting : public SonRouting{
    public:
      SonPrivateOverlayRouting(int net_size, int friend_select_method, int po_th, int max_po);
      SonPrivateOverlayRouting(SonFriendSelect* friends_net, int po_th, int max_po);
      map<int, map<int, int>* >* BuildRoutingTable();  
      bool Check();
      int GetGatewayNode(int source, int dst);      
      void PrintForwardTable();
      void GetLimitedPoStat();      
      map<int, map<int,double>* >* GetJoinablePoMap();
      map<int,int>* GetPoCreateNodes();
    protected:      
      bool IsTrimable(int source, int target);
      void CreatePrivateOverlay(int host_id);
      bool TrimPonConn(int host_id);
      bool TrimPonConn();
      vector<int>* RouteExistAmongFriends(map<int, int>* routing_table, map<int, int>* friends_list, map<int,int>* target_rt);
      void TrimConnections(int host_addr);      
      void CreateOneToOneConns(int host_id);      
      bool MostCommonScCreate(multimap<int, int>* f_freq_table, map<int, int>* host_node_ft, map<int, int>* friend_rt, int friend_id);      
      bool LeastCommonScCreate(multimap<int, int>* f_freq_table, map<int, int>* host_node_ft, map<int, int>* friend_rt, int friend_id);
      bool LeastNearConnScCreate(multimap<int, int>* f_freq_table, map<int, int>* host_node_ft, map<int, int>* fr_table, int friend_id, int predecessor);
      bool RandomShortcutCreate(multimap<int, int>* f_freq_table, map<int, int>* host_node_ft, map<int, int>* fr_table, int friend_id);
      int GetPrivateOverlayJoinStat();
      map<int, map<int, double>* >* BuildJoinablePoMap();
      map<int,int>* DeterminePoCreateNodes();
      unsigned int _threshhold;
      unsigned int _max_po;
      map<int, map<int, double>* >* _joinable_po_map;  // a list of nodes that a node will join in the private overlay
      map<int, multimap<double, int>* >* _score_joinable_po_map; // joinable map of nodes sorted by score
      map<int,int>* _po_create_nodes;  //a list of nodes that will actually create a private ovelay
      map<int, map<int,int>* >* _po_members;
  };

  class SonClusterRouting : public SonRouting{
    public:
      SonClusterRouting(int net_size, int friend_select_method);
      SonClusterRouting(SonFriendSelect* friends_net);
      map<int, map<int, int>* >* BuildRoutingTable();       
      void CheckNoCommonFriends(int host_id);
      void CheckNoCommonFriends();
      void CheckClusterCompleteness(int host_id);         
      static vector<int>* CreateCluster(map<int, int>* self_friend_map, map<int, int>* ff_list);
  };
}
#endif

