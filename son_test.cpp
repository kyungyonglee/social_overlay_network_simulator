#include <iostream>
#include <sstream>
#include <fstream>
#include <cmath>
#include <string.h>

#include "son_friend_select.h"
#include "son_msg_dist.h"
#include "son_routing.h"
#include "son_util.h"


using namespace std;
using namespace Starsky;

#if 0
void printInfo(map<int, pair<double, double> > result) {
	ofstream myfile("output1");
	myfile << "#nodes: " << "\t" << "hit rate" << "\t" << "ave msgs" << "\t" << "hops/hit_rate" << endl;
	map<int,pair<double, double> >::iterator it;
	for (it=result.begin(); it!=result.end(); it++) {
		double hops_ps = it->second.second / it->second.first;
		myfile << it->first << "\t" << it->second.first << "\t" << it->second.second << "\t" << hops_ps << endl;
	}
}
//random string generator
std::set<std::string> rstringGenerator ( int howmany, int length, Ran1Random& r )
{
    std::set<std::string> items;
    for (int no=0; no < howmany; no++)
    {
	std::string item;
	for (int i = 0; i < length; i++)
	{
            int rand_no = (int) (r.getDouble01() * 122);
	    if ( rand_no < 65 ) { rand_no = 65 + rand_no % 56;}
	    if ( (rand_no > 90) && (rand_no < 97) ) { rand_no += 6; }
	    item += (char)rand_no;		  
	}
	items.insert(item);
    }
    return items;
}

double CalculateCloseness(map<int, map<int,char>* >* input){
  map<int, map<int,char>* >::iterator it;
  int total_score = 0, max_friends=0, min_friends = 1000000, total = 0, theory_count = 0, score = 0, per_node_score = 0, per_node_run = 0;
  double sum_per_node_score = 0.0;
  for(it=input->begin(); it != input->end(); it++){
    per_node_score = 0;
    per_node_run = 0;
    cout << it->first << endl;
    int location_index = 1;
    map<int,char> *f_lists = it->second;
    if(f_lists == NULL){
      continue;
    }
    int size = f_lists->size();
    max_friends = size > max_friends ? size:max_friends;
    continue;
    total += size;
    theory_count += (size*(size-1)/2);
    max_friends = f_lists->size() > max_friends ? f_lists->size() : max_friends;
    min_friends = f_lists->size() < min_friends ? f_lists->size() : min_friends;
    map<int, char >::iterator ff_lists;
    for(ff_lists = f_lists->begin(); ff_lists != f_lists->end(); ff_lists++){
      if(input->count(ff_lists->first) == 0){
        continue;
      }
      map<int, char> *target_map = (*input)[ff_lists->first];
      if (target_map == NULL){
        continue;
      }
      map<int, char>::iterator self_lists = f_lists->begin();
      advance(self_lists, location_index);
      for(;self_lists!=f_lists->end();self_lists++){
        if(target_map->count(self_lists->first) > 0){
  //        cout << "self = " <<it->first << " other = " << other_list[j] << endl;
          score++;
          per_node_score++;
        }
        total_score++;
        per_node_run++;
      }
      location_index++;
    }
    if(per_node_run != 0){
      sum_per_node_score += ((double)per_node_score/(double)per_node_run);
    }
//    cout << "sum_per_node_score = " << sum_per_node_score <<" per_node_score  = " << per_node_score << " per_node_run = "<<per_node_run << endl;
  }
  cout << "max_friends " << max_friends << endl;
  double point = (double)score/(double)total_score;
  double avg = (double)total/input->size();
  double per_node_avg = (double)sum_per_node_score/input->size();
  cout << "total number of friends = " << total << " minimum friends = " << min_friends << " maximum friends = " << max_friends << " average friends = " << avg << endl;
  cout << "total score = " << total_score << " match score = " << score << " point = " << point << endl;
  cout << "theory count = " <<theory_count << endl;
  cout << "per node point = " << per_node_avg << " size = " << input->size()  << " sum_per_node_score = "<< sum_per_node_score << endl;
  return point;
}

