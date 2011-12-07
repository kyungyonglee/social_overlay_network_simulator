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


#ifndef starsky__p2pnetwork
#define starsky__p2pnetwork
#include <stdio.h>
#include <iostream>
#include <map>
#include <stdlib.h>

using namespace std;
namespace Starsky {
  class P2PNetwork{
    public:      
      enum P2PConMode{NEAR_CONN, SHORTCUT, UNSTRUCTURED, NUM_ROUTING_MODE};      
      P2PNetwork(int net_size);
      P2PNetwork(P2PNetwork* base_network);
      ~P2PNetwork();
      void InitializeRT();
      void CreateNodeIDs();
      void CreateRoutingTable();
      virtual void CreateNetwork();
      virtual void CheckConnection();
      virtual map<int,int>* GetNodeLists();
      virtual map<int, map<int,int>* >*  GetRoutingTable();
      void PrintRoutingTable();
    protected:      
      bool CreateEdge(int host, int target, int mode);
      map<int, int>* _node_list;   // keeps node ID and degree
      int _net_size;      
      map<int, map<int,int>* >* _routing_table;
  };

  class SymphonyP2P : public P2PNetwork{
    public:
      SymphonyP2P(int net_size, int shortcut_num);
      SymphonyP2P(P2PNetwork* base_network, int shortcut_num);
      void CreateNetwork();
      void CheckConnection();
    protected:
      void FormRing();
      void CreateShortcuts();
      int _num_shortcut;
  };

  class UnstructuredP2P : public P2PNetwork{
    public:
      UnstructuredP2P(int net_size, int conn_num);
      UnstructuredP2P(P2PNetwork* base_network, int conn_num);
      void CreateNetwork();
      void CheckConnection();
    protected:
      int _conn_num;
  };

  class SuperPeerP2P : public UnstructuredP2P{
    public:
      SuperPeerP2P(int net_size, int super_peer_num, int sp_conn_num);
      SuperPeerP2P(int super_peer_num,int sp_conn_num, P2PNetwork* base_network);
      ~SuperPeerP2P();
      map<int,int>* GetSuperpeerMap();
      map<int,int>* GetNodeLists();    //returns working nodes;not superpeers
      map<int,int>* GetSuperpeerLists();      
    protected:
      map<int, int>* _superpeer_map;   //no super peer node/superpeer
      P2PNetwork* _non_sp_nodes;
      void DetermineSuperpeer();
      
  };
}
#endif


