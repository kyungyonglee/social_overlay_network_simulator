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
#include "son_util.h"

using namespace Starsky;
using namespace std;

SonFriendSelect::SonFriendSelect(int net_size):SocialOverlayNetwork(net_size), _net_size_from_file(net_size){
  _friends_map = new map<int, map<int, int>* >();
  _common_friend_freq = new map<int, multimap<int, int>* >();
  _cc_map = new map<string,int>();
}

SonFriendSelect::SonFriendSelect():SocialOverlayNetwork(){
  _friends_map = new map<int, map<int, int>* >();
  _common_friend_freq = new map<int, multimap<int, int>* >();
  _net_size_from_file = 0x7fffffff;
  _cc_map = new map<string,int>();
}

double SonFriendSelect::CalculateCloseness(){
  map<int, map<int, int>* >::iterator it;
  unsigned int total_score = 0, max_friends=0, min_friends = 100000, total = 0, score = 0, per_node_point=0;
  double sum_score = 0.0;
  SonCumStat all_cum;
  SonCumStat over_10_cum;
  SonCumStat over_100_cum;
  SonCumStat over_1000_cum;
  for(it=_friends_map->begin();it != _friends_map->end();it++){
//    cout << it->first << endl;
    multimap<int, int>* freq_table = new multimap<int, int>();
    double local_score = 0.0;
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
    local_score =  ((double)per_node_point/(double)(size*size));
    sum_score += local_score;
    int cum_bin = local_score*100;
    all_cum.AddCumKey(cum_bin);
    if(size > 10){
      over_10_cum.AddCumKey(cum_bin);
      if(size > 100){
        over_100_cum.AddCumKey(cum_bin);
        if(size > 1000){
          over_1000_cum.AddCumKey(cum_bin);
        }
      }
    }
  }
  double avg = (double)total/_friends_map->size();
  double per_node_avg = sum_score/(double)_friends_map->size();
  cout << "total number of friends = " << total << " minimum friends = " << min_friends << " maximum friends = " << max_friends << " average friends = " << avg << endl;
//  cout << "total score = " << total_score << " match score = " << score << " point = " << point << endl;
//  cout << "theory count = " <<theory_count << endl;
  cout << "per node based point = " << per_node_avg << endl;

  cout << "all distribution" <<endl;
  all_cum.PrintStat();
  cout << "over 10 distribution" <<endl;
  over_10_cum.PrintStat();
  cout << "over 100 distribution" <<endl;
  over_100_cum.PrintStat();
  cout << "over 1000 distribution" <<endl;
  over_1000_cum.PrintStat();
  return per_node_avg;
}

double SonFriendSelect::GetOneHopCC(){
  map<int, map<int, int>* >::iterator it;
  unsigned int total_score = 0, total_run = 0, per_node_point=0, run_count=0;
  double sum_score = 0.0;
  for(it=_friends_map->begin();it != _friends_map->end();it++){
    double fr = (double)rand()/(double)RAND_MAX;
    if(fr > 0.1){
      continue;
    }else{
      run_count++;
    }
    per_node_point= 0;
    total_run = 0;
    map<int, int> *f_lists = it->second;
    int location_index = 1;
    map<int, int>::iterator ff_lists;
    for(ff_lists = f_lists->begin(); ff_lists != f_lists->end(); ff_lists++){
      if(_friends_map->count(ff_lists->first) == 0){
        continue;
      }      
      int smaller = ff_lists->first < it->first ? ff_lists->first:it->first;
      int bigger = ff_lists->first < it->first ? it->first:ff_lists->first;
      if(smaller == bigger) continue;
      stringstream key_stream;
      key_stream << smaller << bigger;
      string key = key_stream.str();
      if(_cc_map->count(key) != 0){
        total_run += f_lists->size();
        per_node_point += (*_cc_map)[key];
      }else{        
        map<int, int>* target_map = (*_friends_map)[ff_lists->first];
        map<int, int>::iterator self_lists;
        int local_point=0;
        for(self_lists = f_lists->begin();self_lists!=f_lists->end();self_lists++){
          if(target_map->count(self_lists->first) > 0){
            local_point++;
            per_node_point++;
          }
          total_run++;
        }
        _cc_map->insert(pair<string,int>(key, local_point));
      }
    }
    if(total_run != 0){
      sum_score += ((double)per_node_point/(double)(total_run));
    }
  }
  double per_node_avg = sum_score/(double)run_count;
  cout << "one hop clustering coefficients = " << per_node_avg << endl;
  return per_node_avg;
}