double CalculateAssortativity(map<int, map<int,char>* >* input){
  map<int, map<int,char>* >::iterator it;
  map<int, int> incoming_count;
  long long total_num_edge = 0, term1=0, term2=0, term3=0, temp4=0, n1_sum=0, n2_sum = 0, n1_sq_sum=0, n2_sq_sum=0, n1_avg=0, n2_avg=0;
  for(it=input->begin(); it != input->end(); it++){
  //   cout << it->first << endl;
    map<int, char>*f_lists = it->second;
    int n1_friends = (f_lists->size());
    map<int,char>::iterator f_list_it;
    n1_sum += n1_friends;
    for(f_list_it=f_lists->begin();f_list_it != f_lists->end();f_list_it++){
      if(incoming_count.count(f_list_it->first) != 0){
        int c_count = incoming_count[f_list_it->first];
        c_count++;
        incoming_count[f_list_it->first] = c_count;
      }else{
        incoming_count[f_list_it->first] = 1;
      }
    }
  }

  map<int, int>::iterator n2_it;
  for(n2_it = incoming_count.begin();n2_it!=incoming_count.end();n2_it++){
    n2_sum += n2_it->second;
  }
  n1_avg = n1_sum/input->size();
  n2_avg = n2_sum/incoming_count.size();
//  n2_avg = n2_sum/total_num_edge;
  cout << "n1 avg = " << n1_avg << " n2 avg = " << n2_avg << endl;

  for(it=input->begin(); it != input->end(); it++){
 //   cout << it->first << endl;
    map<int, char> *f_lists = it->second;
    long long n1_friends = (f_lists->size());
    map<int, char>::iterator ff_lists;
    for(ff_lists = f_lists->begin(); ff_lists != f_lists->end(); ff_lists++){
      if(input->count(ff_lists->first) != 0){
        long long n2_friends = ((*input)[ff_lists->first]->size());
        term1+= ((n1_friends-n1_avg)*(n2_friends-n2_avg));
        term2+= ((n1_friends-n1_avg)*(n1_friends-n1_avg));
        term3+= ((n2_friends-n2_avg)*(n2_friends-n2_avg));
        if(term1 <0 || term2 <0 || term3 <0){
          cout << "term1 = " << term1 << " term2 = " << term2 << " term3 = " << term3 << endl;
          cout << "(n1_friends-n1_avg) = " << (n1_friends-n1_avg) << " (n2_friends-n2_avg) : " <<(n2_friends-n2_avg) << " n1_friends = " << n1_friends << " n2_friends = " << n2_friends <<endl;
        }
      }
    }
  }
  
  
  int sqrt_term3 = sqrt(term2)*sqrt(term3);
  cout << "term1 = " << term1 << " term2 = " << term2 << " term3 = " << term3 << " sqrt:term3 " << sqrt_term3 << endl;
  double assortative = (double)term1/(double)sqrt_term3;
  cout << "total num edge = " << total_num_edge << " assortative value = " << assortative << endl;
  return assortative;
}
/*
double CalculateAssortativity(map<int, map<int,char>* >* input){
  map<int, map<int,char>* >::iterator it;
  long long total_num_edge = 0, term1=0, term2=0, term3=0, temp4=0, n1_sum=0, n2_sum = 0, n1_sq_sum=0, n2_sq_sum=0;
  for(it=input->begin(); it != input->end(); it++){
 //   cout << it->first << endl;
    map<int, char>*f_lists = it->second;
    long n1_friends = (f_lists->size() -1);
    map<int,char>::iterator ff_lists;
    for(ff_lists = f_lists->begin(); ff_lists != f_lists->end(); ff_lists++){
      if(input->count(ff_lists->first) != 0){
        total_num_edge++;
        long long n2_friends = ((*input)[ff_lists->first]->size() -1);
        term1 += (n1_friends*n2_friends);
        n1_sum += n1_friends;
        n2_sum += n2_friends;
        n1_sq_sum += (n1_friends*n1_friends);
        n2_sq_sum += (n2_friends*n2_friends);
        if(term1 < 0 || n1_sq_sum < 0 || n2_sq_sum < 0){
          cout << "term1 " <<term1 << " n1_sq_sum : " <<n1_sq_sum << " n2_sq_sum : " << n2_sq_sum <<endl;
        }        
      }
    }
  }
  term2 = (n2_sum/total_num_edge)*n1_sum;
  term3 = (n1_sq_sum - (n1_sum*(n1_sum/total_num_edge)))*(n2_sq_sum-(n2_sum*(n2_sum/total_num_edge)));
  
  int sqrt_term3 = sqrt(term3);
  cout << "term1 = " << term1 << " term2 = " << term2 << " term3 = " << term3 << " sqrt:term3 " << sqrt_term3 << " n1_sum = " << n1_sum<< " n2_sum = " << n2_sum << " n1_sq_sum : " << n1_sq_sum << " n2_sq_sum : " << n2_sq_sum << endl;
  double assortative = (double)(term1-term2)/(double)sqrt(term3);
  cout << "total num edge = " << total_num_edge << " assortative value = " << assortative << endl;
  return assortative;
}
*/
/*
double CalculateAssortativity(map<int, map<int,char>* >* input){
  map<int, map<int,char>* >::iterator it;
  double total_num_edge = 0, term1=0, term2=0, term3=0;
  for(it=input->begin(); it != input->end(); it++){
//    cout << it->first << endl;
    map<int,char> *f_lists = it->second;
    int n1_friends = f_lists->size();
    map<int, char >::iterator ff_lists;
    for(ff_lists = f_lists->begin(); ff_lists != f_lists->end(); ff_lists++){
      if(ff_lists->first < it->first && input->count(ff_lists->first) != 0){
        total_num_edge++;
        int n2_friends = (*input)[ff_lists->first]->size();
        term1 += n1_friends*n2_friends;
        term2 += (0.5*(n1_friends+n2_friends));
        term3 += (0.5*(n1_friends*n1_friends+n2_friends*n2_friends));
      }
    }
  }  
  double term2_final = ((double)term2/(double)total_num_edge);
  double assortative =( (double)term1/(double)total_num_edge - term2_final*term2_final)/((double)term3/(double)total_num_edge - term2_final*term2_final);
  cout << "total num edge = " << total_num_edge << " assortative value = " << assortative << endl;
  return assortative;
}

*/

