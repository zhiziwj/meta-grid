#include "core.h"
#include "rand.h"
std::vector<kline> load_data(std::string filename){
    std::vector<kline> ans;
    std::fstream data;
    std::string str;
    double o,h,l,c;
    data.open(filename);
    while(data>>str>>o>>h>>l>>c){
        ans.push_back({str,o,h,l,c});
    }
    data.close();
    return ans;
}
strategy decode(SA_state input){
    strategy ans;
    ans.p[0]=0;
    for(int i=1;i<state;i++){
        ans.p[i]=ans.p[i-1]+input.t[i-1];
    }
    ans.s[state]=0.0;
    for(int i=state-1;i>=0;i--){
        ans.s[i]=ans.s[i+1]+input.u[i];
    }
    ans.p[state]=ans.p[state-1]+input.t[state-1];
    return ans;
}
result back_test(std::vector<kline> data,strategy s){
    double money=init_cash,shares=0;
    result ans;
    double base=data[0].o;
    for(int i=0;i<data.size();i++){
        double price=data[i].c;
        double norm=price/base;
        double value=0.0,ratio;
        for(int j=0;j<state;j++){
            if(norm<=s.p[j+1]){
                ratio=(norm-s.p[j])/(s.p[j+1]-s.p[j]);
                value=s.s[j]+ratio*(s.s[j+1]-s.s[j]);
                break;
            }
        }
        double t_shares=std::max(value/price,0.0);
        double delta=t_shares-shares;
        if(delta>0){
            double cost=delta*price*(1.0+FEE);
            if(cost<=money){
                money-=cost;
                shares+=delta;
            }
            else{
                shares+=money/(price*(1.0+FEE));
                money=0;
            }
        }
        else{
            delta=std::max(delta,-shares);
            if(delta<0){
                double in=-delta*price*(1.0-FEE);
                money+=in;
                shares+=delta;
            }
        }
        double cnt=money+shares*price;
        ans.line.push_back(cnt);
    }
    return ans;
}
double check(std::vector<double> line){
    if(line.size()<2){
        return 0.0;
    } 
    double s_val=line[0];
    double e_val=line.back();
    double total=(e_val-s_val)/s_val;
    double years=(double)line.size()/time_unit;
    double R=pow(1.0+total,1.0/years)-1.0;
    double max_val=s_val;
    double mod=0.0;
    for(int i=0;i<line.size();i++){
        double val=line[i];
        if(val>max_val){
            max_val=val;
        }
        double drawdown=(max_val-val)/max_val;
        if(drawdown>mod){
            mod=drawdown;
        }
    }
    if(mod<1e-9){
        if(R>0) return 1e9;
        else return 0;
    }
    return R/mod;
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
