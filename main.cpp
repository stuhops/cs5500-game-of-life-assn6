#include <iostream>
#include <mpi.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <chrono> 

#define MCW MPI_COMM_WORLD

using namespace std;
using namespace std::chrono; 

void printBoard(int arr[], const int ROW_SIZE, const int ROWS, string printMsg) {
  cout << " " << printMsg << endl;
  for(int i = 0; i < ROWS; ++i) {
    for(int j = 0; j < ROW_SIZE; ++j) {
      if(arr[ROW_SIZE * i + j])
        cout << " 0";
      else
        cout << " -";
    }
    cout << endl;
  }
  cout << endl;
}


void printArr(int arr[], const int ROW_SIZE, const int ROWS, string printMsg) {
  cout << " " << printMsg << endl;
  for(int i = 0; i < ROWS; ++i) {
    for(int j = 0; j < ROW_SIZE; ++j) {
      cout << " " << arr[ROW_SIZE * i + j];
    }
    cout << endl;
  }
  cout << endl;
}


int calcNeighbors(int arr[], const int ROW_SIZE, int rows, int i, int j) {
  int neighbors = 0;

  //north 3
  if(i != 0) {
    // Upper left
    if(j != 0)
      neighbors += arr[ROW_SIZE * (i-1) + (j-1)];
    // Upper
    neighbors += arr[ROW_SIZE * (i-1) + j];
    // Upper right
    if(j+1 < ROW_SIZE)
      neighbors += arr[ROW_SIZE * (i-1) + (j+1)];
  }

  // Middle 2
  // Left
  if(j != 0)
    neighbors += arr[ROW_SIZE * (i) + (j-1)];
  // Right
  if(j+1 < ROW_SIZE)
    neighbors += arr[ROW_SIZE * (i) + (j+1)];

  // South 3 
  if(i+1 < rows) {
    // Upper left
    if(j != 0)
      neighbors += arr[ROW_SIZE * (i+1) + (j-1)];
    // Upper
    neighbors += arr[ROW_SIZE * (i+1) + j];
    // Upper right
    if(j+1 < ROW_SIZE)
      neighbors += arr[ROW_SIZE * (i+1) + (j+1)];
  }

  return neighbors;
}


void makeNeighborMap(int SPLICE_SIZE, int ROW_SIZE, int graph[]) {
  int tmpArr[SPLICE_SIZE];
  for(int i = 0; i < SPLICE_SIZE; i++) {
    tmpArr[i] = graph[i];
  }

  for(int i = 0; i < SPLICE_SIZE / ROW_SIZE; ++i) {
    for(int j = 0; j < ROW_SIZE; ++j) {
      tmpArr[ROW_SIZE * i + j]= calcNeighbors(graph, ROW_SIZE, SPLICE_SIZE / ROW_SIZE, i, j);
    }
  }
  printArr(tmpArr, ROW_SIZE, SPLICE_SIZE / ROW_SIZE, "2");
}


