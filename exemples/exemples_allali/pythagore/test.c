#include "ccheck.h"
#include <math.h>

static char solution(int a,int b,int c){
  int a2=a*a;
  int b2=b*b;
  int c2=c*c;
  if ((a2+b2)==c2) return 1;
  if ((a2+c2)==b2) return 1;
  if ((b2+c2)==a2) return 1;
  return 0;
}

char pythagore(int a,int b,int c);

int main(){
  int i=0;

  CHECK("%d,%d,%d","%d",0,pythagore,2,3,4);
  CHECK("%d,%d,%d","%d",1,pythagore,3,4,5);

  // random checks
  for(i=0;i<10;++i){
    int a=rand()%100;
    int b=rand()%10;
    int c=sqrt(a*a+b*b);
    CHECK("%d,%d,%d","%d",solution(a,b,c),pythagore,a,b,c);
  }
  
  return EXIT_SUCCESS;
}
