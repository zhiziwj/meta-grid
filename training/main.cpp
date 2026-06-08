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
SA_state SA(std::vector<kline> data){
    std::srand(time(0));
    SA_state cur_state,best_ans;
    for(int i=0;i<state;i++){
        cur_state.t[i]=0.2;
        cur_state.u[i]=10;
    }
    best_ans=cur_state;
    double cur=check(back_test(data,decode(cur_state)).line);
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
            double new_energy=check(back_test(data,decode(new_state)).line);
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
    std::cout<<"[SA] Starting optimization..."<<std::endl;
    SA_state best_params=SA(data);
    strategy best_s=decode(best_params);
    result final_res=back_test(data,best_s);
    double final_score=check(final_res.line);
    std::cout<<"========================="<<std::endl;
    std::cout<<"OPTIMIZED Final Equity: "<<final_res.line.back() << std::endl;
    std::cout<< "OPTIMIZED CALMAR SCORE: "<<final_score<<std::endl;
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