double SonFriendSelect::GetTwoHopsCC(){
  map<int, map<int, int>* >::iterator it;
  unsigned int total_score = 0, total_run = 0, per_node_point=0, run_count=0;
  double sum_score = 0.0;
  for(it=_friends_map->begin();it != _friends_map->end();it++){
    double fr = (double)rand()/(double)RAND_MAX;
    if(fr > 0.01){
      continue;
    }else{
      run_count++;
    }
    per_node_point= 0;
    total_run = 0;
    map<int, int> *f_lists = it->second;
    int location_index = 1;
    map<int, int>::iterator ff_lists;
    for(ff_lists = f_lists->begin(); ff_lists != f_lists->end(); ff_lists++){
      if(_friends_map->count(ff_lists->first) == 0){
        continue;
      }      
      map<int, int>* ff_map = (*_friends_map)[ff_lists->first];
      map<int, int>::iterator foaf_it;
      for(foaf_it=ff_map->begin();foaf_it!=ff_map->end();foaf_it++){
        if(foaf_it->first == it->first) continue;

        int smaller = foaf_it->first < it->first ? foaf_it->first:it->first;
        int bigger = foaf_it->first < it->first ? it->first:foaf_it->first;
        if(smaller == bigger) continue;
        stringstream key_stream;
        key_stream << smaller << bigger;
        string key = key_stream.str();
        if(_cc_map->count(key) != 0){
          total_run += f_lists->size();
          per_node_point += (*_cc_map)[key];
        }else{
          map<int, int>* fff_map = _friends_map->count(foaf_it->first)!=0?(*_friends_map)[foaf_it->first]:NULL;
          if(fff_map == NULL) continue;
          map<int,int>::iterator self_map_it;
          int local_point=0;
          for(self_map_it=f_lists->begin();self_map_it!=f_lists->end();self_map_it++){
            if(fff_map->count(self_map_it->first) > 0){
              per_node_point++;
              local_point++;
            }
            total_run++;
          }
          _cc_map->insert(pair<string,int>(key, local_point));
        }
      }
    }
    if(total_run != 0){
      double cur_point = ((double)per_node_point/(double)(total_run));
      sum_score += cur_point;
      if(run_count %(_friends_map->size()/10000) == 0){
        if(_friends_map->size() > 10000){
          double cur_cc = sum_score/(double)run_count;
          cout << "run index = " <<run_count << " cluster coefficients = " << cur_cc << endl;
        }
      }      
    }
  }
  double per_node_avg = sum_score/(double)run_count;
  cout << "two hops clustering coefficients = " << per_node_avg << endl;
  return per_node_avg;
}

