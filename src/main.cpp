/*
 * main.cpp
 *
 *  Created on: 8 nov. 2013
 *      Author: jfellus
 */
#define MONOTHREAD
#define USE_MATRIX_DOUBLE

#include "common/math.h"
#include "common/multithread.h"
#include "common/utils.h"
#include "common/plot.h"
#include "common/gossip.h"
#include <vector>
#include <string>

using namespace std;


#include <time.h>
#include <unistd.h>
#include <sys/time.h>
static struct timeval ts;
bool tic(size_t ms) {
	struct timeval tv;
	gettimeofday(&tv, 0);
	float dt = (tv.tv_sec-ts.tv_sec)*1000 + 0.001*(tv.tv_usec-ts.tv_usec);
	if(dt > ms) {
		ts = tv;
		return true;
	}
	return false;
}



void gossip(int sender);

void dump_msg_between_partitions();


////////////
// PARAMS //
////////////

int n = get_config("n", 1000);
int K = get_config("K", 64);
int D = get_config("D", 2);
int LIMIT_NDATA = get_config("LIMIT_NDATA", -1);

int N = get_config("N", 100);
int M = get_config("M", N);

int MAX_AVG_PARTITIONS = get_config("MAX_AVG_PARTITIONS", 50);
int T_MAX = get_config("T_MAX", N*M*MAX_AVG_PARTITIONS);

int NB_TICKS_PLOT = get_config("NB_TICKS_PLOT", 1000);

string DATAFILE = get_config_str("DATAFILE", "");
string DATAGEN = get_config_str("DATAGEN", "");

int RANDOM_SEED = get_config("RANDOM_SEED", -1);


//////////
// DATA //
//////////

Matrix X;

int t = 0;
float rt = 0;

long total_msg_size = 0;

//////////////////////




int __node_last_id = 0;

class Node;
Node* cur_partitioning_node = NULL;
Matrix* tmpS;
Matrix* tmpW;

bool consensus_changed = false;
int msg_between_partitions = 0;



Matrix consensus;
Matrix sum_w;


//////////
// NODE //
//////////

class Node {
public:
	int id;

	Matrix X;
	int n;

	int M;

	Matrix mu;
	Matrix S;
	Matrix w;

	Matrix oldS, oldW;
	Matrix newS, newW;

	float rate_com, rate_partition;
	float nextEvtTime;

	int m;

	int nb_update_mu, nb_evt_partition, nb_evt_send, nb_evt_receive;

	Node() {this->id = __node_last_id++;n=0;w=0.0;}
	void init(Matrix& X, int first, int n) {
		this->n = n;
		this->X.create_ref(X.get_row(first),D,n);

		mu.create(D,K);
		oldmu.create(D,K);

		S.create(D,K);
		w.create(K,1);
		oldS.create(D,K);
		oldW.create(K,1);
		newS.create(D,K);
		newW.create(K,1);

		nb_update_mu = 0;
		S = 0.0; w = 0.0;
		oldS=oldW=0.0;
		this->M = ::M;
		m = 0;

		nb_evt_partition = nb_evt_send = nb_evt_receive = 0;
		rate_com = rate_partition = 1;

		random_partition();
	}

	~Node() {}

	////////////////////////

	void event() {
		if(m>=M) {
			m=0;
			local_partition();
//			nextEvtTime = rt + rand_exp(rate_partition);

//			dump_msg_between_partitions();
		}
		else {
			m++;
			gossip(id);
//			nextEvtTime = rt + rand_exp(rate_com);

//			msg_between_partitions++;
		}
	}


	// 1) LOCAL PARTITIONING

