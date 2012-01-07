#include <iostream>
#include <sstream>
#include <fstream>
#include <cmath>
#include <string.h>

#include "p2p_network.h"
#include "p2p_message.h"
#include "son_util.h"
#include "p2p_global_data.h"

#define SIMULATION

using namespace std;
using namespace Starsky;

int PerformFlooding(P2PMessageDist& msg_dist, P2PAction* res_disc_act, int begin_addr, int ttl, bool superpeer, map<int,int>* sp_map){
  map<string, GlobalClass*>* channel = res_disc_act->GetChannel();
  P2PRdQuery* res_disc_query = (P2PRdQuery*)((*channel)["query"]);
  map<int,int>* result = res_disc_query->Result;
  map < int,int >* visited_nodes = new map<int,int>();
  map<int,int> handled_sp_nodes;
  int total_msg = msg_dist.P2PFlooding(begin_addr, begin_addr, ttl, visited_nodes,res_disc_act);
  map<int,int>::iterator res_it;
  int max_hops = 0;
  multimap<int,int> temp_sort;
  for(res_it=result->begin();res_it!=result->end();res_it++){
    int host_node;
    if(superpeer == true){
      host_node = (*sp_map)[res_it->first];
    }else{
      host_node = res_it->first;
    }
    int cur_hops = visited_nodes->count(host_node)!= 0 ? (ttl-((*visited_nodes)[host_node])) : -1;
    int greedy_rt_hops = msg_dist.P2PGreedyRouting(host_node, begin_addr, NULL);
    if(superpeer == true){
      greedy_rt_hops += 2;
    }
    if(superpeer==true){
      if( handled_sp_nodes.count(host_node) == 0){
        total_msg += greedy_rt_hops;
        handled_sp_nodes[host_node] = 1;
      }
    }else{
      total_msg += greedy_rt_hops;
    }
    temp_sort.insert(pair<int,int>((cur_hops+greedy_rt_hops), host_node));
  }
  if(temp_sort.size() <= res_disc_query->Number){
    if(temp_sort.size() != 0){
       max_hops = temp_sort.rbegin()->first;
    }else{
      max_hops = 0;
    }
  }else{
    multimap<int,int>::iterator ts_it = temp_sort.begin();
    advance(ts_it, res_disc_query->Number);
    max_hops = ts_it->first;
  }
  res_disc_query->CurHops = max_hops;
  res_disc_query->TotalMsgs = total_msg;
  delete visited_nodes;
//  cout << "unstructed query total message = " << total_msg << endl;
  return total_msg;
}

void PrintQueryStat(map<int, map<int, ResDiscResult*>* >& stat, string prefix){
  map<int, map<int, ResDiscResult*>* >::iterator all_it;
  for(all_it=stat.begin();all_it!=stat.end();all_it++){
    map<int, ResDiscResult*>::iterator sub_it;
    for(sub_it = all_it->second->begin();sub_it!=all_it->second->end();sub_it++){
      ResDiscResult* result = sub_it->second;
      cout <<prefix <<"\t" << all_it->first <<"\t"<< sub_it->first  <<"\t"<< result->Count << "\t" << (result->Hops/result->Count) << "\t" << result->MaxHops << "\t" << result->MinHops << "\t" << (result->Completeness/result->Count) << "\t" << (result->TotalMessages/result->Count) << "\t"  << result->MaxMessages << "\t" << result->MinMessages << "\t" << (result->AvgResultAge/result->Count) << "\t" << (result->FalseResult/(double)result->Count)  << "\t" << (result->TotalQueriedNodes/result->Count) <<endl;
    }
  }
}

