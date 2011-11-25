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

#ifndef starsky__son_util
#define starsky__son_util

using namespace std;

namespace Starsky {
  class SonStatistics{
    public:
      SonStatistics(int id);
      void UpdateStat(map<int, int>* input_result);
      void UpdateStat(int input);
      virtual void PrintStat();
      void Merge(SonStatistics* in_stat);
    protected:
      int _id;
      unsigned long _sum;
      unsigned long _min;
      unsigned long _max;
      unsigned long _count;
      unsigned long _deliver_fail;
  };
  class SonCumStat : public SonStatistics{
    public:
      SonCumStat(int id);
      SonCumStat();
      void AddCumKey(int key);
      void AddCumKey(map<int,int>* key);
      void PrintStat();
    protected:
      map<int,int>* _cum_dist_map;
  };

  class SonUtil{
    public:
      SonUtil();
      static bool MultimapKeyValueExist(multimap<int,int>* input_mm, int key, int value);      
      static bool DeleteMultimapEntry(multimap<int, int>* input_mm, int key, int value);
      static int GetClosestNode(int src, map<int,int>* routing_table, int dest);    
      static int GetClosestNode(map<int,map<int, int>*>* routing_table, int dest);    
      static int GetDistance(int addr_a, int addr_b);
      static map<int, SonStatistics*>* SummarizeStat(map<int, SonStatistics*>* input_stat_map);
      static map<int,int>* GetInteresctEntry(map<int, int>* rt1, map<int,int>* rt2);
  };
}
#endif
