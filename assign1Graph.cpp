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

/* Global variables */
int** mutualFriendsMatrix;
int** recMatrix;
AdjacencyMatrix adjMatrix;
int size;
int threadCount;
int numRecommends;

/* Serial functions */
int** makeMatrix(size_t rows, size_t columns);
void printMatrix(int** matrix, size_t rows, size_t columns);
void findMutualFriends (int i);
void indicesOfKMaxElements(int* input, int size, int* results, int k);
void recommendFriends(int start, int end);

/* Parallel function */
void* PthFindMutualFriends(void* rank);

/*----------------------------------------------------------------------------*/
int main(int argc, char** argv)
{
    if(argc<4){
      cout<<"To run: ./assign1Graph filename k threads"<<endl;
      cout<<"./assign1Graph networkDatasets/toyGraph1.txt 4 2"<<endl;
      return 0;
    }

    pthread_t* threadHandles;
    ifstream myfile(argv[1],std::ios_base::in);
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

    size = (int)adjMatrix.size();
    numRecommends = atoi(argv[2]);
    mutualFriendsMatrix = makeMatrix(size, size);
    recMatrix = makeMatrix(size, numRecommends);

    threadCount = atoi(argv[3]);
    if(threadCount > 1) {
      threadHandles = (pthread_t*)malloc(threadCount*sizeof(pthread_t));

      for (int thread = 0; thread < threadCount; thread++) {
        pthread_create(&threadHandles[thread], NULL,
                       PthFindMutualFriends, (void*) thread);
      }

      for (int thread = 0; thread < threadCount; thread++) {
        pthread_join(threadHandles[thread], NULL);
      }
      free(threadHandles);
    } else {
      for (int i = 0; i < size; i++) {
        findMutualFriends(i);
      }
    }
    recommendFriends(0,size);

    printMatrix(recMatrix, size, numRecommends);

    return 0;
}

void printAdjMatrix(AdjacencyMatrix adjMatrix)
{
    for (unsigned i=0; i<adjMatrix.size(); i++)
    {
        for (unsigned j=0; j<adjMatrix[i].size(); j++) cout<<adjMatrix[i][j]<<" ";
        cout<<endl;
    }
}

/*------------------------------------------------------------------------------
 * Function: makeMatrix
 * Purpose:  Allocate a matrix of ints of size rows x columns
 */
int** makeMatrix(size_t rows, size_t columns) {
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

/*------------------------------------------------------------------------------
 * Function: printMatrix
 * Purpose:  Print a matrix to std out
 * In args:  matrix, rows, columns
 */
void printMatrix(int** matrix, size_t rows, size_t columns) {
  for (unsigned r = 0; r < rows; r++) {
    for (unsigned c = 0; c < columns; c++) {
      cout << matrix[r][c] << " ";
    }
    cout << endl;
  }
}

/*------------------------------------------------------------------------------
 * Function:        countMutualFriends
 * Purpose:         Counts 
 * In args:         i : row in adjMatrix
 * Global in vars:  adjMatrix, size
 * Global out vars: mutualFriendsMatrix
 */
void findMutualFriends(int i) {
  // I don't want to be recommended myself as a friend
  mutualFriendsMatrix[i][i] = -1;

  // For every other person in the world, I already know whether or not I am
  // friends with him/her
  for (int j = i; j < size; j++) {
    // If i am friends with person j, then I want to look at all of j's friends
    // and see which ones I'm not already friends with.
    if (adjMatrix[i][j] == 1) {
      if(mutualFriendsMatrix[i][j] == -1) {
          continue;
      }
      // I don't want to j to be recommended to me, we're already friends
      mutualFriendsMatrix[i][j] = -1;

      // Looking at all of j's friends,
      for (int k = 0; k < size; k++) {
        if (adjMatrix[i][k] == 0      // I'm not yet friends with this person, k
            && adjMatrix [j][k] == 1  // j is already friends with this person
            && i != k) {              // this person is not me
          mutualFriendsMatrix[i][k]++;// then I have one more mutual friend, j,
          mutualFriendsMatrix[k][i]++;
                                      // with this person, k, than I knew I
                                      // had before.
        }
      }
    }
  }
}

// Takes an array of integers 'input' of length 'size' and places the indices
// of the 'k' max elements into the int array 'results', assumed to be of
// length 'k'. Max index is placed in results[0], index of 2nd largest in results[1], etc.
void indicesOfKMaxElements(int* input, int size, int* results, int k) {
  // priority_queue is basically a min heap
  priority_queue<pair<int, int>, vector<pair<int,int> >, greater <pair<int, int> > > q;
  for (int i = 0; i < size; i++) {
    if(q.size() < (size_t)k) {
      q.push(pair<int,int>(input[i], i));
    } else if (q.top().first < input[i]) {
      q.pop();
      q.push(pair<int,int>(input[i], i));
    }
  }

  k = q.size();
  for (int i = 0; i < k; i++) {
    results[k - i - 1] = q.top().second;
    q.pop();
  }
  return;
}

void recommendFriends(int startRow, int endRow) {
  for (int i = startRow; i < endRow; i++) {
    indicesOfKMaxElements(mutualFriendsMatrix[i], size, recMatrix[i], numRecommends);
  }
}

void* PthFindMutualFriends(void* rank) {
  long myRank = (long)rank;

  for (int i = myRank; i < size; i += threadCount) {
    findMutualFriends(i);
  }

  pthread_exit(NULL);
}

