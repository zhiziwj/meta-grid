#include "core.h"
#include "backtest.h"
#include <cassert>
#include <cmath>
void test_decode(){
    SA_state input;
    input.t[0]=0.2; input.t[1]=0.3; input.t[2]=0.5;
    input.u[0]=10;  input.u[1]=20;  input.u[2]=30;
    strategy s=decode(input);
    assert(fabs(s.p[0]-0.0)<1e-9);
    assert(fabs(s.p[1]-0.2)<1e-9);
    assert(fabs(s.p[2]-0.5)<1e-9);
    assert(fabs(s.p[3]-1.0)<1e-9);
    assert(fabs(s.s[3]-0.0)<1e-9);
    assert(fabs(s.s[2]-30.0)<1e-9);
    assert(fabs(s.s[1]-50.0)<1e-9);
    assert(fabs(s.s[0]-60.0)<1e-9);
    std::cout<<"[PASS] test_decode"<<std::endl;
}
void test_back_test_empty(){
    std::vector<kline> empty;
    strategy s;
    result r=back_test(empty,s);
    assert(r.line.empty());
    std::cout<<"[PASS] test_back_test_empty"<<std::endl;
}
void test_back_test_simple(){
    std::vector<kline> data;
    data.push_back({"2020-01-01",100.0,105.0,95.0,100.0});
    data.push_back({"2020-01-02",100.0,105.0,95.0,110.0});
    data.push_back({"2020-01-03",110.0,115.0,105.0,90.0});
    data.push_back({"2020-01-04",90.0,95.0,85.0,100.0});
    SA_state input;
    input.t[0]=0.2; input.t[1]=0.3; input.t[2]=0.5;
    input.u[0]=10;  input.u[1]=20;  input.u[2]=30;
    strategy s=decode(input);
    result r=back_test(data,s);
    assert(r.line.size()==4);
    assert(r.line.back()>0);
    std::cout<<"[PASS] test_back_test_simple (equity="<<r.line.back()<<")"<<std::endl;
}
void test_check_normal(){
    std::vector<double> line;
    line.push_back(1000.0);
    line.push_back(1100.0);
    line.push_back(1050.0);
    line.push_back(1200.0);
    double score=check(line);
    assert(score>0.0);
    std::cout<<"[PASS] test_check_normal (score="<<score<<")"<<std::endl;
}
void test_check_too_short(){
    std::vector<double> line;
    line.push_back(1000.0);
    double score=check(line);
    assert(fabs(score-0.0)<1e-9);
    std::cout<<"[PASS] test_check_too_short"<<std::endl;
}
void test_check_zero_start(){
    std::vector<double> line;
    line.push_back(0.0);
    line.push_back(100.0);
    double score=check(line);
    assert(fabs(score-0.0)<1e-9);
    std::cout<<"[PASS] test_check_zero_start"<<std::endl;
}
void test_check_heavy_loss(){
    std::vector<double> line;
    line.push_back(1000.0);
    line.push_back(1.0);
    double score=check(line);
    assert(score<0.0);
    std::cout<<"[PASS] test_check_heavy_loss (score="<<score<<")"<<std::endl;
}
void test_back_test_zero_segment(const char* title,SA_state input){
    std::vector<kline> data;
    data.push_back({"2020-01-01",100.0,105.0,95.0,100.0});
    data.push_back({"2020-01-02",100.0,105.0,95.0,105.0});
    strategy s=decode(input);
    result r=back_test(data,s);
    assert(r.line.size()==2);
    std::cout<<"[PASS] test_zero_segment_"<<title<<std::endl;
}
void test_back_test_zero_segments(){
    SA_state input;
    input.t[0]=0.0; input.t[1]=0.3; input.t[2]=0.5;
    input.u[0]=10;  input.u[1]=20;  input.u[2]=30;
    test_back_test_zero_segment("t0",input);
    input.t[0]=0.2; input.t[1]=0.0; input.t[2]=0.5;
    test_back_test_zero_segment("t1",input);
    input.t[0]=0.2; input.t[1]=0.3; input.t[2]=0.0;
    test_back_test_zero_segment("t2",input);
}
int main(){
    std::cout<<"=== Running Tests ==="<<std::endl;
    test_decode();
    test_back_test_empty();
    test_back_test_simple();
    test_check_normal();
    test_check_too_short();
    test_check_zero_start();
    test_check_heavy_loss();
    test_back_test_zero_segments();
    std::cout<<"=== All Tests Passed ==="<<std::endl;
    return 0;
}