void CreateHistogram(map<int, map<int,char>* >* input){
  map<int, map<int,char>* >::iterator it;
  map<int, int> incoming_count;
  map <int, int> neighbor_number_map;
  for(int i=1;i<1000;i++){
    neighbor_number_map[i*100] = 0;
  }
  long max_friends = 0, prev_num = 0;
  for(it=input->begin(); it != input->end(); it++){
    max_friends = max_friends > it->second->size() ? max_friends : it->second->size();
    prev_num = neighbor_number_map[(1+it->second->size()/100)*100];
    neighbor_number_map[(1+it->second->size()/100)*100] = prev_num + 1;
  }

  map<int,int>::iterator nnm_it;
  for(nnm_it=neighbor_number_map.begin();nnm_it != neighbor_number_map.end();nnm_it++){
    if(nnm_it->second != 0){
      cout << nnm_it->first << " : " << endl;
    }
  }
}

void WriteToFile(map<int, map<int,char>* >* input){
  ofstream outfile;
  outfile.open("flickt_diff_data");
  map<int, map<int,char>* >::iterator it;
  string buf;
  for(it=input->begin(); it != input->end(); it++){
    stringstream ss;
    ss << it->first;
    buf = ss.str();
    map<int,char> *f_lists = it->second;
    map<int, char >::iterator ff_lists;
    for(ff_lists = f_lists->begin(); ff_lists != f_lists->end(); ff_lists++){
      stringstream temp;
      temp << ff_lists->first;
      buf += "\t";
      buf += temp.str();
    }    
    buf += "\n";
    outfile << buf;
  }  
  outfile.close();
}

void FileParse(char* filename){
  string line;
  ifstream file(filename);
  map<int, map<int,AddressedNode*>* >* list_parse = new map<int, map<int,AddressedNode*>* >();
  if(file.is_open()){
    int self_id = -1, friend_id=-1;
    map<int, AddressedNode*>* table;
    while(file.good()){
      getline(file, line);
      self_id = -1;
      char* pch;
      pch = strtok((char*)line.c_str(), "\t");
      while(pch != NULL){        
        if(self_id < 0){
          self_id = atoi(pch);
        }else{
          friend_id = atoi(pch);
          if(list_parse->count(self_id) == 0){
            table = new map<int, AddressedNode*>();
            list_parse->insert(pair<int, map<int, AddressedNode*>* >((int)self_id, table));
          }else{
            table = (*list_parse)[self_id];
          }
          table->insert(pair<int, AddressedNode*>((int)friend_id, NULL));
        }
        pch = strtok(NULL, "\t");
      }
    }
    file.close();
  }
//  FriendsNetwork::Clustering(list_parse);
//  WriteToFile(list_parse);
//  CalculateAssortativity(list_parse);
  CalculateCloseness(list_parse);
//  CreateHistogram(list_parse);
}
#endif