double SonFriendSelect::GetThreeHopsCC(){
  map<int, map<int, int>* >::iterator it;
  unsigned int total_score = 0, total_run = 0, per_node_point=0, run_count=897;
  double sum_score = 46.4967216;
  for(it=_friends_map->begin();it != _friends_map->end();it++){
    double fr = (double)rand()/(double)RAND_MAX;
    if(fr < 0.99){
      continue;
    }else{
      run_count++;
    }
    per_node_point= 0;
    total_run = 0;
    map<int, int> *f_lists = it->second;
    int location_index = 1;
    map<int, int>::iterator ff_lists;
    for(ff_lists = f_lists->begin(); ff_lists != f_lists->end(); ff_lists++){
      if(_friends_map->count(ff_lists->first) == 0){
        continue;
      }      
      map<int, int>* ff_map = (*_friends_map)[ff_lists->first];
      map<int, int>::iterator foaf_it;
      for(foaf_it=ff_map->begin();foaf_it!=ff_map->end();foaf_it++){
        double fr = (double)rand()/(double)RAND_MAX;
        if(foaf_it->first == it->first) continue;
        map<int, int>* fff_map = _friends_map->count(foaf_it->first)!=0?(*_friends_map)[foaf_it->first]:NULL;
        if(fff_map == NULL) continue;
        map<int,int>::iterator fff_map_it;
        for(fff_map_it=fff_map->begin();fff_map_it!=fff_map->end();fff_map_it++){
          int smaller = fff_map_it->first < it->first ? fff_map_it->first:it->first;
          int bigger = fff_map_it->first < it->first ? it->first:fff_map_it->first;
          if(smaller == bigger) continue;
          stringstream key_stream;
          key_stream << smaller << bigger;
          string key = key_stream.str();
          if(f_lists->size()*fff_map->size() > 100000 && _cc_map->count(key) != 0){
            total_run += f_lists->size();
            per_node_point += (*_cc_map)[key];
          }else{
            int local_point = CalculateCC(it->first, fff_map_it->first);
            per_node_point += local_point;
            total_run+= f_lists->size();
            if(f_lists->size()*fff_map->size() > 100000){
              _cc_map->insert(pair<string,int>(key, local_point));
            }
          }
        }
      }
    }
    if(total_run != 0){
      double cur_point = ((double)per_node_point/(double)(total_run));
      sum_score += cur_point;
//      if(_friends_map->size() > 10000 && run_count %(_friends_map->size()/10000) == 0){
//        if(_friends_map->size() > 10000){
          double cur_cc = sum_score/(double)run_count;
          cout << "run index = " <<run_count << " cluster coefficients = " << cur_cc << endl;
//        }
//      }      
    }
  }
  double per_node_avg = sum_score/(double)run_count;
  cout << "three hops clustering coefficients = " << per_node_avg << endl;
  return per_node_avg;
}

double SonFriendSelect::GetFourHopsCC(){
  map<int, map<int, int>* >::iterator it;
  unsigned int total_score = 0, total_run = 0, per_node_point=0, run_count=0;
  double sum_score = 0.0;
  for(it=_friends_map->begin();it != _friends_map->end();it++){
    double fr = (double)rand()/(double)RAND_MAX;
    if(fr < 0.99){
      continue;
    }else{
      run_count++;
    }
    per_node_point= 0;
    total_run = 0;
    map<int, int> *f_lists = it->second;
    int location_index = 1;
    map<int, int>::iterator ff_lists;
    for(ff_lists = f_lists->begin(); ff_lists != f_lists->end(); ff_lists++){
      if(_friends_map->count(ff_lists->first) == 0){
        continue;
      }      
      double fr1 = (double)rand()/(double)RAND_MAX;
      if(fr1 < 0.9) continue;
      map<int, int>* ff_map = (*_friends_map)[ff_lists->first];
      map<int, int>::iterator foaf_it;
      for(foaf_it=ff_map->begin();foaf_it!=ff_map->end();foaf_it++){        
        double fr2 = (double)rand()/(double)RAND_MAX;
        if(fr2 < 0.9) continue;
        if(foaf_it->first == it->first) continue;
        map<int, int>* fff_map = _friends_map->count(foaf_it->first)!=0?(*_friends_map)[foaf_it->first]:NULL;
        if(fff_map == NULL) continue;
        map<int,int>::iterator fff_map_it;
        for(fff_map_it=fff_map->begin();fff_map_it!=fff_map->end();fff_map_it++){
          if(fff_map_it->first == it->first) continue;          
          double fr3 = (double)rand()/(double)RAND_MAX;
          if(fr3 < 0.9) continue;
          map<int, int>* ffff_map = _friends_map->count(fff_map_it->first)!=0?(*_friends_map)[fff_map_it->first]:NULL;
          if(ffff_map == NULL) continue;
          map<int,int>::iterator ffff_map_it;
          for(ffff_map_it=ffff_map->begin();ffff_map_it!=ffff_map->end();ffff_map_it++){            
            double fr4 = (double)rand()/(double)RAND_MAX;
            if(fr4 < 0.9) continue;
            int smaller = ffff_map_it->first < it->first ? ffff_map_it->first:it->first;
            int bigger = ffff_map_it->first < it->first ? it->first:ffff_map_it->first;
            if(smaller == bigger) continue;
            stringstream key_stream;
            key_stream << smaller << bigger;
            string key = key_stream.str();
            if(f_lists->size()*ffff_map->size() > 100000 && _cc_map->count(key) != 0){
              total_run += f_lists->size();
              per_node_point += (*_cc_map)[key];
            }else{
              int local_point = CalculateCC(it->first, ffff_map_it->first);
              per_node_point += local_point;
              total_run+= f_lists->size();
              if(f_lists->size()*ffff_map->size() > 100000){
                _cc_map->insert(pair<string,int>(key, local_point));
              }
            }
          }
        }
      }
    }
    if(total_run != 0){
      double cur_point = ((double)per_node_point/(double)(total_run));
      sum_score += cur_point;
//      if(_friends_map->size() > 10000 && run_count %(_friends_map->size()/10000) == 0){
//        if(_friends_map->size() > 10000){
          double cur_cc = sum_score/(double)run_count;
          cout << "run index = " <<run_count << " cluster coefficients = " << cur_cc << endl;
//        }
//      }      
    }
  }
  double per_node_avg = sum_score/(double)run_count;
  cout << "four hops clustering coefficients = " << per_node_avg << endl;
  return per_node_avg;
}

