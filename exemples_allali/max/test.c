#include "ccheck.h"

static int solution(int a,int b){
  if (a>b) return a;
  return b;
}

int max(int a,int b);

int main(){
  int i=0;
  // zero case:
  CHECK("%d,%d","%d",0,max,0,0);
  // one case:
  CHECK("%d,%d","%d",1,max,12,1);
  // random checks
  for(i=0;i<10;++i){
    int a=rand()%1000-500;
    int b=rand()%1000-500;
    CHECK("%d,%d","%d",solution(a,b),max,a,b);
  }
  
  return EXIT_SUCCESS;
}
