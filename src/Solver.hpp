#ifndef Solver_hpp
#define Solver_hpp

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

  Texture *progress;

  void init(SDL_Renderer *r)
  {
    renderer = r;
    progress = new Texture();
  }
  void printPuzzle(std::vector<tRow> cells);
  void printCombos(std::vector<tRow> cells);

  int requiredCells(const std::vector<int> nums);

  bool solve(Nonogram *puzzle, float scale);
  bool solveRow(Nonogram *puzzle, std::vector<tRow> *valid, int row, int size);
  bool solveColumn(Nonogram *puzzle, std::vector<tRow> *valid, int column, int size);
  void trimValidity( std::vector<std::vector<int>> *cells, std::vector<std::vector<tRow>> *validRows, std::vector<std::vector<tRow>> *validCols);

  bool appendRow(
    Nonogram *puzzle, bool isColumn, tRow init, const std::vector<int> pendingNums,
    unsigned int rowSize, int rowIndex, tRow &lastCombo, ctpl::thread_pool *lineQueue
  );
  bool appendRowEdge(Nonogram *puzzle, bool isColumn, tRow init, const std::vector<int> pendingNums, unsigned int rowSize, int rowIndex, tRow &lastCombo);

  bool clueScan(Nonogram *puzzle);
  Texture *generateBitmap(std::vector<std::vector<int>> *cells);

  bool checkConflict(bool isColumn, int rowIndex, int index, int value);
  bool checkConflictEdge(Nonogram *puzzle, bool isColumn, int rowIndex, tRow row);
  bool checkConflictCross(Nonogram *puzzle, bool isColumn, int rowIndex, int index, int value);

  void render();
  void updateSolvePreview(int x, int y, int val);

  tRow edgeLogic(Nonogram *puzzle, bool isColumn, const std::vector<int> row, int rowIndex, unsigned int rowSize);
  tRow lineSolve(Nonogram *puzzle, bool isColumn, const std::vector<int> row, int rowIndex, unsigned int rowSize, ctpl::thread_pool *lineQueue);
  tRow getClues(bool isColumn, int rowIndex);
  tRow calcClues(tRow row);
  tRow calcSafeClues(tRow row);
  std::vector<tRow> calcSafeRuns(tRow row);


  void lineLogic(bool isColumn, Nonogram *puzzle, tRow *rowResults, tRow *colResults, int i, tRow *intersects, ctpl::thread_pool *lineQueue);
private:
  SDL_Renderer *renderer;
  float scale;
  std::vector<int> invalidRowCombos;
  std::vector<int> invalidColCombos;

  bool comboStarted = false;
  int comboProgressId = 0;

  int comboIterator;

  int rowComboIndex;
  int colComboIndex;

  std::mutex solutionMutex;
  std::mutex renderMutex;
  std::mutex resultMutex;
  std::mutex intersectMutex;
  std::mutex comboMutex;
};

#endif // Solver_hpp