double SonFriendSelect::GetFiveHopsCC(){
  map<int, map<int, int>* >::iterator it;
  unsigned int total_score = 0, total_run = 0, per_node_point=0, run_count=0;
  double sum_score = 0.0;
  for(it=_friends_map->begin();it != _friends_map->end();it++){
    double fr = (double)rand()/(double)RAND_MAX;
    if(fr < 0.99){
      continue;
    }else{
      run_count++;
    }
    per_node_point= 0;
    total_run = 0;
    map<int, int> *f_lists = it->second;
    int location_index = 1;
    map<int, int>::iterator ff_lists;
    for(ff_lists = f_lists->begin(); ff_lists != f_lists->end(); ff_lists++){
      if(_friends_map->count(ff_lists->first) == 0){
        continue;
      }      
      double fr1 = (double)rand()/(double)RAND_MAX;
      if(fr1 < 0.9) continue;
      map<int, int>* ff_map = (*_friends_map)[ff_lists->first];
      map<int, int>::iterator foaf_it;
      for(foaf_it=ff_map->begin();foaf_it!=ff_map->end();foaf_it++){        
        double fr2 = (double)rand()/(double)RAND_MAX;
        if(fr2 < 0.9) continue;
        if(foaf_it->first == it->first) continue;
        map<int, int>* fff_map = _friends_map->count(foaf_it->first)!=0?(*_friends_map)[foaf_it->first]:NULL;
        if(fff_map == NULL) continue;
        map<int,int>::iterator fff_map_it;
        for(fff_map_it=fff_map->begin();fff_map_it!=fff_map->end();fff_map_it++){
          if(fff_map_it->first == it->first) continue;          
          double fr3 = (double)rand()/(double)RAND_MAX;
          if(fr3 < 0.9) continue;
          map<int, int>* ffff_map = _friends_map->count(fff_map_it->first)!=0?(*_friends_map)[fff_map_it->first]:NULL;
          if(ffff_map == NULL) continue;
          map<int,int>::iterator ffff_map_it;
          for(ffff_map_it=ffff_map->begin();ffff_map_it!=ffff_map->end();ffff_map_it++){            
            double fr4 = (double)rand()/(double)RAND_MAX;
            if(fr4 < 0.9) continue;
            map<int, int>* fffff_map = _friends_map->count(ffff_map_it->first)!=0?(*_friends_map)[ffff_map_it->first]:NULL;
            if(fffff_map == NULL) continue;
            map<int,int>::iterator fffff_map_it;
            for(fffff_map_it=fffff_map->begin();fffff_map_it!=fffff_map->end();fffff_map_it++){           
              double fr5 = (double)rand()/(double)RAND_MAX;
              if(fr5 < 0.9) continue;
              int smaller = fffff_map_it->first < it->first ? fffff_map_it->first:it->first;
              int bigger = fffff_map_it->first < it->first ? it->first:fffff_map_it->first;
              if(smaller == bigger) continue;
              stringstream key_stream;
              key_stream << smaller << bigger;
              string key = key_stream.str();
              if(f_lists->size()*fffff_map->size() > 100000 && _cc_map->count(key) != 0){
                total_run += f_lists->size();
                per_node_point += (*_cc_map)[key];
              }else{
                int local_point = CalculateCC(it->first, fffff_map_it->first);
                per_node_point += local_point;
                total_run+= f_lists->size();
                if(f_lists->size()*fffff_map->size() > 100000){
                  _cc_map->insert(pair<string,int>(key, local_point));
                }
              }
            }    
          }
        }
      }
    }
    if(total_run != 0){
      double cur_point = ((double)per_node_point/(double)(total_run));
      sum_score += cur_point;
//      if(_friends_map->size() > 10000 && run_count %(_friends_map->size()/10000) == 0){
//        if(_friends_map->size() > 10000){
          double cur_cc = sum_score/(double)run_count;
          cout << "run index = " <<run_count << " cluster coefficients = " << cur_cc << endl;
//        }
//      }      
    }
  }
  double per_node_avg = sum_score/(double)run_count;
  cout << "five hops clustering coefficients = " << per_node_avg << endl;
  return per_node_avg;
}