void KcMsgTest(SonMsgDist* son_msg, SonFriendSelect* sfs){
  map<int, map<int, int>* >* friends_map = sfs->GetFriendsMap();
  map<int, map<int, int>* >::iterator fm_it;
  map<int, SonStatistics*> routing_hops_stat;
  for(fm_it = friends_map->begin();fm_it != friends_map->end();fm_it++){
    map<int, int>* msg_recipients = new map<int, int>();
    map<int, int>::iterator fit;
    int friend_count = fm_it->second->size();
    for(fit = fm_it->second->begin();fit != fm_it->second->end();fit++){
      msg_recipients->insert(pair<int,int>(fit->first,-1));
    }
    int total_msg = son_msg->FloodDeliever(fm_it->first, msg_recipients,0, -1);
    for(fit = msg_recipients->begin(); fit != msg_recipients->end();fit++){
      if (fit->second < 0){
        cout << "root = " << fm_it->first << " neighbor size = " << friend_count << " neighbor = " << fit->first << " : " <<((*friends_map)[fit->first])->size()<<endl;
      }
    }
    SonStatistics* son_stat_rt;
    if(routing_hops_stat.count(friend_count) != 0){
      son_stat_rt = routing_hops_stat[friend_count];
    }else{
      son_stat_rt = new SonStatistics(friend_count);
      routing_hops_stat[friend_count] = son_stat_rt;
    }
    son_stat_rt->UpdateStat(msg_recipients);
 //   cout << "total generated message = " << total_msg << endl;
    delete msg_recipients;
    son_msg->UpdateMsgOverhead(friend_count, total_msg);
  }
  map<int, SonStatistics*>::iterator ss_it;
  map<int, SonStatistics*>* summ_result = SonUtil::SummarizeStat(&routing_hops_stat);
  cout << endl << endl;
  for(ss_it=summ_result->begin();ss_it != summ_result->end();ss_it++){
    ss_it->second->PrintStat();
  }
  delete summ_result;
  cout << endl << endl;
  son_msg->PrintMsgOverhead();
}

void PoMsgTest(SonMsgDist* son_msg, SonFriendSelect* sfs, map<int,int>* po_create_nodes, map<int, map<int,int>* >* po_join_map){
  map<int, map<int, int>* >* friends_map = sfs->GetFriendsMap();
  map<int, map<int, int>* >::iterator fm_it;
  map<int, SonStatistics*> routing_hops_stat;
  map<int, SonStatistics*> fwd_overhead_stat;
  for(fm_it = friends_map->begin();fm_it != friends_map->end();fm_it++){
    map<int, int>* msg_recipients = new map<int, int>();
    map<int, int>::iterator fit;
    int friend_count = fm_it->second->size();
    for(fit = fm_it->second->begin();fit != fm_it->second->end();fit++){
      msg_recipients->insert(pair<int,int>(fit->first,-1));
    }
    int total_msg = 0;
    if(po_create_nodes->count(fm_it->first) != 0){  //private overlay nodes
      total_msg = son_msg->MulticastDeliever(fm_it->first, msg_recipients, msg_recipients,0);
    }else{  //no private overlay nodes
      total_msg = son_msg->FloodDeliever(fm_it->first, msg_recipients, 0, 2);
    }
    for(fit = msg_recipients->begin(); fit != msg_recipients->end();fit++){
      if (fit->second < 0){
        int hops = son_msg->GreedyDeliever(fm_it->first, fit->first, 1);
        total_msg += hops;
        (*msg_recipients)[fit->first] = hops;
      }
    }

    SonStatistics* son_stat_rt;
    if(routing_hops_stat.count(friend_count) != 0){
      son_stat_rt = routing_hops_stat[friend_count];
    }else{
      son_stat_rt = new SonStatistics(friend_count);
      routing_hops_stat[friend_count] = son_stat_rt;
    }
    son_stat_rt->UpdateStat(msg_recipients);
 //   cout << "total generated message = " << total_msg << endl;
    delete msg_recipients;
    son_msg->UpdateMsgOverhead(friend_count, total_msg);
  }
  map<int, SonStatistics*>::iterator ss_it;
  map<int, SonStatistics*>* summ_result = SonUtil::SummarizeStat(&routing_hops_stat);
  cout << endl << endl;
  for(ss_it=summ_result->begin();ss_it != summ_result->end();ss_it++){
    ss_it->second->PrintStat();
  }
  delete summ_result;
  cout << endl<< endl;
  son_msg->PrintMsgOverhead();
}

