#ifndef Solver_hpp
#define Solver_hpp

#define SOLVER_BOUND_TOP 0
#define SOLVER_BOUND_RIGHT 1
#define SOLVER_BOUND_BOTTOM 2
#define SOLVER_BOUND_LEFT 3

#include <SDL2/SDL.h>
#include <iostream>
#include <vector>
#include <string>
#include "Texture.hpp"
#include "ctpl_stl.h"

class Nonogram;

class Solver {
public:
  typedef std::vector<int> tRow;

  Solver();
  ~Solver();

  std::vector<std::vector<int>> *solution;
  int saverIndex = 0;

  Texture *progress;

  void init(SDL_Renderer *r)
  {
    renderer = r;
    progress = new Texture();
  }
  void printPuzzle(std::vector<tRow> cells);
  void printCombos(std::vector<tRow> cells);

  int requiredCells(const std::vector<int> nums);

  std::vector<std::vector<int>> solve(Nonogram *puzzle, float scale, bool storeSolution);
  bool solveRow(Nonogram *puzzle, std::vector<tRow> *valid, int row, int size);
  bool solveColumn(Nonogram *puzzle, std::vector<tRow> *valid, int column, int size);
  void trimValidity( std::vector<std::vector<int>> *cells, std::vector<std::vector<tRow>> *validRows, std::vector<std::vector<tRow>> *validCols);

  bool appendRow(
    Nonogram *puzzle, bool isColumn, tRow init, const std::vector<int> pendingNums,
    unsigned int rowSize, int rowIndex, tRow &lastCombo
  );
  bool appendRowEdge(Nonogram *puzzle, bool isColumn, tRow init, const std::vector<int> pendingNums, unsigned int rowSize, int rowIndex, tRow &lastCombo);

  int getEdgeClueIndex(bool isColumn, int rowIndex, int dir);
  int getBound(int edge);

  bool clueScan(Nonogram *puzzle);
  bool edgeFill(Nonogram *puzzle);
  Texture *generateBitmap(std::vector<std::vector<int>> *cells);

  bool checkConflict(bool isColumn, int rowIndex, int index, int value);
  bool checkConflictEdge(Nonogram *puzzle, bool isColumn, int rowIndex, tRow row);
  bool checkConflictMulti(Nonogram *puzzle, bool isColumn, int rowIndex, tRow row, int dir);
  bool checkConflictCross(Nonogram *puzzle, bool isColumn, int rowIndex, int index, int value);

  void render();
  void updateSolvePreview(int x, int y, int val);

  tRow edgeLogic(Nonogram *puzzle, bool isColumn, const std::vector<int> row, int rowIndex, unsigned int rowSize);
  tRow lineSolve(Nonogram *puzzle, bool isColumn, const std::vector<int> row, int rowIndex, unsigned int rowSize);
  tRow getClues(bool isColumn, int rowIndex);
  tRow calcClues(tRow row);
  tRow calcSafeClues(tRow row);
  std::vector<tRow> calcSafeRuns(tRow row);


  void lineLogic(bool isColumn, Nonogram *puzzle, tRow *rowResults, tRow *colResults, int i, tRow *intersects);
private:
  SDL_Renderer *renderer;
  float scale;
  std::vector<int> invalidRowCombos;
  std::vector<int> invalidColCombos;

  bool comboStarted = false;
  bool multiLine = false;
  int comboProgressId = 0;

  int comboIterator;

  int rowComboIndex;
  int colComboIndex;

  std::vector<int> solvedRows;
  std::vector<int> solvedCols;

  std::mutex solutionMutex;
  std::mutex renderMutex;
  std::mutex resultMutex;
  std::mutex intersectMutex;
  std::mutex comboMutex;
};

#endif // Solver_hpp
