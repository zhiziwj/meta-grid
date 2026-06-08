#include "core.h"
#include "rand.h"
#include "backtest.h"
std::vector<kline> load_data(std::string filename){
    std::vector<kline> ans;
    std::fstream data;
    std::string str;
    double o,h,l,c;
    data.open(filename);
    if(!data.is_open()){
        std::cerr<<"Error: cannot open file "<<filename<<std::endl;
        return ans;
    }
    while(data>>str>>o>>h>>l>>c){
        ans.push_back({str,o,h,l,c});
    }
    data.close();
    return ans;
}
SA_state SA(std::vector<kline> data,const std::vector<double>& bases){
    std::srand(time(0));
    SA_state cur_state,best_ans;
    for(int i=0;i<state;i++){
        cur_state.t[i]=0.2;
        cur_state.u[i]=10;
    }
    best_ans=cur_state;
    double cur=check(back_test(data,decode(cur_state),&bases).line);
    double best=cur;
    double T=100000,speed=0.99;
    int num_time=1000;
    while(T>1e-4){
        for(int i=1;i<=num_time;i++){
            SA_state new_state=cur_state;
            int rd=rand_dim();
            new_state.u[rd]+=distur_rand()*T;
            new_state.u[rd]=std::max(new_state.u[rd],1.0);
            new_state.t[rd]+=distur_rand()*T*0.001;
            new_state.t[rd]=std::max(new_state.t[rd],1e-5);
            double new_energy=check(back_test(data,decode(new_state),&bases).line);
            double de=new_energy-cur;
            if(de>0||exp(de/T)>rand01()){
                cur_state=new_state;
                cur=new_energy;
            }
            if(new_energy>best){
                best=new_energy;
                best_ans=new_state;
            }
        }
        T*=speed;
    }
    return best_ans;
}
std::vector<kline> data;
int main(int argc,char* argv[]){
    std::string filename="bitc_ohlc.txt";
    if(argc>1){
        filename=argv[1];
    }
    data=load_data(filename);
    if(data.empty()){
        std::cerr<<"No data loaded. Exiting."<<std::endl;
        return 1;
    }
    std::cout<<"Loaded "<<data.size()<<" bars from "<<filename<<"."<<std::endl;
    int split_idx=(int)(data.size()*SPLIT_RATIO);
    std::vector<kline> train_data(data.begin(),data.begin()+split_idx);
    std::vector<kline> test_data(data.begin()+split_idx,data.end());
    std::cout<<"Train: "<<train_data.size()<<" bars, Test: "<<test_data.size()<<" bars"<<std::endl;
    std::vector<double> train_bases=calc_ma(train_data);
    std::cout<<"[SA] Starting optimization on training data..."<<std::endl;
    SA_state best_params=SA(train_data,train_bases);
    strategy best_s=decode(best_params);
    std::cout<<"[Eval] In-sample on training data:"<<std::endl;
    result train_res=back_test(train_data,best_s,&train_bases);
    double train_score=check(train_res.line);
    std::cout<<"  Final Equity: "<<train_res.line.back()<<std::endl;
    std::cout<<"  Calmar Score: "<<train_score<<std::endl;
    std::vector<double> test_bases=calc_ma(test_data);
    std::cout<<"[Eval] Out-of-sample on testing data:"<<std::endl;
    result test_res=back_test(test_data,best_s,&test_bases);
    double test_score=check(test_res.line);
    std::cout<<"  Final Equity: "<<test_res.line.back()<<std::endl;
    std::cout<<"  Calmar Score: "<<test_score<<std::endl;
    std::cout<<"[BM] Buy & Hold on testing data:"<<std::endl;
    result bh_res=buy_and_hold(test_data);
    double bh_score=check(bh_res.line);
    std::cout<<"  Final Equity: "<<bh_res.line.back()<<std::endl;
    std::cout<<"  Calmar Score: "<<bh_score<<std::endl;
    std::cout<<"========================="<<std::endl;
    std::cout<<"Best t:";
    for(int i=0;i<state;i++){
        std::cout<<best_params.t[i]<<" ";
    }
    std::cout<<"\nBest u:";
    for(int i=0;i<state;i++){
        std::cout<<best_params.u[i]<<" ";
    }
    std::cout<<"\n";
    return 0;
}