void PrintCoverageStat(string prefix, map<double, vector<double>* >& result){
  map<double, vector<double>* >::iterator res_it;
  
  for(res_it=result.begin();res_it!=result.end();res_it++){
    vector<double>* covg_vector = res_it->second;
    double sum=0.0, avg=0.0, max=0.0, min=10000000.0;
    multimap<double, int> temp_buf;
    for(int i=0;i<covg_vector->size();i++){
      cout << prefix << "\t" << res_it->first << "\t" << (*covg_vector)[i] << endl;
//      temp_buf.insert(pair<double,int>((*covg_vector)[i],1));
//      sum += (*covg_vector)[i];
//      min = min <(*covg_vector)[i] ? min : (*covg_vector)[i];
//      max = max < (*covg_vector)[i] ? (*covg_vector)[i] : max;
    }
/*    
    avg = sum/covg_vector->size();
    double sqared_sum = 0.0;
    for(int j=0;j<covg_vector->size();j++){
      sqared_sum += pow((avg-(*covg_vector)[j]),2);
    }    
    double std_dev = sqared_sum/covg_vector->size();
    int size = temp_buf.size();
    multimap<double,int>::iterator tbit = temp_buf.begin();
    advance(tbit, size/95);
    double low_1 = tbit->first;
    tbit = temp_buf.begin();
    advance(tbit, size-(size/95));
    double high_1 = tbit->first;
*/    
//    cout << res_it->first << "\t" << avg << "\t" << std_dev <<"\t"<< low_1 << "\t" << high_1 << endl;
  }
}

void RunSymphony(P2PNetwork* base_network, AttrValuePair* avp, P2PMessageDist& msg_dist){
}

void RunUnstructured(P2PNetwork* base_network, AttrValuePair* avp, P2PMessageDist& msg_dist){
}

void RunSuperpeer(P2PNetwork* base_network, AttrValuePair* avp, P2PMessageDist& msg_dist){
}

