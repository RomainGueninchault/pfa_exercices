#include "ccheck.h"

static int solution(int a,int b){
  if (a<b) return a;
  return b;
}

int min(int a,int b);

int main(){
  int i=0;
  // zero case:
  CHECK("%d,%d","%d",0,min,0,0);
  // one case:
  CHECK("%d,%d","%d",1,min,12,1);
  // random checks
  for(i=0;i<10;++i){
    int a=rand()%1000-500;
    int b=rand()%1000-500;
    CHECK("%d,%d","%d",solution(a,b),min,a,b);
  }
  
  return EXIT_SUCCESS;
}