	void local_partition() {
	//	dump_codebook();

		nb_evt_partition++;
	//	 fappend(fmt("data/stats/evt/nb_evts_partition.txt"), fmt("%f %u\n", rt, id));

		 newS = newW = 0.0;

#ifdef MONOTHREAD
		for(int i=0; i<n; i++) {
			double* x = X.get_row(i);
			int k = mu.nearest_neighbor(x);
			newS.row_add(k,x);
			newW[k]++;
		}
#else
		cur_partitioning_node = this;
		local_partitioning_mt(n);
		for(int i=0; i<NBTHREADS; i++) {
			oldS += tmpS[i]; oldW += tmpW[i];
		}
#endif

		float d1 = newS.l2(oldS);
		float d2 = newW.l2(oldW);
//		fappend("data/stats/delta_partitioning.txt", fmt("%f %f\n", rt, d1+d2));
//		fappend("data/stats/REC_before_partitioning.txt", fmt("%f %f\n", rt, consensus.l2(mu)));
		if(d1>0.001 || d2 > 0.01){
			S += newS;
			w += newW;
			S -= oldS;
			w -= oldW;
			oldS = newS; oldW = newW;
		}

		consensus_changed = true;
		update_mu();
	}



	// 2) GOSSIP CODEBOOK UPDATES

	int send(Node& node) {
//		nb_evt_send++;
	//	fappend(fmt("data/stats/evt/nb_evts_send.txt"), fmt("%f %u\n", rt, id));

		w /= 2; 	S /= 2;

		node.receive(S,w);
		return K*(D+1);
	}

	void receive(Matrix& S2, Matrix& w2) {
		w += w2; S += S2;

//		nb_evt_receive++;
	//	fappend("data/stats/evt/nb_evts_receive.txt", fmt("%f %u\n", rt, id));
		update_mu();
	}




	///////////////////

	Matrix oldmu;
	void update_mu() {
		oldmu = mu;
		mu = S;
		for(int k=0; k<K; k++)	mu.row_sdiv(k,w[k]);

		if(tic(1000)) DBG("Node" << id << " : " << oldmu.l2(mu));

		//dump_codebook();
	}


	void local_partition_mt(int th, int first, int n);

	void random_partition() {
		nextEvtTime = rt + frand(0,M*rate_com);

		newS = newW = 0.0;
		for(int i=0; i<n; i++) {
			double* x = X.get_row(i);
			int k =	rand()%K;
			newS.row_add(k,x);
			newW[k]++;
		}

		oldS = newS; oldW = newW;
		S = newS; w = newW;

		nb_evt_partition++;
	//	fappend("data/stats/evt/nb_evts_partition.txt", fmt("%f %u\n", rt, id));
		consensus_changed = true;
		update_mu();
		//dump_codebook();
		nextEvtTime = rt + rand_exp(rate_partition);
	}

	void dump_codebook() {
		system(fmt("mkdir -p data/codebooks/%u", id));
		mu.save(fmt("data/codebooks/%u/codebook_%u.txt", id, nb_update_mu++));
	}

};

Node* node;
#include "misc.cpp"


__multithread_3__(local_partitioning_mt) (int th, int first, int n) {
	cur_partitioning_node->local_partition_mt(th, first,n);
}

void Node::local_partition_mt(int th, int first, int n) {
	tmpS[th]=tmpW[th]=0.0;

	for(int i=first; i<n; i++) {
		double* x = X.get_row(i);
		int k = mu.nearest_neighbor(x);
		tmpS[th].row_add(k,x);
		tmpW[th][k]++;
	}
}



void gossip(int sender) {
	total_msg_size += node[sender].send(node[gossip_choose_receiver(sender)]);

	//fappend("data/stats/total_msg_size.txt",fmt("%f %ld\n",rt, total_msg_size));
	//fappend("data/stats/total_msg_size_N.txt",fmt("%f %f\n",(float)rt, (float)total_msg_size/N));
}



/////////////////////




//////////
// DUMP //
//////////

int nb_consensus_update = 0;

void dump_consensus() {
	system(fmt("mkdir -p data/codebooks/consensus"));
	consensus.save(fmt("data/codebooks/consensus/codebook_%u.txt", nb_consensus_update++));
	consensus_changed = false;
}


void compute_consensus() {
	consensus = 0.0;
	sum_w = 0.0;
	for(int i=0; i<N; i++) {
		consensus += node[i].S;
		sum_w += node[i].w;
	}

//	if(sum_w.min() < 0) throw "<0";

	for(int k=0; k<K; k++) consensus.row_sdiv(k, sum_w[k]);

	//if(consensus_changed) dump_consensus();
}




