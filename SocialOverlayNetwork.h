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
#include <stdio.h>
#include <iostream>
#include <map>
#include <stdlib.h>

#ifndef starsky__socialoverlaynetwork
#define starsky__socialoverlaynetwork

using namespace std;
namespace Starsky {
  class SocialOverlayNetwork{
    public:
      SocialOverlayNetwork(int net_size);
      SocialOverlayNetwork();
      void CreateNodeIDs();
      
    protected:
      map<int, int>* _node_list;   // keeps node ID and degree
      int _net_size;
  };
}
#endif

