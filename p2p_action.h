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


#ifndef starsky__p2paction
#define starsky__p2paction
#include <stdio.h>
#include <iostream>
#include <string>
#include <map>
#include <stdlib.h>
#include <vector>
#include "p2p_global_data.h"
#include "son_util.h"


using namespace std;
namespace Starsky {
  class P2PRdQuery;
  class P2PAction{
    public:
      P2PAction(map<string, GlobalClass*>* channel);
      virtual bool Execute();
      virtual bool Execute(int host_id);
      map<string, GlobalClass*>* GetChannel();
    protected:
      map<string, GlobalClass*>* _channel;
  };

  class ResourceDiscoveryAct : public P2PAction{
    public:
      ResourceDiscoveryAct(map<string, GlobalClass*>* channel, map<int, map<int,int>* >* global_rt);
      ~ResourceDiscoveryAct();      
      static void CloneQueryClass(P2PRdQuery* host, P2PRdQuery* copied);
      bool Execute(int host_id);
      bool Aggregate();      
      static bool Matchmaking(int host_id, P2PRdQuery* query);
    protected:      
      bool FreeChildActions();
      map<int, map<int,int>* >* _routing_table;
      multimap<int, ResourceDiscoveryAct*>* _child_actions;
  };
  
  class DhtResDiscAct : public P2PAction{
    public:
      DhtResDiscAct(map<string, GlobalClass*>* channel, map<int, map<int, vector<int>*>* >* dht_res_info);
      bool Execute(int host_id);
    protected:
      map<int, map<int, vector<int>*>* >* _dht_res_info;
  };

  class SingleResDiscAct: public P2PAction{
    public: 
      SingleResDiscAct(map<string, GlobalClass*>* channel);
      bool Execute(int host_id);
  };

  class SuperpeerResDiscAct : public P2PAction{
    public:
      SuperpeerResDiscAct(map<string, GlobalClass*>* channel, map<int, map<int, multimap<int,int>*>*>* spav_asgn);
      bool Execute(int host_id);
    protected:
      map<int, map<int, multimap<int,int>*>*>* _sp_av_assignment; // superpeerID/attribute/value/hostID      
  };
  class P2PNodeCountAct : public P2PAction{
    public:
      P2PNodeCountAct(map<string, GlobalClass*>* channel);
      bool Execute();
     bool Execute(int host_id);
  };
}
#endif
