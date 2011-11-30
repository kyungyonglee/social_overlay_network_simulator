#include <iostream>
#include <sstream>
#include <fstream>
#include <cmath>
#include <string.h>

#include "p2p_network.h"
#include "p2p_message.h"
#include "son_util.h"
#include "p2p_global_data.h"


using namespace std;
using namespace Starsky;

void PerformFlooding(P2PMessageDist* msg_dist, P2PAction* res_disc_act, int begin_addr, int ttl, bool superpeer, map<int,int>* sp_map){
  map<string, GlobalClass*>* channel = res_disc_act->GetChannel();
  P2PRdQuery* res_disc_query = (P2PRdQuery*)((*channel)["query"]);
  map < int,int >* visited_nodes = new map<int,int>();
  int total_msg = msg_dist->P2PFlooding(begin_addr, begin_addr, ttl, visited_nodes,res_disc_act);
  map<int,int>* result = res_disc_query->Result;
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
    int greedy_rt_hops = msg_dist->P2PGreedyRouting(host_node, begin_addr, NULL);
    if(superpeer == true){
      greedy_rt_hops += 2;
    }
    total_msg += greedy_rt_hops;
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
  delete visited_nodes;
  cout << "unstructed query total message = " << total_msg << endl;
}

int main(int argc, char *argv[]) 
{
    P2PNetwork* p2p;
//    srand((unsigned)time(0));
    if(argc == 5){
      int num_node = atoi(argv[1]);
      int p2p_mode = atoi(argv[2]); 
      int query_mode = atoi(argv[3]);
      int ttl = atoi(argv[4]);
      switch(p2p_mode){
        case 0:
          p2p = new SymphonyP2P(num_node, (log(num_node)));
          break;
        case 1:        
          p2p = new UnstructuredP2P(num_node, (log(num_node)));
          break;
        case 2:
          p2p = new SuperPeerP2P(num_node, 100, 4);
          break;
        default:
          cout << "not available network mode (0:Symphony, 1: Unstructured 2: Superpeer)" << endl;
          return 1;
          break;
      }

 //     p2p->PrintRoutingTable();
//      p2p->CheckConnection();
      map<int,int>* node_list = p2p->GetNodeLists();
      P2PResInfoDhtUpdate* dht_avp = new P2PResInfoDhtUpdate(node_list);
//      AttrValuePair* avp = new AttrValuePair(p2p->GetNodeLists());
      dht_avp->AddAttribute(0, "./zipf_10");
      dht_avp->AddAttribute(1, "./zipf_100");
      dht_avp->AddAttribute(2, "./zipf_1000");
      dht_avp->UpdateDht();
      
      P2PAction* res_disc_act;
      map<int,int>::iterator node_it;
      P2PMessageDist* msg_dist = new P2PMessageDist((p2p->GetRoutingTable()));
      for(node_it=node_list->begin();node_it!=node_list->end();node_it++){
        map<string, GlobalClass*>* channel = new map<string, GlobalClass*>();
        P2PRdQuery* res_disc_query = new P2PRdQuery(dht_avp, query_mode, true);
        (*channel)["query"] = res_disc_query;
        if(query_mode == DHT){
          res_disc_act = new DhtResDiscAct(channel, dht_avp->GetDhtResInfo());
 //         res_disc_query->CurHops = msg_dist->P2PSequentialCrawling(node_it->first, res_disc_query->AddrBegin, res_disc_query->AddrEnd, res_disc_act);          
          res_disc_query->CurHops = msg_dist->P2PTreeMulticast(node_it->first, res_disc_query->AddrBegin, res_disc_query->AddrEnd, res_disc_act, true);
        }else if((query_mode == OPTIMAL) || (query_mode == FIRST_FIT) || (query_mode == SUB_REGION)){
          res_disc_act = new ResourceDiscoveryAct(channel, p2p->GetRoutingTable());
          res_disc_act->Execute(node_it->first);
        }else if(query_mode == FLOODING){
          int begin_node;
          bool superpeer;
          if(p2p_mode == 2){
            SuperPeerP2P* sp2p = (SuperPeerP2P*)p2p;            
            map<int,int>* sp_node_map = sp2p->GetSuperpeerMap();
            begin_node = (*sp_node_map)[node_it->first];
            map<int, map<int, multimap<int,int>*>*>* savn = dht_avp->UpdateToSuperpeers(sp_node_map);            
            res_disc_act = new SuperpeerResDiscAct(channel, savn);
            PerformFlooding(msg_dist, res_disc_act, begin_node, ttl, true, sp_node_map);
          }else{            
            res_disc_act = new SingleResDiscAct(channel);
            begin_node = node_it->first;
            PerformFlooding(msg_dist, res_disc_act, begin_node, ttl, false, NULL);
          }
        }
        cout << node_it->first << " : " << res_disc_query->CurHops << endl;
        if(res_disc_query->CurHops > 50){
          cout << "Attribute = " << res_disc_query->Attribute << " Begin = " <<res_disc_query->Begin << " End = " <<res_disc_query->End << " Target node = " << res_disc_query->Number << " actual size = " << res_disc_query->Result->size() << endl;
        }
        res_disc_query->CheckResultCorrectness();
        delete res_disc_query;
        delete channel;
        delete res_disc_act;
      }
      return 1;
//      dht_avp->CheckCumuMap();
//      dht_avp->UpdateDht();
      
//      dht_avp->CheckDhtValidity();
//      return 1;
/*
      P2PMessageDist msg_dist = P2PMessageDist((p2p->GetRoutingTable()));
      for(node_it=node_list->begin();node_it!=node_list->end();node_it++){
        map<string, GlobalClass*>* channel = new map<string, GlobalClass*>();
        P2PAction* action = new P2PNodeCountAct(channel);
        map<int,int>* node_list = p2p->GetNodeLists();
        int depth = msg_dist.P2PTreeMulticast(node_it->first, 0, 0x7fffffff, action, true);
        P2PStat* stat =(P2PStat*)((*channel)["result"]);
        if(stat->TotalNodes != num_node){
          cout << "could not crawl all nodes" << endl;
          return 1;
        }
        cout << depth << endl;
        delete stat;
        delete channel;
      }
      cout << endl;
*/      
      return 1;
    }else{
      cout << "usage: p2p_test num_node network_mode(0:symphony, 1: unstructured, 2:superpeer) query_mode(0:first_fit 1:sub_region 2:all region, 3:Dht, 4:flooding) ttl_of_flooding" << endl;
    }
}    


	
    
