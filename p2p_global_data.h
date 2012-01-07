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


#ifndef starsky__p2pglobaldata
#define starsky__p2pglobaldata
#include <stdio.h>
#include <iostream>
#include <string>
#include <map>
#include <stdlib.h>
#include <vector>

using namespace std;
namespace Starsky {
  class GlobalClass{
    public:
      GlobalClass();
  };


  class P2PRdResponse : public GlobalClass{
    public:
      P2PRdResponse();
      ~P2PRdResponse();
      map<int, int>* Response;
      int CurHops;
  };

  class P2PStat : public GlobalClass{
    public:
      P2PStat();
      int TotalNodes;
      int MaxDepth;
  };    

  class P2PNodeFailure{
    public:
      P2PNodeFailure();
      P2PNodeFailure(map<int,int>* node_list, int num_failed_node);
      ~P2PNodeFailure();
      bool CheckIfFailed(int test_node_id);
      bool AddFailedNode(int failed_node_id);
    protected:
      int _num_failed_nodes;
      map<int,int>* _failed_nodes;
  };

  class ResDiscResult{
    public:
      ResDiscResult();
      int Count;
      double FalseResult;
      long AvgResultAge;
      long Hops;
      long MaxHops;
      long MinHops;
      long TotalMessages;
      long MaxMessages;
      long MinMessages;
      long TotalQueriedNodes;
      double InCompleteness;
      double Completeness;
  };
}
#endif
