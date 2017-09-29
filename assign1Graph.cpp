//CSCI415; Saeed Salem, 9/6/2017
//To compile: g++ -O3 -w assign1Graph.cpp -lpthread -o assign1Graph
//To run: ./assign1Graph filename k
// ./assign1Graph networkDatasets/toyGraph1.txt 2

#include <stdio.h>   /* printf, fgets */
#include <stdlib.h>  /* atoi */
#include <iostream>
#include <fstream>
#include <vector>
#include <queue>

using namespace std;

typedef vector<vector<int> > AdjacencyMatrix;
AdjacencyMatrix adjMatrix;

int** makeRectangularMatrix(size_t rows, size_t columns);
void printRectangularMatrix(int** matrix, size_t rows, size_t columns);
int** findMutualFriends (AdjacencyMatrix adjMatrix, unsigned start, unsigned end, int** resMatrix);
void indicesOfKMaxElements(int* input, size_t size, int* results, unsigned k);
void recommendFriends(AdjacencyMatrix adjMatrix, size_t size, unsigned start, unsigned end, unsigned numRecommends, int** recMatrix);


void printAdjMatrix(AdjacencyMatrix adjMatrix)
{
    for (unsigned i=0; i<adjMatrix.size(); i++)
    {
        for (unsigned j=0; j<adjMatrix[i].size(); j++) cout<<adjMatrix[i][j]<<" ";
        cout<<endl;
    }
}

int** makeRectangularMatrix(size_t rows, size_t columns) {
  int** matrix = 0;
  matrix = new int*[rows];

  for (unsigned r = 0; r < rows; r++) {
    matrix [r] = new int[columns];

    for (unsigned c = 0; c < columns; c++) {
      matrix[r][c] = 0;
    }
  }

  return matrix;
}

void printRectangularMatrix(int** matrix, size_t rows, size_t columns) {
  for (unsigned r = 0; r < rows; r++) {
    for (unsigned c = 0; c < columns; c++) {
      cout << matrix[r][c] << " ";
    }
    cout << endl;
  }}

// O(n^3)
int** findMutualFriends (AdjacencyMatrix adjMatrix, unsigned start, unsigned end, int** resMatrix) {
  unsigned size = adjMatrix.size();

  // For each person in the matrix, number i. I am i, i is me.
  for (unsigned i = start; i < end; i++) {

    // For every other person in the world, I already know whether or not I am
    // friends with him/her
    for (unsigned j = 0; j < size; j++) {
      // If i am friends with person j, then I want to look at all of j's friends
      // and see which ones I'm not already friends with.
      if (adjMatrix[i][j] == 1) {
        // I don't want to j to be recommended to me, we're already friends
        resMatrix[i][j] = -1;

        // Looking at all of j's friends,
        for (unsigned k = 0; k < size; k++) {
          if (adjMatrix[i][k] == 0     // I'm not yet friends with this person, k
              && adjMatrix [j][k] == 1 // j is already friends with this person
              && i != k) {             // this person is not me
            resMatrix[i][k]++;         // then I have one more mutual friend, j,
                                       // with this person, k, than I knew I
                                       // had before.
          }
        }
      }
    }
  }

  return resMatrix;
}

// Takes an array of integers 'input' of length 'size' and places the indices
// of the 'k' max elements into the int array 'results', assumed to be of
// length 'k'. Max index is placed in results[0], index of 2nd largest in results[1], etc.
void indicesOfKMaxElements(int* input, size_t size, int* results, unsigned k) {
  // priority_queue is basically a min heap
  priority_queue<pair<int, int>, vector<pair<int,int> >, greater <pair<int, int> > > q;
  for (unsigned i = 0; i < size; i++) {
    if(q.size() < k) {
      q.push(pair<int,int>(input[i], i));
    } else if (q.top().first < input[i]) {
      q.pop();
      q.push(pair<int,int>(input[i], i));
    }
  }

  k = q.size();
  for (unsigned i = 0; i < k; i++) {
    results[k - i - 1] = q.top().second;
    q.pop();
  }
  return;
}

void recommendFriends(AdjacencyMatrix adjMatrix, unsigned start, unsigned end, unsigned numRecommends, int** recMatrix) {
  unsigned size = adjMatrix.size();
  int** mutualFriendsMatrix = makeRectangularMatrix(size, size);
  findMutualFriends(adjMatrix, 0, size, mutualFriendsMatrix);

  for (unsigned i = start; i < end; i++) {
    indicesOfKMaxElements(mutualFriendsMatrix[i], size, recMatrix[i], numRecommends);
  }
}

int main(int argc, char** argv)
{
    if(argc<4){
      cout<<"To run: ./assign1Graph filename k threads"<<endl;
      cout<<"./assign1Graph networkDatasets/toyGraph1.txt 4 2"<<endl;
      return 0;
    }

    ifstream myfile(argv[1],std::ios_base::in);
    int k = atoi(argv[2]);
    int numThreads = atoi(argv[3]);
    int u,v;
    int maxNode = 0;
    vector<pair<int,int> > allEdges;
    while(myfile >> u >> v)
    {
        allEdges.push_back(make_pair(u,v));
        if(u > maxNode) {
          maxNode = u;
        }

        if(v > maxNode) {
          maxNode = v;
        }
    }
    myfile.close();

    int n = maxNode + 1;  //Since nodes starts with 0

    adjMatrix = AdjacencyMatrix(n,vector<int>(n));
    //populate the matrix
    for(unsigned i = 0; i < allEdges.size(); i++){
       u = allEdges[i].first;
       v = allEdges[i].second;
       adjMatrix[u][v] = 1;
       adjMatrix[v][u] = 1;
    }

    unsigned size = adjMatrix.size();

    int** recMatrix = makeRectangularMatrix(size, k);
    recommendFriends(adjMatrix, 0, size, k, recMatrix);

    printRectangularMatrix(recMatrix, size, k);

return 0;
}