void MulticastMsgTest(SonMsgDist* son_msg, SonFriendSelect* sfs){
  map<int, map<int, int>* >* friends_map = sfs->GetFriendsMap();
  map<int, map<int, int>* >::iterator fm_it;
  map<int, SonStatistics*> routing_hops_stat;
  for(fm_it = friends_map->begin();fm_it != friends_map->end();fm_it++){
    map<int, int>* msg_recipients = new map<int, int>();
    map<int, int>::iterator fit;
    int friend_count = fm_it->second->size();
    for(fit = fm_it->second->begin();fit != fm_it->second->end();fit++){
      msg_recipients->insert(pair<int,int>(fit->first,-1));
    }
    int total_msg = son_msg->MulticastDeliever(fm_it->first, msg_recipients, msg_recipients,0);
//    cout << "root = " << fm_it->first << " neighbor number = " << fm_it->second->size()<<endl;
    if(friend_count > 100 && total_msg != msg_recipients->size()){
//      cout << "hops taken = " << total_msg << " msg recipients size = " << msg_recipients->size() << " root = " << fm_it->first<<endl;
    }
    for(fit = msg_recipients->begin(); fit != msg_recipients->end();fit++){
      if ( fm_it->second->size() > 100 &&  fit->second < 0){
//        cout << "root = " << fm_it->first << " neighbor size = " << fm_it->second->size() << " neighbor = " << fit->first << " : " <<((*friends_map)[fit->first])->size()<<endl;
      }
    }
    SonStatistics* son_stat_rt;
    if(routing_hops_stat.count(friend_count) != 0){
      son_stat_rt = routing_hops_stat[friend_count];
    }else{
      son_stat_rt = new SonStatistics(friend_count);
      routing_hops_stat[friend_count] = son_stat_rt;
    }
    son_stat_rt->UpdateStat(msg_recipients);
 //   cout << "total generated message = " << total_msg << endl;
    delete msg_recipients;
    son_msg->UpdateMsgOverhead(friend_count, total_msg);
  }
  map<int, SonStatistics*>::iterator ss_it;
  map<int, SonStatistics*>* summ_result = SonUtil::SummarizeStat(&routing_hops_stat);
  cout << endl << endl;
  for(ss_it=summ_result->begin();ss_it != summ_result->end();ss_it++){
    ss_it->second->PrintStat();
  }
  delete summ_result;
  cout << endl << endl;
  son_msg->PrintMsgOverhead();
}



