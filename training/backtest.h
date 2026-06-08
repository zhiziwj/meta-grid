#ifndef BACKTEST_H
#define BACKTEST_H
#include "core.h"
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
std::vector<double> calc_ma(const std::vector<kline>& data){
    std::vector<double> ma(data.size());
    double sum=0;
    for(int i=0;i<(int)data.size();i++){
        sum+=data[i].c;
        if(i>=ma_period){
            sum-=data[i-ma_period].c;
            ma[i]=sum/ma_period;
        }
        else{
            ma[i]=sum/(i+1);
        }
    }
    return ma;
}
result back_test(std::vector<kline> data,strategy s,const std::vector<double>* bases=nullptr){
    result ans;
    if(data.empty()){
        return ans;
    }
    double money=init_cash,shares=0;
    double fixed_base=data[0].o;
    for(int i=0;i<(int)data.size();i++){
        double base=(bases!=nullptr && i<(int)bases->size()) ? (*bases)[i] : fixed_base;
        double price=data[i].c;
        double norm=price/base;
        double value=0.0,ratio;
        for(int j=0;j<state;j++){
            if(norm<=s.p[j+1]){
                double seg=s.p[j+1]-s.p[j];
                if(seg<1e-12){
                    value=s.s[j];
                }
                else{
                    ratio=(norm-s.p[j])/seg;
                    value=s.s[j]+ratio*(s.s[j+1]-s.s[j]);
                }
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
result buy_and_hold(std::vector<kline> data){
    result ans;
    if(data.empty()){
        return ans;
    }
    double shares=init_cash/data[0].o*(1.0-FEE);
    double money=0;
    for(int i=0;i<(int)data.size();i++){
        double cnt=money+shares*data[i].c;
        ans.line.push_back(cnt);
    }
    return ans;
}
double check(std::vector<double> line){
    if(line.size()<2){
        return 0.0;
    } 
    double s_val=line[0];
    if(s_val<1e-12){
        return 0.0;
    }
    double e_val=line.back();
    double total=(e_val-s_val)/s_val;
    if(total<=-0.999){
        return -1e9;
    }
    double years=(double)line.size()/time_unit;
    double R=pow(1.0+total,1.0/years)-1.0;
    double max_val=s_val;
    double mod=0.0;
    for(int i=0;i<(int)line.size();i++){
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
#endif
