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

#include <string>
#include <map>
#include "SocialOverlayNetwork.h"
#ifndef starsky__son_friend_select
#define starsky__son_friend_select

using namespace std;
namespace Starsky {
  class SonFriendSelect : public SocialOverlayNetwork{
    public:
      SonFriendSelect(int net_size);
      SonFriendSelect();
      virtual map<int, map<int, int>* >* DetermineFriends() = 0;
      double CalculateCloseness();
      void WriteToFile(string filename);      
      void WriteCompleteFriendMap(string file_name);
      map<int, multimap<int, int>* >*  BuildCommonFriendsFreq(string input);
      map<int, map<int, int>* >* GetFriendsMap();
      map<int, multimap<int, int>* >* GetFreqMap();      
      void PrintFriendMap();
    protected:
      map<int, map<int, int>* >* _friends_map;   // friend list and common friend
      map<int, multimap<int, int>* >* _common_friend_freq;
      int _net_size_from_file;
  };

  class SonNNFriendSelect : public SonFriendSelect{
    public:
      SonNNFriendSelect(int net_size);
      map<int, map<int, int>* >* DetermineFriends();  
  };

  class SonRandomFriendSelect : public SonFriendSelect{
    public:
      SonRandomFriendSelect(int net_size);
      map<int, map<int, int>* >* DetermineFriends();  
  };

  class SonFriendFromFile : public SonFriendSelect{
    public:
      SonFriendFromFile(string filename);
      SonFriendFromFile(string filename, int net_size);
      map<int, map<int, int>* >* DetermineFriends();  
    protected:
      string _input_file;
      void FillNodeList();
  };
}

#endif