void CompareIncompleteness(map<int, map<int, ResDiscResult*>* >& better, map<int, map<int, ResDiscResult*>* >& worse){
  map<int, map<int, ResDiscResult*>* >::iterator better_it;
  for(better_it=better.begin();better_it!=better.end();better_it++){
    map<int, ResDiscResult*>* bmap = better_it->second;
    map<int, ResDiscResult*>::iterator bmap_it;
    for(bmap_it=bmap->begin();bmap_it!=bmap->end();bmap_it++){
      ResDiscResult* better_res = bmap_it->second;
      ResDiscResult* worse_res = (*(worse[better_it->first]))[bmap_it->first];
      if(better_res->InCompleteness > worse_res->InCompleteness){
        cout << "This should not happen"<< endl;
      }
    }
  }
}
#ifdef SIMULATION
int main(int argc, char *argv[]){
  srand((unsigned)time(0));  
  int num_node, ttl, num_failed_nodes, redundant_tree;
  int total_hops = 0, run_index=0, target_run_index = 1000;
  int node_index = 0, spacing, max_target_num = 9999;
  
  if(argc == 5){
    num_node = atoi(argv[1]);
    ttl = atoi(argv[2]);
    num_failed_nodes = atoi(argv[3]);
    redundant_tree = atoi(argv[4]);
  }else{
    cout << "Usage : p2p_network network_size ttl number_of_failed_nodes redundant_tree(0:no 1:Yes)"<< endl;
    return 1;
  }
  
  P2PNetwork* p2p = new P2PNetwork(num_node);
  map<int, map<int, ResDiscResult*>* > tree_all_query_stat;
  map<int, map<int, ResDiscResult*>* > ff_query_stat;
  map<int, map<int, ResDiscResult*>* > sr_query_stat;
  map<int, map<int, ResDiscResult*>* > dht_query_stat;
  map<int, map<int, ResDiscResult*>* > unstructured_query_stat;
  map<int, map<int, ResDiscResult*>* > superpeer_query_stat;

  map<int,int>* node_list = p2p->GetNodeLists();
  P2PResInfoDhtUpdate* dht_avp = new P2PResInfoDhtUpdate(node_list);
  dht_avp->AddAttribute(0, "./zipf_10");
  dht_avp->AddAttribute(1, "./zipf_100");
  dht_avp->AddAttribute(2, "./zipf_1000");
  dht_avp->UpdateDht();

  int superpeer_nodes_num = (node_list->size())/200;
  SymphonyP2P symphony_net = SymphonyP2P(p2p, log(num_node));
  UnstructuredP2P unstructured_net = UnstructuredP2P(p2p, (log(num_node)));
  SuperPeerP2P superpeer_net = SuperPeerP2P(superpeer_nodes_num, log(superpeer_nodes_num), p2p);

  P2PAction* res_disc_act;
  map<int,int>::iterator node_it;
  P2PNodeFailure* failed_nodes = new P2PNodeFailure(p2p->GetNodeLists(), num_failed_nodes);   // for super-peer, it will be non-super-peer nodes

//add failed node among super peerss; same ratio with overall failure
  map<int,int>* sp_nodes = superpeer_net.GetSuperpeerLists();
  map<int,int>::iterator spn_it;
  int sp_fail_node = (superpeer_nodes_num/(double)num_node) * num_failed_nodes;
//  cout << "sp_fail_node = " << sp_fail_node << endl;
  for(int i=0;i<sp_fail_node;){
    spn_it = sp_nodes->begin();
    advance(spn_it, rand()%sp_nodes->size());
    if(failed_nodes->AddFailedNode(spn_it->first) == true){
      i++;
    }
  }
  
  P2PMessageDist symph_msg_dist = P2PMessageDist((symphony_net.GetRoutingTable()), failed_nodes);
  P2PMessageDist unstr_msg_dist = P2PMessageDist((unstructured_net.GetRoutingTable()), failed_nodes);
  P2PMessageDist sp_msg_dist = P2PMessageDist((superpeer_net.GetRoutingTable()), failed_nodes);
  
  map<int,int>* sp_node_map = superpeer_net.GetSuperpeerMap();  // for superpeer
  map<int, map<int, multimap<int,int>*>*>* savn = dht_avp->UpdateToSuperpeers(sp_node_map);  //for superpeer

  spacing = num_node/target_run_index;
  map<double, vector<double>* > static_sub_region_res_coverage;
  map<double, vector<double>* > dynamic_sub_region_res_coverage;
  for(node_it=node_list->begin();node_it!=node_list->end();node_it++){
    if(node_index++%spacing != 0 || failed_nodes->CheckIfFailed(node_it->first)==true){          
      continue;
    }
    run_index++;
//    cout << node_index << endl;
    
    map<string, GlobalClass*>* channel = new map<string, GlobalClass*>();
    P2PRdQuery* res_disc_query = new P2PRdQuery(dht_avp, DHT, true, max_target_num);
    (*channel)["query"] = res_disc_query;


//To check tree-depth
/*
    res_disc_query->Mode = OPTIMAL;
    res_disc_query->Clockwise = true;
    res_disc_query->DetermineAllQueryRange();
    res_disc_act = new ResourceDiscoveryAct(channel, symphony_net.GetRoutingTable(), failed_nodes, false);
    res_disc_act->Execute(node_it->first);
    delete (ResourceDiscoveryAct*)res_disc_act;
    total_hops += res_disc_query->CurHops;
    res_disc_query->Initialize();
*/
// Sub-Region query complteness check based on queried region
/*
    for(int b=0;b<=1;b++){
      for(double d=0.6;d<=1.4;d+=0.2){
        res_disc_query->Initialize();
        res_disc_query->Mode = SUB_REGION;
        res_disc_query->DetermineSubRegionQuery(d, node_it->first);
        res_disc_act = new ResourceDiscoveryAct(channel, symphony_net.GetRoutingTable(), failed_nodes, (bool)b);
        res_disc_act->Execute(node_it->first);
        delete (ResourceDiscoveryAct*)res_disc_act;
        vector<double>* buf;
        if(b == 0){
          if(static_sub_region_res_coverage.count(d) == 0){
            buf = new vector<double>();
            static_sub_region_res_coverage[d] = buf;
          }else{
            buf = static_sub_region_res_coverage[d];
          }
          buf->push_back(res_disc_query->GetResultCoverage());
        }else{
          if(dynamic_sub_region_res_coverage.count(d) == 0){
            buf = new vector<double>();
            dynamic_sub_region_res_coverage[d] = buf;
          }else{
            buf = dynamic_sub_region_res_coverage[d];
          }
          buf->push_back(res_disc_query->GetResultCoverage());
        }        
      }
      res_disc_query->Initialize();
    }
*/    

//MAAN (Dht-based) resource discovery
///*
    res_disc_query->DetermineDhtQueryRange();
    res_disc_act = new DhtResDiscAct(channel, dht_avp->GetDhtResInfo(), failed_nodes);
//    cout <<node_it->first << "\t" << res_disc_query->AddrBegin << "\t"<<res_disc_query->AddrEnd << endl ;
    res_disc_query->CurHops = symph_msg_dist.P2PSequentialCrawling(node_it->first, res_disc_query->AddrBegin, res_disc_query->AddrEnd, res_disc_act);          
    res_disc_query->TotalMsgs = res_disc_query->CurHops;
    delete res_disc_act;
    res_disc_query->CheckResultCorrectness(failed_nodes, dht_query_stat);
    res_disc_query->Initialize();
//*/

//Result Comparison between D-MARD modules (querying the entire network)
/*    
    res_disc_query->Mode = OPTIMAL;
    res_disc_query->Clockwise = true;
    res_disc_query->DetermineAllQueryRange();
    res_disc_act = new ResourceDiscoveryAct(channel, symphony_net.GetRoutingTable(), failed_nodes, false);
    res_disc_act->Execute(node_it->first);
    delete (ResourceDiscoveryAct*)res_disc_act;
    res_disc_query->CheckResultCorrectness(failed_nodes, tree_all_query_stat);
    res_disc_query->Initialize();
*/

//First fit and subregion:determined based on tree dedundancy
///*
    res_disc_query->Mode = FIRST_FIT;
    res_disc_query->Clockwise = true;
    res_disc_query->DetermineAllQueryRange();
    res_disc_act = new ResourceDiscoveryAct(channel, symphony_net.GetRoutingTable(), failed_nodes, false);
    res_disc_act->Execute(node_it->first);
    delete (ResourceDiscoveryAct*)res_disc_act;
    int cur_hops = res_disc_query->CurHops;
//// In a non-redundant mode, we will run sub-region query propagation


    if(redundant_tree == 1){
      res_disc_query->CheckResultCorrectness(failed_nodes, sr_query_stat);
      res_disc_query->Clockwise = false;
      res_disc_query->DetermineAllQueryRange();
      res_disc_act = new ResourceDiscoveryAct(channel, symphony_net.GetRoutingTable(), failed_nodes, false);
      res_disc_act->Execute(node_it->first);
      delete (ResourceDiscoveryAct*)res_disc_act;
      res_disc_query->CurHops = res_disc_query->CurHops > cur_hops ? res_disc_query->CurHops : cur_hops;
      res_disc_query->CheckResultCorrectness(failed_nodes, ff_query_stat);
      res_disc_query->Initialize();
    }else{
////result initialize for previous first-fit query
      res_disc_query->CheckResultCorrectness(failed_nodes, ff_query_stat);
      res_disc_query->Initialize();

      bool complete = false;
      double query_region = 1.0;
      int prev_begin, prev_end;
      while(complete == false){
        res_disc_query->Initialize();
        res_disc_query->Mode = SUB_REGION;
        res_disc_query->DetermineSubRegionQuery(query_region, node_it->first);
        res_disc_act = new ResourceDiscoveryAct(channel, symphony_net.GetRoutingTable(), failed_nodes, false);
        res_disc_act->Execute(node_it->first);
        delete (ResourceDiscoveryAct*)res_disc_act;
        if(res_disc_query->Result->size() >= res_disc_query->Number || (res_disc_query->AddrBegin==prev_begin && res_disc_query->AddrEnd==prev_end)){
          complete = true;
        }
        query_region += 0.3;
        prev_begin = res_disc_query->AddrBegin;
        prev_end = res_disc_query->AddrEnd;
      }
      res_disc_query->CheckResultCorrectness(failed_nodes, sr_query_stat);
      res_disc_query->Initialize();
    }
//*/

//unstructured message propagation
///*   
    res_disc_act = new SingleResDiscAct(channel, failed_nodes);
    PerformFlooding(unstr_msg_dist, res_disc_act, node_it->first, ttl, false, NULL);
    res_disc_query->CheckResultCorrectness(failed_nodes, unstructured_query_stat);
    res_disc_query->Initialize();
    delete res_disc_act;   
//*/

//Superpeer-based resource discovery
///*
    int begin_sp_node = (*sp_node_map)[node_it->first];
    res_disc_act = new SuperpeerResDiscAct(channel, savn, failed_nodes);
    PerformFlooding(sp_msg_dist, res_disc_act, begin_sp_node, ttl, true, sp_node_map);
    res_disc_query->CheckResultCorrectness(failed_nodes, superpeer_query_stat);
    res_disc_query->Initialize();
    delete res_disc_act;  
// */
    delete res_disc_query;
    delete channel;
  }
//  double avg_hops = total_hops/(double)run_index;
//  cout << num_node << "\t" << avg_hops << endl;
//  PrintCoverageStat("static", static_sub_region_res_coverage);
//  cout << endl;
//  PrintCoverageStat("dynamic", dynamic_sub_region_res_coverage);
  PrintQueryStat(dht_query_stat, "dht");
//  PrintQueryStat(tree_all_query_stat, "tree-all");
  PrintQueryStat(ff_query_stat, "first-fit");
  PrintQueryStat(sr_query_stat, "sub-region");
  PrintQueryStat(unstructured_query_stat, "unstructured");
  PrintQueryStat(superpeer_query_stat, "superpeer");
//  CompareIncompleteness(ff_query_stat, sr_query_stat);
  return 1;
}
#else
int main(int argc, char *argv[]) 
{
    P2PNetwork* p2p;
    srand((unsigned)time(0));    
    
    if(argc == 6){
      int num_node = atoi(argv[1]);
      int p2p_mode = atoi(argv[2]); 
      int query_mode = atoi(argv[3]);
      int ttl = atoi(argv[4]);
      int num_failed_nodes = atoi(argv[5]);
      int total_hops = 0, run_index=0, target_run_index = 1000;
      int node_index = 0, spacing, max_target_num = 99;
      
      switch(p2p_mode){
        case 0:
          p2p = new SymphonyP2P(num_node, (log(num_node)));
          break;
        case 1:        
          p2p = new UnstructuredP2P(num_node, (log(num_node)));
          break;
        case 2:
          p2p = new SuperPeerP2P(num_node, (num_node/200), log(num_node/200));
          break;
        default:
          cout << "not available network mode (0:Symphony, 1: Unstructured 2: Superpeer)" << endl;
          return 1;
          break;
      }

 //     p2p->PrintRoutingTable();
//      p2p->CheckConnection();
      map<int, map<int, ResDiscResult*>* > query_stat;
      for(int i=10;i<=100;i+=10){
        map<int, ResDiscResult*>* res_disc_map = new map<int, ResDiscResult*>();
        query_stat[i] = res_disc_map;
        for(int j=0;j<3;j++){
          ResDiscResult* result = new ResDiscResult();
          (*res_disc_map)[j] = result;
        }
      }
      
      map<int,int>* node_list = p2p->GetNodeLists();
      P2PResInfoDhtUpdate* dht_avp = new P2PResInfoDhtUpdate(node_list);
//      AttrValuePair* avp = new AttrValuePair(p2p->GetNodeLists());
      dht_avp->AddAttribute(0, "./zipf_10");
      dht_avp->AddAttribute(1, "./zipf_100");
      dht_avp->AddAttribute(2, "./zipf_1000");
      dht_avp->UpdateDht();
      int res_update_period = 1;
      P2PAction* res_disc_act;
      map<int,int>::iterator node_it;
      P2PNodeFailure* failed_nodes = new P2PNodeFailure(p2p->GetNodeLists(), num_failed_nodes);   // for super-peer, it will be non-super-peer nodes
      P2PMessageDist* msg_dist = new P2PMessageDist((p2p->GetRoutingTable()), failed_nodes);

      spacing = num_node/target_run_index;
      for(node_it=node_list->begin();node_it!=node_list->end();node_it++){
        if(node_index++%spacing != 0){          
          continue;
        }
        map<string, GlobalClass*>* channel = new map<string, GlobalClass*>();
        P2PRdQuery* res_disc_query = new P2PRdQuery(dht_avp, query_mode, true, max_target_num);
        (*channel)["query"] = res_disc_query;
        if(query_mode == DHT){
          res_update_period = 300;
          res_disc_act = new DhtResDiscAct(channel, dht_avp->GetDhtResInfo(), failed_nodes);
          res_disc_query->CurHops = msg_dist->P2PSequentialCrawling(node_it->first, res_disc_query->AddrBegin, res_disc_query->AddrEnd, res_disc_act);          
  //        res_disc_query->CurHops = msg_dist->P2PTreeMulticast(node_it->first, res_disc_query->AddrBegin, res_disc_query->AddrEnd, res_disc_act, true);
          res_disc_query->TotalMsgs = res_disc_query->CurHops;
          delete res_disc_act;
        }else if((query_mode == OPTIMAL) || (query_mode == FIRST_FIT) || (query_mode == SUB_REGION)){
          res_update_period = 1;
          if(query_mode == SUB_REGION){
            res_disc_query->DetermineSubRegionQuery(2.0, node_it->first);
          }
          res_disc_act = new ResourceDiscoveryAct(channel, p2p->GetRoutingTable(), failed_nodes);
          res_disc_act->Execute(node_it->first);
          delete (ResourceDiscoveryAct*)res_disc_act;
        }else if(query_mode == FLOODING){
          int begin_node;
          bool superpeer;
          if(p2p_mode == 2){            
            res_update_period = 300;
            SuperPeerP2P* sp2p = (SuperPeerP2P*)p2p;            
            map<int,int>* sp_node_map = sp2p->GetSuperpeerMap();
            begin_node = (*sp_node_map)[node_it->first];
            map<int, map<int, multimap<int,int>*>*>* savn = dht_avp->UpdateToSuperpeers(sp_node_map);            
            res_disc_act = new SuperpeerResDiscAct(channel, savn, failed_nodes);
            PerformFlooding(msg_dist, res_disc_act, begin_node, ttl, true, sp_node_map);
            map<int, map<int, multimap<int,int>*>*>::iterator savn_it;
            for(savn_it=savn->begin();savn_it!=savn->end();savn_it++){
              map<int, multimap<int,int>*>::iterator sav_it;
              for(sav_it=savn_it->second->begin();sav_it != savn_it->second->end();sav_it++){
                delete sav_it->second;
              }
              delete savn_it->second;
            }
            delete savn;
          }else{                        
            res_update_period = 1;
            res_disc_act = new SingleResDiscAct(channel, failed_nodes);
            begin_node = node_it->first;
            PerformFlooding(msg_dist, res_disc_act, begin_node, ttl, false, NULL);
          }
          delete res_disc_act;
        }else if(query_mode == NODE_COUNT){
          if(failed_nodes.CheckIfFailed(node_it->first) == true){
            continue;
          }
          res_disc_act = new P2PNodeCountAct(channel, failed_nodes);
          map<int,int>* node_list = p2p->GetNodeLists();
          int depth = msg_dist->P2PTreeMulticast(node_it->first, 0, 0x7fffffff, res_disc_act, true);
          P2PStat* stat =(P2PStat*)((*channel)["result"]);
          cout << node_it->first << " : " << stat->MaxDepth<<endl;
          if(stat->TotalNodes != num_node){
            cout << "could not crawl all nodes : " << stat->TotalNodes << endl;
          }          
          delete res_disc_act;
        }
        if(query_mode != NODE_COUNT){
    //      int invalid_num = res_disc_query->CheckResultValidity(150, res_update_period);
          total_hops += res_disc_query->CurHops;
          run_index++;
    //      cout << node_it->first << "\t" << res_disc_query->CurHops <<" : " << res_disc_query->Result->size() << " : " << invalid_num << endl;
          if(res_disc_query->CurHops > 50){
            cout << "Attribute = " << res_disc_query->Attribute << " Begin = " <<res_disc_query->Begin << " End = " <<res_disc_query->End << " Target node = " << res_disc_query->Number << " actual size = " << res_disc_query->Result->size() << endl;
          }
          res_disc_query->CheckResultCorrectness(failed_nodes, query_stat);
        }
        delete res_disc_query;
        delete channel;
      }
      if(p2p_mode == 2){
        delete (SuperPeerP2P*)p2p;
      }else{
        delete p2p;
      }      
      double avg_hops = total_hops/(double)run_index;
      cout << num_node << "\t" << avg_hops <<endl;
      return 1;
    }else{
      cout << "usage: p2p_test num_node network_mode(0:symphony, 1: unstructured, 2:superpeer) query_mode(0:first_fit 1:sub_region 2:all region, 3:Dht, 4:flooding 5:node count) ttl_of_flooding number_of_failed_nodes" << endl;
    }
}    
#endif