int main(int argc, char *argv[]) 
{
    SonFriendSelect* sfs;
    SonRouting* son_routing;
    SonMsgDist* son_messaging;
//srand((unsigned)time(0));
    if(argc == 4){
      string filename = argv[1];
      int route_mode = atoi(argv[2]);
      int msg_mode = atoi(argv[3]);
      string freq_name = filename + ".freq";
      sfs = new SonFriendFromFile(filename);
//      sfs->CalculateCloseness();
//      sfs->WriteToFile(freq_name);
//      sfs->WriteCompleteFriendMap("orkut_all.complete");
//      return 1;
      sfs->BuildCommonFriendsFreq(freq_name);
//      sfs->PrintFriendMap();
//      return 1;
      if (route_mode == 0){
        son_routing = new SonKCoverageRouting(sfs);
      }else if (route_mode == 1){
        son_routing = new SonPrivateOverlayRouting(sfs);
      }else if (route_mode == 2){
        son_routing = new SonClusterRouting(sfs);
      }
      son_routing->BuildRoutingTable();
      son_routing->GetRoutingTableStat();
  //    son_routing->PrintRoutingTable();
      son_messaging = new SonMsgDist(son_routing);
      if(route_mode == 0){
        KcMsgTest(son_messaging, sfs);
      }else if(route_mode == 1){
        SonPrivateOverlayRouting* temp_po_son = (SonPrivateOverlayRouting*)son_routing;
        PoMsgTest(son_messaging, sfs, temp_po_son->GetPoCreateNodes(), temp_po_son->GetJoinablePoMap());
      }
      return 1;
    }else if (argc != 12) {
	    cerr << "Usage: " << argv[0] << ", number of nodes, number of friends,  cohesiveness, mode(uni:0, multi:1), friends_connection(no conn:0, with conn:1), friend connection select method(0:random, 1:harmony, 2:common friends), use only friends (0:false, 1:true), friends select method(0:random, 1:nearest-neighbor), use lookaside buffer(0,1), number of random friends, number of bucket" << endl;
        return 1;
    }
 /*   
    int nodes = atoi(argv[1]);
    int num_neighbor = atoi(argv[2]);
    float cohesiveness = atof(argv[3]);
    int mode = atoi(argv[4]);
    int friends_conn = atoi(argv[5]);
    int friends_mode = atoi(argv[6]);
    bool f_mode = friends_mode == 0 ? false:true;
    int use_friends = atoi(argv[7]);
    int friends_select_method = atoi(argv[8]);
    bool uf = use_friends == 0 ? false:true;
    int use_lookaside = atoi(argv[9]);
    int num_random_friends = atoi(argv[10]);
    int num_bucket = atoi(argv[11]);
*/    
#if 1
    
#else
    map<int, pair<double, double> > result;
    int sum_c_nodes=0, sum_c_edges=0, sum_c_in_depth=0, sum_c_depth=0;
	auto_ptr<FriendsNetwork> network_ptr ( new FriendsNetwork(nodes, ran_no, friends_mode, friends_select_method, NULL) );
    FriendsNetwork* tree_net = network_ptr.get();
//    cout << "bgin creating net" <<endl;
	tree_net->createEvenNet(nodes);
    cout << "begine determine friends" << endl;
    if(friends_select_method == 0){
      tree_net->DetermineFriends(num_neighbor, cohesiveness, f_mode);
    }else if(friends_select_method == 1){
      tree_net->MakeNNFriends(num_neighbor, cohesiveness, f_mode, num_random_friends, num_bucket);
    }
    cout << "calculate closeness" <<endl;
    tree_net->CalculatePowerLawCoeff();
    tree_net->CalculateAssortativity();
    
    double current_cc = tree_net->CalculateCloseness();
    cout << "original cc = " << current_cc <<endl;
    if(friends_conn == 1){
      FriendsNetwork::Clustering(tree_net->GetFriendsMap());
//      tree_net->MakeFriendsConnection(friends_mode);
    }else if(friends_conn == 0){
      tree_net->makeShortcutConnection(&(tree_net->node_map), true);
    }
    return 1;
    tree_net->BuildRoutingTable();
    tree_net->InspectRoutingTable();
//    return 1;
//    network_ptr->printNetInfo(true);
/*   random node selction
    UniformNodeSelector item_src(ran_no);
    item_src.selectFrom(network_ptr.get());
    AddressedNode* item_source = dynamic_cast<AddressedNode*> (item_src.select());
*/    

    auto_ptr<SymphonyMessage> message_ptr ( new SymphonyMessage(ran_no, num_neighbor, cohesiveness, mode, uf, tree_net, (use_lookaside==1)));
//    cout << "begin msg " <<endl;
    auto_ptr<NodeIterator> ni( tree_net->getNodeIterator() );
    int run_index = 0, no_run_index = 0;
    while(ni->moveNext() ) {
      AddressedNode* nodei = dynamic_cast<AddressedNode*> (ni->current());
      Network* ret = message_ptr->visit(nodei, *tree_net);
      if(ret != NULL){
        no_run_index++;
      }else{
        run_index++;
      }
//      cout << run_index <<endl;
    }    
    int average_max_hops = (message_ptr->GetTotalMaxHops()/run_index);
    int average_hops = message_ptr->GetTotalHops() / run_index;
    int min_max_hops = message_ptr->GetMinMaxHops();
    cout << "average hops = " << average_hops << " max hops = " << message_ptr->GetMaxHops() << " max average hops = " << average_max_hops << " minimum max hops = " << min_max_hops << endl;
    cout << "no run index = " << no_run_index << " run index = " << run_index << endl;
//    cout << "end msg" <<endl;
#endif
}    


	
    
