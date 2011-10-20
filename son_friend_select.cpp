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

#include <string.h>
#include <iostream>
#include <sstream>
#include <fstream>

#include "son_friend_select.h"

using namespace Starsky;
using namespace std;

SonFriendSelect::SonFriendSelect(int net_size):SocialOverlayNetwork(net_size), _net_size_from_file(net_size){
  _friends_map = new map<int, map<int, int>* >();
  _common_friend_freq = new map<int, multimap<int, int>* >();
}

SonFriendSelect::SonFriendSelect():SocialOverlayNetwork(){
  _friends_map = new map<int, map<int, int>* >();
  _common_friend_freq = new map<int, multimap<int, int>* >();
  _net_size_from_file = 0x7fffffff;
}

double SonFriendSelect::CalculateCloseness(){
  map<int, map<int, int>* >::iterator it;
  unsigned int total_score = 0, max_friends=0, min_friends = 100000, total = 0, score = 0, per_node_point=0;
  double sum_score = 0.0;
  for(it=_friends_map->begin();it != _friends_map->end();it++){
//    cout << it->first << endl;
    multimap<int, int>* freq_table = new multimap<int, int>();
    per_node_point= 0;
    map<int, int> *f_lists = it->second;
    int size = f_lists->size();
    total += size;
    int location_index = 1;
    max_friends = f_lists->size() > max_friends ? f_lists->size() : max_friends;
    min_friends = f_lists->size() < min_friends ? f_lists->size() : min_friends;
    map<int, int>::iterator ff_lists;
    for(ff_lists = f_lists->begin(); ff_lists != f_lists->end(); ff_lists++){
      int local_point = 0;
      if(_friends_map->count(ff_lists->first) == 0){
        continue;
      }      
      map<int, int>* target_map = (*_friends_map)[ff_lists->first];
      map<int, int>::iterator self_lists = f_lists->begin();
 //     advance(self_lists, location_index);
      for(self_lists = f_lists->begin();self_lists!=f_lists->end();self_lists++){
        map<int, int>::iterator temp_it;
        if(target_map->count(self_lists->first) > 0){
          score++;
          local_point++;
          per_node_point++;
        }
        total_score++;
      }
      location_index++;
      freq_table->insert(pair<int, int>(local_point, ff_lists->first));
    }
    _common_friend_freq->insert(pair<int, multimap<int, int>* >(it->first, freq_table));
    sum_score += ((double)per_node_point/(double)(size*size));
  }
  double avg = (double)total/_friends_map->size();
  double per_node_avg = sum_score/(double)_friends_map->size();
  cout << "total number of friends = " << total << " minimum friends = " << min_friends << " maximum friends = " << max_friends << " average friends = " << avg << endl;
//  cout << "total score = " << total_score << " match score = " << score << " point = " << point << endl;
//  cout << "theory count = " <<theory_count << endl;
//  cout << "per node based point = " << per_node_avg << endl;
  return per_node_avg;
}

void SonFriendSelect::WriteToFile(string file_name){
/////write frequency map to a file
  ofstream outfile;
  outfile.open(file_name.c_str());
  map<int, multimap<int, int>* >::iterator it;
  string buf;
  for(it=_common_friend_freq->begin(); it != _common_friend_freq->end(); it++){
    multimap<int, int>* f_lists = it->second;
    multimap<int, int>::iterator ff_lists;
    for(ff_lists = f_lists->begin(); ff_lists != f_lists->end(); ff_lists++){
      stringstream ss;
      ss << it->first << "\t" << ff_lists->second << "\t" << ff_lists->first << endl ;
      buf = ss.str();     
      outfile << buf;      
    }    
  }  
  outfile.close();
}

void SonFriendSelect::WriteCompleteFriendMap(string file_name){
/////write frequency map to a file
  ofstream outfile;
  outfile.open(file_name.c_str());
  map<int, map<int, int>* >::iterator it;
  string buf;
  for(it=_friends_map->begin(); it != _friends_map->end(); it++){
    map<int, int>* f_lists = it->second;
    if(f_lists == NULL){
      cout << "flist is null" << endl;
      continue;
    }
    map<int, int>::iterator ff_lists;    
    stringstream ss;
    ss << it->first << "\t";
    for(ff_lists = f_lists->begin(); ff_lists != f_lists->end(); ff_lists++){
      map<int, int>* ff_map = (*_friends_map)[ff_lists->first];
      if (ff_map == NULL){
        cout << "ffmap is null" << endl;
        continue;
      }
      if (NULL != ff_map && ff_map->count(it->first) != 0){
        ss << ff_lists->first << "\t" ;
      }
    }    
    ss << "\n";
    buf = ss.str();     
    outfile << buf;      
  }  
  outfile.close();
}


