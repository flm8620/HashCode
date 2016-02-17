#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <cassert>
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
  ofstream imgOut(imgFolder+"img"+num+".ppm",ios::binary);

  ss.clear();
  ss<<"P6 "<<M<<' '<<N<<' '<<255<<' ';
  string s;
  ss>>s;
  imgOut.write(s.c_str(),s.size());
  //imgOut<<'P'<<'6'<<' ';
  //imgOut<<M<<N<<endl;
  for(int i=0;i<N;i++){
    for(int j=0;j<M;j++){
      imgOut.write((char *)&(R[i*M+j]),sizeof(unsigned char));
      imgOut.write((char *)&(G[i*M+j]),sizeof(unsigned char));
      imgOut.write((char *)&(B[i*M+j]),sizeof(unsigned char));
    }
  }
  imgOut.close();
}