void compute_errors() {
	DBG("J1=");
	compute_consensus();

//	// J2(t) : Error to Consensus
//	float J2 = 0;
//	for(int i=0; i<N; i++) {
//		for(int k=0; k<K; k++) {
//		//	if(sum_w[k]!=0)
//				J2 += sum_w[k] *
//						vector_l2p2_double(consensus.get_row(k), node[i].mu.get_row(k), D);
//		}
//	}
//	J2 /= N*K;
//
//	DBG("J2="<<J2);

	// J1(t) : MQE of consensus
	float J1 = 0;
	for(int i=0; i<n; i++) {
		double* x = X.get_row(i);
		J1 += consensus.nearest_neighbor_distance(x);
	}
	J1 /= n;


	fappend("data/stats/J1_msg.txt", fmt("%u %f\n", t, J1));
//	fappend("data/stats/J2_msg.txt", fmt("%u %f\n", t, J2));
	fappend("data/stats/J1.txt", fmt("%f %f\n", rt, J1));
//	fappend("data/stats/J2.txt", fmt("%f %f\n", rt, J2));

	DBG("J1=" << J1);
}



void dump_nb_evts() {
	for(int i=0; i<N; i++) {
		fappend("data/stats/nb_evts_partition.txt", fmt("%u\n", node[i].nb_evt_partition));
		fappend("data/stats/nb_evts_send.txt", fmt("%u\n", node[i].nb_evt_send));
		fappend("data/stats/nb_evts_receive.txt", fmt("%u\n", node[i].nb_evt_receive));
	}
}

void dump_msg_between_partitions() {
	if(msg_between_partitions<N)
		fappend("data/stats/nb_msg_between_partition.txt", fmt("%f %u\n", rt, msg_between_partitions));
	msg_between_partitions=0;
}









//////////////////////////////


void simulate_async() {
	int evtNode = -1;
	for(t=0; t<T_MAX; t++) {
		rt = node[0].nextEvtTime;
		for(int i=0; i<N; i++) if(rt >= node[i].nextEvtTime) {evtNode = i; rt = node[i].nextEvtTime;}
		node[evtNode].event();

		if((t % (T_MAX/NB_TICKS_PLOT))==0)
		{
			DBG_PERCENT(t, T_MAX);
			compute_errors();
		}
	}
}

void simulate_sync() {
	int mn = 0;
	for(t=0; t<T_MAX; t++) {
		if(mn >= M*N) {
			for(int i=0; i<N; i++) node[i].local_partition();
			mn = 0;
		} else {
			gossip(gossip_choose_sender());
			mn++;
		}

		if((t % (T_MAX/NB_TICKS_PLOT))==0)
		{
			DBG_PERCENT(t, T_MAX);
			compute_errors();
		}

		rt++;
	}
}




void simulate() {
	gettimeofday(&ts, 0);
	for(t=0; t<T_MAX; t++) {
		int i = gossip_choose_sender();
		node[i].event();
		if(tic(1000)) DBG(t);
	}
	compute_errors();
}







int main(int argc, char **argv) {
	try {
	DBG_START("Init ");
	init();
	if(DATAGEN.empty()) {
		load_data(DATAFILE.c_str());
		char ss[255]; strcpy(ss,DATAFILE.c_str());
	//	chdir(dirname(ss));
	} else {
		generate_data(DATAGEN.c_str());
		X.save("data/x.txt");
	}



	consensus.create(D,K);
	sum_w.create(K,1);
	tmpS = new Matrix[N];
	tmpW = new Matrix[N];
	for(int i=0; i<N; i++) {
		tmpS[i].create(D,K);
		tmpW[i].create(K,1);
	}

	DBG_END();


	////////////////////////

	simulate();

	//////////////////////

	DBG("finished");

	dump_nb_evts();
	}catch(const char* c) {DBG("ERROR : " << c );}
}