map<int, multimap<int, int>* >*  SonFriendSelect::BuildCommonFriendsFreq(string input){
  string line;
  ifstream file(input.c_str());
  if(file.is_open()){
    int self_id = -1, friend_id=-1, frequency = -1;
    multimap<int, int>* table;
    while(file.good()){
      getline(file, line);
      char* pch;
      int index = 0;
      pch = strtok((char*)line.c_str(), "\t");
      while(pch != NULL){
        if(index == 0){
          self_id = atoi(pch);
          if (self_id > _net_size_from_file){
            break;
          }
        }else if (index == 1){
          friend_id = atoi(pch);
          if(friend_id > _net_size_from_file){
            break;
          }
        }else if (index == 2){
          frequency = atoi(pch);          
          if(_common_friend_freq->count(self_id) == 0){
            table = new multimap<int, int>();
            _common_friend_freq->insert(pair<int, multimap<int, int>* >((int)self_id, table));
          }else{
            table = (*_common_friend_freq)[self_id];
          }
          table->insert(pair<int, int>(frequency, (int)friend_id));
          break;
        }
        pch = strtok(NULL, "\t");
        index++;
      }
    }
    file.close();
  }
  return _common_friend_freq;
}

map<int, map<int, int>* >* SonFriendSelect::GetFriendsMap(){
  return _friends_map;
}

map<int, multimap<int, int>* >* SonFriendSelect::GetFreqMap(){
  return _common_friend_freq;
}

void SonFriendSelect::PrintFriendMap(){
  unsigned int threshold = 100;
  map<int, map<int, int>* >::iterator fm_it;
  for(fm_it = _friends_map->begin();fm_it != _friends_map->end();fm_it++){
    int num_join_overlay = 0;
    if (fm_it->second->size() > threshold){
      map<int, int>* ffm = fm_it->second;
      map<int, int>::iterator ffm_it;
      for(ffm_it = ffm->begin();ffm_it != ffm->end();ffm_it++){
        if(_friends_map->count(ffm_it->first) != 0){
          map<int, int>* tfm = (*_friends_map)[ffm_it->first];
          if(tfm->size() > threshold){
            num_join_overlay++;
 //           cout << "both private overlay : " << fm_it->second->size() << " : " << tfm->size() << endl; 
          }
        }
      }
      cout << "both private overlay : " << fm_it->second->size() << " : " << num_join_overlay<< endl; 
    }
  }
}

SonFriendFromFile::SonFriendFromFile(string filename) : SonFriendSelect(), _input_file(filename){
  DetermineFriends();
}

SonFriendFromFile::SonFriendFromFile(string filename, int net_size) : SonFriendSelect(net_size), _input_file(filename){
  DetermineFriends();
}

map<int, map<int, int>* >* SonFriendFromFile::DetermineFriends(){
  string line;
  ifstream file(_input_file.c_str());
  if(file.is_open()){
    int self_id = -1, friend_id=-1;
    map<int, int>* table;
    while(file.good()){
      getline(file, line);
      self_id = -1;
      char* pch;
      pch = strtok((char*)line.c_str(), "\t");
      while(pch != NULL){        
        if(self_id < 0){
          self_id = (int)atoi(pch);
          if( self_id > _net_size_from_file){
            break;
          }
        }else{
          friend_id = (int)atoi(pch);
          if(friend_id > _net_size_from_file){
            pch = strtok(NULL, "\t");
            continue;
          }
          if(_friends_map->count(self_id) == 0){
            table = new map<int, int>();
            _friends_map->insert(pair<int, map<int, int>* >(self_id, table));
          }else{
            table = (*_friends_map)[self_id];
          }
          table->insert(pair<int, int>(friend_id, 3));
        }
        pch = strtok(NULL, "\t");
      }
    }
    file.close();
  }
  return _friends_map;
}

void SonFriendFromFile::FillNodeList(){

}

SonNNFriendSelect::SonNNFriendSelect(int net_size) : SonFriendSelect(net_size){
  DetermineFriends();
}

map<int, map<int, int>* >* SonNNFriendSelect::DetermineFriends(){
  return NULL;
}


SonRandomFriendSelect::SonRandomFriendSelect(int net_size) : SonFriendSelect(net_size){
  DetermineFriends();
}

map<int, map<int, int>* >* SonRandomFriendSelect::DetermineFriends(){
  return NULL;
}


