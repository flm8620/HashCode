#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <iomanip>
#ifndef srcPath
#define srcPath "."
#endif
using namespace std;
string imgFolder = srcPath "/imgOut/";

void write(vector<unsigned char> & R,vector<unsigned char> & G,vector<unsigned char> & B
           ,int N,int M,int number) {
  stringstream ss;
  ss<<setfill ('0') << setw (7)<< number;
  string num;
  ss>>num;
  ofstream imgOut(imgFolder+"img"+num+".ppm");
  imgOut<<"P3"<<endl;
  imgOut<<M<<' '<<N<<endl;
  for(int i=0;i<N;i++){
    for(int j=0;j<M;j++){
      imgOut<<int(R[i*M+j])<<' '<<int(G[i*M+j])<<' '<<int(B[i*M+j])<<' ';
    }
    imgOut<<endl;
  }
  imgOut.close();
}
