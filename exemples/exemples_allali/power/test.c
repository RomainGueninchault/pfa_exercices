#include "ccheck.h"

static int solution(int a,int b){
  int r=1;
  while(b>0){
    r*=a;
    b-=1;
  }
  return r;
}

int power(int a,int b);

int main(){
  int i=0;
  // zero case:
  CHECK("%d,%d","%d",solution(10,0),power,10,0);
  // one case:
  CHECK("%d,%d","%d",solution(12,1),power,12,1);
  // random checks
  for(i=0;i<10;++i){
    int a=rand()%100;
    int b=rand()%10;
    CHECK("%d,%d","%d",solution(a,b),power,a,b);
  }
  
  return EXIT_SUCCESS;
}