double SonFriendSelect::GetRandomPairCC(){
  map<int, map<int,int>* >::iterator fr_map_it;
  int scores = 0, total_runs = 0, run_count=0;
  for(fr_map_it=_friends_map->begin();fr_map_it!=_friends_map->end();fr_map_it++){
    map<int, map<int,int>* >::iterator inner_fr_map_it = _friends_map->begin();
    advance(inner_fr_map_it, rand()%_friends_map->size());
    if(fr_map_it->first == inner_fr_map_it->first) continue;
    int local_matching = CalculateCC(fr_map_it->first, inner_fr_map_it->first);   
    run_count++;
    scores += (local_matching*2);
    total_runs += fr_map_it->second->size();
    total_runs += inner_fr_map_it->second->size();
    if(run_count %1000 == 0){
      double temp_cc = (double)scores/(double)total_runs;
      cout << "temp cluster coefficients: index = " << run_count << " cc = " <<  temp_cc << endl;
    }
  }
  double random_cc = (double)scores/(double)total_runs;
  cout << "random pair cluster coefficients = " << random_cc << endl;
}

double SonFriendSelect::GetMultipleHopsCC(int hops){
  int target_pair = 1000000, node_num=_friends_map->size(), host_id = 0, next_id=0, run_count = 0;
  map<int,map<int,int>* >::iterator frt_it;
  map<int,int>::iterator nfrt_it;
  map<int,int>* fr_table;
  unsigned long total_runs = 0, total_points = 0, local_runs = 0;
  
//  for(;target_pair>0;target_pair--){
  for(frt_it=_friends_map->begin();frt_it != _friends_map->end();frt_it++){
 //   frt_it = _friends_map->begin();
  //  double fr = rand()/RAND_MAX;
  //  advance(frt_it, rand()%node_num);
    host_id = frt_it->first;    
    if(((*_friends_map)[host_id])->size() < 3) continue;
    int num_measure = ((*_friends_map)[host_id])->size() < 10 ? 1 : ((*_friends_map)[host_id])->size()/10;
    for(int k=0;k<num_measure;k++){
      next_id = host_id;
      for(int i=0;i<hops;i++){
        fr_table = (*_friends_map)[next_id];
        nfrt_it = fr_table->begin();
        advance(nfrt_it, rand()%fr_table->size());
        map<int,int> visited_nodes;
        while(nfrt_it->first == host_id || ((*_friends_map)[nfrt_it->first])->size() < 2){
          nfrt_it = fr_table->begin();
          advance(nfrt_it, rand()%fr_table->size());        
          visited_nodes[nfrt_it->first] = 1;
          if(visited_nodes.size() == fr_table->size()){
            goto LOOP_END;
          }
        }
        next_id = nfrt_it->first;
      }
      local_runs = CalculateCC(host_id, next_id);
      total_points += local_runs;
      total_runs += ((*_friends_map)[host_id])->size();
LOOP_END:      
      run_count++;
    }
  }
  double cc = (double)total_points/(double)total_runs;
  cout << hops << " hops cc = " << cc << " total run = " << run_count<<endl;
  return cc;
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
  ofstream outfile;
  outfile.open(file_name.c_str());
  map<int, map<int, int>* >::iterator it;
  string buf;
  int max_degree = 0;
  for(it=_friends_map->begin(); it != _friends_map->end(); it++){
    map<int, int>* f_lists = it->second;
    if(f_lists == NULL){
      cout << "flist is null" << endl;
      continue;
    }
    max_degree = f_lists->size() > max_degree ? f_lists->size() : max_degree;
    map<int, int>::iterator ff_lists;    
    stringstream ss;
    ss << it->first << "\t";
    for(ff_lists = f_lists->begin(); ff_lists != f_lists->end(); ff_lists++){
      map<int, int>* ff_map = _friends_map->count(ff_lists->first) != 0 ? (*_friends_map)[ff_lists->first] : NULL;
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
  cout << "max degree = " << max_degree;
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

void SonFriendSelect::FreeFreqMap(){
  delete _common_friend_freq;
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

void SonFriendSelect::PrintDegreeDist(){
  map<int, map<int, int>* >::iterator fm_it;
  SonCumStat degree_dist;
  for(fm_it = _friends_map->begin();fm_it != _friends_map->end();fm_it++){
    if(fm_it->second == NULL) continue;
    degree_dist.AddCumKey(fm_it->second->size());
  }
  degree_dist.PrintStat();
}


void SonFriendSelect::CalcNonCommFriendsCC(){
  map<int, multimap<int,int>* >::iterator frt_it;
  unsigned long point = 0, total_inst=0, run_index=0;
  for(frt_it = _common_friend_freq->begin();frt_it!=_common_friend_freq->end();frt_it++){
    multimap<int,int>* frq_tb = frt_it->second;
    pair<multimap<int,int>::iterator, multimap<int,int>::iterator> range = frq_tb->equal_range(0);
    multimap<int,int>::iterator mm_it;
    for(mm_it=range.first;mm_it!=range.second;mm_it++){
      map<int,int>* ff_map = (*_friends_map)[mm_it->second];
      map<int,int>::iterator ff_it;
      for(ff_it=ff_map->begin();ff_it!=ff_map->end();ff_it++){
        point += CalculateCC(frt_it->first, ff_it->first);
        total_inst += frq_tb->size();
      }
    }
    run_index++;
    if(run_index %(_friends_map->size()/100) == 0){
      float cc = (float)point/(float)total_inst;
      if(_friends_map->size() > 10000){
        cout << "run index = " <<run_index << " cluster coefficients = " << cc << endl;
      }
    }
  }  
  float ccf = (float)point/(float)total_inst;
  cout << "no common friends cluster coefficients = " << ccf << endl;
}

int SonFriendSelect::CalculateCC(int node1, int node2){
  map<int,int>* n1_fmap = (*_friends_map)[node1];
  map<int,int>* n2_fmap = (*_friends_map)[node2];
  map<int,int>::iterator fm_it;
  int point = 0;
  for(fm_it=n1_fmap->begin();fm_it!=n1_fmap->end();fm_it++){
    if(n2_fmap->count(fm_it->first) != 0){
      point++;
    }
  }
  return point;
}

void SonFriendSelect::GetNoCommonFriendStat(){
  map<int, multimap<int,int>* >::iterator freq_it;
  map<int, SonStatistics*> no_common_friends_stat;
  for(freq_it=_common_friend_freq->begin();freq_it!=_common_friend_freq->end();freq_it++){
    multimap<int,int>* self_freq_map = freq_it->second;
    int key = self_freq_map->size();
    SonStatistics* local_stat;
    int value = self_freq_map->count(0);
    if(no_common_friends_stat.count(key) != 0){
      local_stat = no_common_friends_stat[key];
    }else{
      local_stat = new SonStatistics(key);
      no_common_friends_stat[key] = local_stat;
    }
    local_stat->UpdateStat(value);
  }
  map<int,SonStatistics*>* summ_stat = SonUtil::SummarizeStat(&no_common_friends_stat);
  map<int,SonStatistics*>::iterator summ_stat_it;
  for(summ_stat_it=summ_stat->begin();summ_stat_it!=summ_stat->end();summ_stat_it++){
    summ_stat_it->second->PrintStat();
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


