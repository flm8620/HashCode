#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cassert>

#ifndef srcPath
#define srcPath "."
#endif


using namespace std;
string inputFile = srcPath "/logo.in";
void readFile(string fileName, vector<bool> &wall, int &n, int &m) {
  ifstream input(fileName);
  input >> n >> m;
  wall.resize(n * m);
  for (int i = 0; i < n; i++) {
    for (int j = 0; j < m; j++) {
      char c;
      input >> c;
      if (c == '.') {
        wall[i * m + j] = false;
      } else {
        wall[i * m + j] = true;
      }
    }
    char c;
    input >> c;//delete '\n'
    //assert(c == '\n');
  }
}
int main() {
  int n, m;
  vector<bool> wall;
  readFile(inputFile,wall,n,m);

  for (int i = 0; i < n; i++) {
    for (int j = 0; j < m; j++) {
      if (wall[i * m + j])
        cout << '#';
      else
        cout << '.';
    }
    cout << endl;

  }
  return 0;
}