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


#ifndef starsky__son_msg_dist
#define starsky__son_msg_dist
#include <vector>
#include <map>
#include "son_routing.h"

using namespace std;
namespace Starsky {
  class SonMsgDist{
    public:
      SonMsgDist(SonRouting* routing_info);
      int GreedyDeliever(int source, int dest, int cur_hops);      
      int FloodDeliever(int source, map<int, int>* dist, int cur_hops);
      int MulticastDeliever(int source, map<int, int>* dist, int cur_hops);
      SonRouting* GetRoutingInfo();      
      static int GetDistance(int addr_a, int addr_b);
    protected:
      SonRouting* _routing_info;
  };
}
#endif

