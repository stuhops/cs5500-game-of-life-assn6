#include <iostream>
#include <mpi.h>
#include <unistd.h>
#include <stdlib.h>

#define MCW MPI_COMM_WORLD

using namespace std;

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


int calcNeighbors(int arr[], const int ROW_SIZE, int i, int j) {
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
  if(i+1 < ROW_SIZE) {
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


int main(int argc, char **argv) {

  int rank, size;
  int data;
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MCW, &rank);
  MPI_Comm_size(MCW, &size);
  const int ITERS = 4;
  const int ROW_SIZE = 8;
  const int SPLICE_SIZE = ROW_SIZE * (2 + ROW_SIZE / size);

  int arr[SPLICE_SIZE];

  if(!rank) {                             //0  1  2  3  4  5  6  7
    for(int iters = 0; iters < ITERS; ++iters) {
      int graph[(1 + ROW_SIZE) * ROW_SIZE] = {0, 0, 0, 0, 0, 0, 0, 0,
                                              0, 0, 0, 0, 0, 0, 0, 0,
                                              0, 0, 0, 0, 0, 0, 0, 0,
                                              0, 0, 0, 0, 1, 0, 0, 0,
                                              0, 0, 0, 0, 1, 0, 0, 0,
                                              0, 0, 0, 0, 1, 0, 0, 0,
                                              0, 0, 0, 0, 0, 0, 0, 0,
                                              0, 0, 0, 0, 0, 0, 0, 0,
                                              0, 0, 0, 0, 0, 0, 0, 0,
                                              };

      int tmpArrZ[SPLICE_SIZE - ROW_SIZE];
      printBoard(&graph[ROW_SIZE], ROW_SIZE, ROW_SIZE, "Full Board:");
      
      for(int j = 0; j < SPLICE_SIZE - ROW_SIZE; ++j) {
        tmpArrZ[j] = graph[j];
      }
      // printBoard(tmpArrZ, ROW_SIZE, 3, "Board 0");

      int tmpArr[SPLICE_SIZE];
      for(int i = 1; i < size; ++i) {
        for(int j = 0; j < SPLICE_SIZE; ++j) {
          tmpArr[j] = graph[(i*2-1)*ROW_SIZE + j];
        }
        // printBoard(tmpArr, ROW_SIZE, 4, "Board " + to_string(i));
        MPI_Send(tmpArr, SPLICE_SIZE, MPI_INT, i, 0, MCW);
      }

      for(int i = 0; i < ROW_SIZE; ++i) {
        for(int j = 0; j < 2 + ROW_SIZE / size; ++j) {
          int neighbors = calcNeighbors(arr, ROW_SIZE, i, j);

          if(neighbors == 0 || neighbors == 1)
            tmpArrZ[ROW_SIZE * i + j] = 0;
          else if(neighbors == 3)
            tmpArrZ[ROW_SIZE * i + j] = 1;
          else if(neighbors > 3)
            tmpArrZ[ROW_SIZE * i + j] = 0;
        }
      }
      // printBoard(tmpArrZ, ROW_SIZE, 3, "Board 0");

      for(int i = 0; i < SPLICE_SIZE - ROW_SIZE; ++i) {
        arr[i] = tmpArrZ[i];
      }

      for(int i = 1; i < size; ++i) {
        MPI_Recv(tmpArr, SPLICE_SIZE, MPI_INT, i, 0, MCW,MPI_STATUS_IGNORE);

        for(int j = ROW_SIZE; j < SPLICE_SIZE; ++j) {
          graph[(i*2-1)*ROW_SIZE + j] = tmpArr[j];
        }
      }

      printBoard(&graph[ROW_SIZE], ROW_SIZE, ROW_SIZE, "Full Board:");
    }
  }

  if(rank) {
    for(int iters = 0; iters < ITERS; ++iters){
      int tmpArr[SPLICE_SIZE];
      MPI_Recv(arr, SPLICE_SIZE, MPI_INT, MPI_ANY_SOURCE, 0, MCW,MPI_STATUS_IGNORE);

      for(int i = 0; i < SPLICE_SIZE; ++i) {
        tmpArr[i] = arr[i];
      }

      for(int i = 0; i < SPLICE_SIZE / ROW_SIZE; ++i) {
        for(int j = 0; j < ROW_SIZE; ++j) {
          int neighbors = calcNeighbors(arr, ROW_SIZE, i, j);
          // if(rank == 2 && i == 1 && (j == 3 || j == 4 || j == 5)) {
          //   cout << endl << "(i, j) = (" << i << ", " << j << ") - Neighbors = " << neighbors << endl << endl;
          // }

          if(neighbors == 0 || neighbors == 1) {
            tmpArr[ROW_SIZE * i + j] = 0;
            // cout << 1 << "Hit on process " << rank << endl;
          }
          else if(neighbors == 3) {
            tmpArr[ROW_SIZE * i + j] = 1;
            // cout << 2 << "Hit on process " << rank << endl;
          }
          else if(neighbors > 3) {
            tmpArr[ROW_SIZE * i + j] = 0;
            // cout << 3 <<  "Hit on process " << rank << endl;
          }
        }
      }

      MPI_Send(tmpArr, SPLICE_SIZE, MPI_INT, 0, 0, MCW);
    }

  }

  MPI_Finalize();
  return 0;
}