int main(int argc, char **argv) {

  int rank, size;
  int data;
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MCW, &rank);
  MPI_Comm_size(MCW, &size);
  const int ITERS = 40;
  const int ROW_SIZE = 32;
  const int SPLICE_SIZE = ROW_SIZE * (2 + ROW_SIZE / size);


  if(!rank) {                             //0  1  2  3  4  5  6  7
    srand(time(0));
    int graph[(1 + ROW_SIZE) * ROW_SIZE];
    for(int i = 0; i < ROW_SIZE; ++i) {
      for(int j = 0; j < ROW_SIZE; ++j) {
        graph[ROW_SIZE * i + j] = 0;
        if(!(rand() % 5))
          graph[ROW_SIZE * i + j] = 1;
      }
    }

  // int graph[(1+ROW_SIZE) * ROW_SIZE] = {0,0,0,0,0,0,0,0,
  //                                       0,0,0,0,0,0,0,0,
  //                                       0,1,0,0,0,0,0,0,
  //                                       0,0,1,1,0,0,0,0,
  //                                       0,1,1,0,0,0,0,0,
  //                                       0,0,0,0,0,0,0,0,
  //                                       0,0,0,0,0,0,0,0,
  //                                       0,0,0,0,0,0,0,0,
  //                                       0,0,0,0,0,0,0,0};

      int tmpArrZ[SPLICE_SIZE];
      printBoard(&graph[ROW_SIZE], ROW_SIZE, ROW_SIZE, "Full Board at Iteration 0:");
      
    for(int iters = 0; iters < ITERS; ++iters) {
      sleep(1);

      auto start = high_resolution_clock::now();
      for(int i = 0; i < ROW_SIZE; ++i) {
        graph[i] = 0;
      }

      for(int j = 0; j < SPLICE_SIZE; ++j) {
        tmpArrZ[j] = graph[j];
      }
      // printBoard(tmpArrZ, ROW_SIZE, (SPLICE_SIZE / ROW_SIZE) - 1, "Board 0");

      int tmpArr[SPLICE_SIZE];
      for(int i = 1; i < size; ++i) {
        for(int j = 0; j < SPLICE_SIZE; ++j) {
          tmpArr[j] = graph[(i*(ROW_SIZE/size)-1)*ROW_SIZE + j];
        }
        // printBoard(tmpArr, ROW_SIZE, (2 + ROW_SIZE / size), "Board " + to_string(i));
        MPI_Send(tmpArr, SPLICE_SIZE, MPI_INT, i, 0, MCW);
      }

      for(int i = 0; i < (SPLICE_SIZE / ROW_SIZE); ++i) {
        for(int j = 0; j < ROW_SIZE; ++j) {
          int neighbors = calcNeighbors(graph, ROW_SIZE, SPLICE_SIZE / ROW_SIZE, i, j);

          if(neighbors == 0 || neighbors == 1) {
            tmpArrZ[ROW_SIZE * i + j] = 0;
            // cout << 1 << "Hit on process " << rank << endl;
          }
          else if(neighbors == 3){
            tmpArrZ[ROW_SIZE * i + j] = 1;
            // cout << 2 << " Hit on process " << rank << endl;
          }
          else if(neighbors > 3){
            tmpArrZ[ROW_SIZE * i + j] = 0;
            // cout << 3 << " Hit on process " << rank << endl;
          }
        }
      }
      // printBoard(tmpArrZ, ROW_SIZE, (SPLICE_SIZE / ROW_SIZE), "Board 0 After");
      // printArr(tmpArrZ, ROW_SIZE, (SPLICE_SIZE / ROW_SIZE) - 1, "Board 0 After");

      for(int i = 0; i < SPLICE_SIZE - ROW_SIZE; ++i) {
        graph[i] = tmpArrZ[i];
      }

      for(int i = 1; i < size; ++i) {
        MPI_Recv(tmpArr, SPLICE_SIZE, MPI_INT, i, 0, MCW,MPI_STATUS_IGNORE);

        for(int j = ROW_SIZE; j < SPLICE_SIZE; ++j) {
          graph[(i*(ROW_SIZE/size)-1)*ROW_SIZE + j] = tmpArr[j];
        }
      }

      auto stop = high_resolution_clock::now(); 
      auto duration = duration_cast<microseconds>(stop - start);
      cout << " TIME: " << duration.count() << " microseconds" << endl;

      string printMsg = "Full Board at iteration " + to_string(iters+1) + ":";
      printBoard(&graph[ROW_SIZE], ROW_SIZE, ROW_SIZE, printMsg);
    }
  }

  // <<<<<<<<<<<<<<<< SLAVES >>>>>>>>>>>>>>>>>>>>>
  if(rank) {
    for(int iters = 0; iters < ITERS; ++iters){
      int arr[SPLICE_SIZE];
      int tmpArr[SPLICE_SIZE];
      MPI_Recv(arr, SPLICE_SIZE, MPI_INT, MPI_ANY_SOURCE, 0, MCW,MPI_STATUS_IGNORE);

      for(int i = 0; i < SPLICE_SIZE; ++i) {
        tmpArr[i] = arr[i];
      }

      for(int i = 0; i < SPLICE_SIZE / ROW_SIZE; ++i) {
        for(int j = 0; j < ROW_SIZE; ++j) {
          int neighbors = calcNeighbors(arr, ROW_SIZE, SPLICE_SIZE / ROW_SIZE, i, j);
          // if(rank == 1 && i == 1 && (j == 5)) {
          //   cout << endl << "(i, j) = (" << i << ", " << j << ") - Neighbors = " << neighbors << endl << endl;
          // }

          if(neighbors == 0 || neighbors == 1) {
            tmpArr[ROW_SIZE * i + j] = 0;
            // cout << 1 << "Hit on process " << rank << endl;
          }
          else if(neighbors == 3) {
            tmpArr[ROW_SIZE * i + j] = 1;
            // cout << 2 <<  "Hit on process " << rank << " at coord (" << i << ", " << j << ") " << endl;
          }
          else if(neighbors > 3) {
            tmpArr[ROW_SIZE * i + j] = 0;
            // cout << 3 <<  "Hit on process " << rank << " at coord (" << i << ", " << j << ") " << endl;
          }
        }
      }
      if(rank == 1) {
        // makeNeighborMap(SPLICE_SIZE, ROW_SIZE, arr);
        // printBoard(tmpArr, ROW_SIZE, 6, "Board 1 After");
        // printArr(tmpArr, ROW_SIZE, 6, "Board 0 After");
      }

      MPI_Send(tmpArr, SPLICE_SIZE, MPI_INT, 0, 0, MCW);
    }

  }

  MPI_Finalize();
  return 0;
}