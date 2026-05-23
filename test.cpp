#include "core.h"
#include "rand.h"
#include <fstream>
std::vector<kline> load_data(){
    std::vector<kline> ans;
    std::fstream data;
    std::string str;
    double o,h,l,c;
    data.open("bitc_ohlc.txt");
    while(data>>str>>o>>h>>l>>c){
        ans.push_back({str,o,h,l,c});
    }
    data.close();
    return ans;
}
SA_state load_strategy(){
    SA_state ans;
    std::fstream file;
    file.open("strategy.txt");
    for(int i=0;i<state;i++){
    	file>>ans.t[i];
	}
    for(int i=0;i<state;i++){
    	file>>ans.u[i];
	}
    file.close();
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
std::vector<kline> data;
int main(){
    data=load_data();
    std::cout<<"Loaded "<<data.size()<<" bars."<<std::endl;
    SA_state params=load_strategy();
    strategy s=decode(params);
    result res=back_test(data,s);
    double score=check(res.line);
    std::cout<<"========================="<<std::endl;
    std::cout<<"Final Equity: "<<res.line.back()<<std::endl;
    std::cout<<"CALMAR SCORE: "<<score<<std::endl;
    std::cout<<"========================="<<std::endl;
    return 0;
}
