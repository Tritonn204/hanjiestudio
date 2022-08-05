#include <iostream>
#include <vector>
#include <string>
#include <SDL2/SDL.h>
#include <thread>

#include "Nonogram.hpp"
#include "Solver.hpp"
#include "Console.hpp"
#include "Texture.hpp"
#include "Solver.hpp"
#include "ctpl_stl.h"
#include "mathutil.hpp"

typedef std::vector<int> tRow;

template <typename T1, typename T2>
bool IsSubset(std::vector<T1> const& a, std::vector<T2> const& b) {
   for (typename std::vector<T1>::const_iterator i = a.begin(), y = a.end(); i != y; ++i) {
      bool match = true;

      typename std::vector<T1>::const_iterator ii = i;
      for (typename std::vector<T2>::const_iterator j = b.begin(), z = b.end(); j != z; ++j) {
          if (ii == a.end() || *j != *ii) {
              match = false;
              break;
          }
          ii++;
      }

      if (match)
         return true;
   }

   return false;
}
Solver::Solver() {}

Solver::~Solver() {}

void Solver::printPuzzle(std::vector<tRow> cells){
  std::cout << std::endl;
  for (size_t i = 0; i < cells[0].size(); i++) {
    for (size_t j = 0; j < cells.size(); j++) {
      std::cout << ((cells[j][i] == 1) ? '$' : (cells[j][i] == 2 ? '?' : (cells[j][i] == 0 ? '-' : '/')));
    }
    std::cout << std::endl;
  }
}

void Solver::printCombos(std::vector<tRow> cells){
  std::cout << std::endl;
  for (size_t i = 0; i < cells.size(); i++) {
    if (cells[i].size() < 1) continue;
    for (size_t j = 0; j < cells[0].size(); j++) {
      std::cout << ((cells[i][j] == 1) ? '$' : (cells[i][j] == 2 ? '?' : '-'));
    }
    std::cout << std::endl;
  }
  std::cout << std::endl;
}

int Solver::requiredCells(const std::vector<int> nums){
  int sum = 0;
  for (auto i = nums.begin(); i != nums.end(); ++i){
    sum += (*i + 1); // The number + the at-least-one-cell gap at is right
  }
  return (sum == 0) ? 0 : sum - 1; // The right-most number don't need any gap
}

int Solver::getBound(int edge)
{
  int result;
  bool found = false;
  switch(edge) {
    case SOLVER_BOUND_TOP:
    {
      for(size_t j = 0; j < (*solution)[0].size(); j++) {
        for(size_t i = 0; i < (*solution).size(); i++) {
          if ((*solution)[i][j] == 2) return j;
        }
      }
      break;
    }
    case SOLVER_BOUND_RIGHT:
    {
      for(size_t i = (*solution).size()-1; i > 0; i--) {
        for(size_t j = 0; j < (*solution)[0].size(); j++) {
          if ((*solution)[i][j] == 2) return i;
        }
      }
      break;
    }
    case SOLVER_BOUND_BOTTOM:
    {
      for(size_t j = (*solution)[0].size()-1; j > 0; j--) {
        for(size_t i = 0; i < (*solution).size(); i++) {
          if ((*solution)[i][j] == 2) return j;
        }
      }
      break;
    }
    case SOLVER_BOUND_LEFT:
    {
      for(size_t i = 0; i < (*solution).size(); i++) {
        for(size_t j = 0; j < (*solution)[0].size(); j++) {
          if ((*solution)[i][j] == 2) return i;
        }
      }
      break;
    }
  }
  return 0;
}

std::vector<std::vector<int>> Solver::solve(Nonogram *puzzle, float S)
{
  const auto processor_count = std::thread::hardware_concurrency();
  int threadCount = processor_count;
  if (threadCount == 0) threadCount = 1;
  ctpl::thread_pool lineQueue(threadCount);

  std::cout << threadCount << " threads" << std::endl;

  scale = S;
  solution = new std::vector<std::vector<int>>(puzzle->cells.size(), std::vector<int>(puzzle->cells[0].size(), 2));

  bool looper = true;
  int count = 0;

  progress = new Texture();
  progress->createContext(renderer);
  progress = generateBitmap(solution);
  progress->scale(scale);

  render();

  tRow edgeResults;
  tRow rowResults;
  tRow columnResults;

  int fails = 0;

  int rowEdges[2] = {0, puzzle->rows.size()-1};
  int colEdges[2] = {0, puzzle->columns.size()-1};

  while(looper == true) {
    bool result = false;

    rowResults.clear();
    columnResults.clear();
    edgeResults.clear();
    edgeResults = tRow(4, 2);
    rowResults = tRow(puzzle->rows.size(), 2);
    columnResults = tRow(puzzle->columns.size(), 2);

    std::atomic<int> totalIntersects(0);
    tRow intersects;

    tRow fake(1000);

    log("edge solving");

    for (int i = 0; i < 2; i++) {
      lineQueue.push([&, i](int){
        tRow gen = edgeLogic(puzzle, false, puzzle->rows[rowEdges[i]], rowEdges[i], puzzle->cells.size());
        for(size_t j = 0; j < gen.size(); j++) {
          if (gen[j] != (*solution)[j][rowEdges[i]] && gen[j] < 2) {
            auto&& scopedLock = std::lock_guard< std::mutex >(solutionMutex);
            (void)scopedLock;
            (*solution)[j][rowEdges[i]] = gen[j];
            edgeResults[i] = 1;
            result = true;
            updateSolvePreview(j, rowEdges[i], gen[j]);
          } else {
            edgeResults[i] = 0;
          }
          if ((*solution)[j][rowEdges[i]] == 1) {
            int dir = rowEdges[i] == 0 ? 1 : -1;
            int clue = rowEdges[i] == 0 ? puzzle->columns[j].front() : puzzle->columns[j].back();
            for(int n = 0; n < clue; n++) {
              if ((*solution)[j][rowEdges[i]+(n*dir)] == 2) {
                auto&& scopedLock = std::lock_guard< std::mutex >(solutionMutex);
                (void)scopedLock;
                (*solution)[j][rowEdges[i]+(n*dir)] = 1;
                updateSolvePreview(j, rowEdges[i]+(n*dir), 1);
                edgeResults[i] = 1;
              }
            }
            if (rowEdges[i]+((clue)*dir) > 0 && rowEdges[i]+((clue)*dir) < (*solution)[0].size()){
              if ((*solution)[j][rowEdges[i]+((clue)*dir)] == 2) {
                auto&& scopedLock = std::lock_guard< std::mutex >(solutionMutex);
                (void)scopedLock;
                (*solution)[j][rowEdges[i]+((clue)*dir)] = 0;
                updateSolvePreview(j, rowEdges[i]+((clue)*dir), 0);
                edgeResults[i] = 1;
              }
            }
            render();
          }
        }
        render();
      });
    }

    intersects.clear();
    totalIntersects = 0;

    for (int i = 0; i < 2; i++) {
      lineQueue.push([&, i](int){
        tRow gen = edgeLogic(puzzle, true, puzzle->columns[colEdges[i]], colEdges[i], puzzle->cells[0].size());
        for(size_t j = 0; j < gen.size(); j++) {
          if (gen[j] != (*solution)[colEdges[i]][j] && gen[j] < 2) {
            auto&& scopedLock = std::lock_guard< std::mutex >(solutionMutex);
            (void)scopedLock;
            (*solution)[colEdges[i]][j] = gen[j];
            edgeResults[i+2] = 1;
            updateSolvePreview(colEdges[i], j, gen[j]);
            result = true;
          } else {
            edgeResults[i+2] = 0;
          }
          if ((*solution)[colEdges[i]][j] == 1) {
            int dir = colEdges[i] == 0 ? 1 : -1;
            int clue = colEdges[i] == 0 ? puzzle->rows[j].front() : puzzle->rows[j].back();
            for(int n = 0; n < clue; n++) {
              if ((*solution)[colEdges[i]+(n*dir)][j] == 2) {
                auto&& scopedLock = std::lock_guard< std::mutex >(solutionMutex);
                (void)scopedLock;
                (*solution)[colEdges[i]+(n*dir)][j] = 1;
                updateSolvePreview(colEdges[i]+(n*dir), j, 1);
                edgeResults[i+2] = 1;
              }
            }
            if (colEdges[i]+((clue)*dir) > 0 && colEdges[i]+((clue)*dir) < (*solution).size()){
              if ((*solution)[colEdges[i]+((clue)*dir)][j] == 2) {
                auto&& scopedLock = std::lock_guard< std::mutex >(solutionMutex);
                (void)scopedLock;
                (*solution)[colEdges[i]+((clue)*dir)][j] = 0;
                updateSolvePreview(colEdges[i]+((clue)*dir), j, 0);
                edgeResults[i+2] = 1;
              }
            }
            render();
          }
        }
        render();
      });
    }

    bool isDone = false;
    while (!isDone) {
      SDL_PumpEvents();
      if (
        std::find(edgeResults.begin(), edgeResults.end(),2) == edgeResults.end()
      ) {
        isDone = true;
      }
      SDL_Delay(5);
    }

    log("row scan");

    totalIntersects = 0;
    intersects.clear();

    std::atomic<int> doneRows(0);

    for(size_t i = 0; i < puzzle->cells[0].size(); i++) {
      lineQueue.push([&,i](int){
        lineLogic(false, puzzle, &rowResults, &columnResults, i, &intersects, &lineQueue);
        ++doneRows;
      });
    }

    isDone = false;
    while (!isDone) {
      SDL_PumpEvents();
      if (doneRows % 5 == 0 || doneRows == puzzle->cells[0].size()) {
        for(size_t j = 0; j < intersects.size(); j++) {
          ++totalIntersects;
          lineQueue.push([&,j](int) {
            lineLogic(true, puzzle, &fake, &fake, intersects[j], &fake, &lineQueue);
            --totalIntersects;
          });
        }
        intersects.clear();
      }
      if (
        std::find(rowResults.begin(), rowResults.end(),2) == rowResults.end() &&
        totalIntersects == 0
      ) {
        isDone = true;
      }
      SDL_Delay(5);
    }

    totalIntersects = 0;
    intersects.clear();

    log("column scan");

    std::atomic<int> doneCols(0);

    for(size_t i = 0; i < puzzle->cells.size(); i++) {
      lineQueue.push([&,i](int){
        lineLogic(true, puzzle, &columnResults, &columnResults, i, &intersects, &lineQueue);
        ++doneCols;
      });
    }

    isDone = false;
    while (!isDone) {
      SDL_PumpEvents();
      if (doneCols % 5 == 0 || doneCols == puzzle->cells.size()) {
        for(size_t j = 0; j < intersects.size(); j++) {
          ++totalIntersects;
          lineQueue.push([&,j](int) {
            lineLogic(false, puzzle, &fake, &fake, intersects[j], &fake, &lineQueue);
            --totalIntersects;
          });
        }
        intersects.clear();
      }
      if (
        std::find(columnResults.begin(), columnResults.end(),2) == columnResults.end() &&
        totalIntersects == 0
      ) {
        isDone = true;
      }
      SDL_Delay(5);
    }

    if(
      std::find(edgeResults.begin(), edgeResults.end(),1) != edgeResults.end() ||
      std::find(rowResults.begin(), rowResults.end(),1) != rowResults.end() ||
      std::find(columnResults.begin(), columnResults.end(), 1) != columnResults.end()
    ) {
      result = true;
      fails = 0;
    } else if (fails < 0) {
      result = true;
      fails++;
    }


    if (!result && clueScan(puzzle))
      result = true;

    if (edgeFill(puzzle)) result = true;

    if (result) multiLine = false;

    if (!result && !multiLine){
       multiLine = true;
       result = true;
    }
    looper = result;
  }
  printPuzzle(*solution);
  for(size_t i = 0; i < puzzle->cells.size(); i++) {
    for(size_t j = 0; j < puzzle->cells[0].size(); j++) {
      if ((*solution)[i][j] == 2)
      return *solution;
    }
  }
  progress->~Texture();
  return std::vector<std::vector<int>>();
}

void Solver::lineLogic(bool isColumn, Nonogram *puzzle, tRow *rowResults, tRow *colResults, int i, tRow *intersects, ctpl::thread_pool *lineQueue)
{
  int totalLength = isColumn ? (*solution).size() : (*solution)[0].size();
  tRow fake(totalLength);
  if (!isColumn) {
    // std::cout << "row " << (i+1) << std::endl;
    tRow gen = lineSolve(puzzle, false, puzzle->rows[i], i, puzzle->cells.size(), lineQueue);
    for(size_t j = 0; j < gen.size(); j++) {
      if (gen[j] != (*solution)[j][i] && gen[j] < 2) {
        auto&& scopedLock = std::lock_guard< std::mutex >(solutionMutex);
        (void)scopedLock;
        (*solution)[j][i] = gen[j];
        (*rowResults)[i] = 1;
        updateSolvePreview(j, i, gen[j]);
        if(!std::count((*intersects).begin(), (*intersects).end(), j)) {
          auto&& scopedLock = std::lock_guard< std::mutex >(intersectMutex);
          (void)scopedLock;
          (*intersects).push_back(j);
        }
      }
    }
    render();
    if ((*rowResults)[i] == 2) (*rowResults)[i] = 0;
  } else {
    // std::cout << "column " << (i+1) << std::endl;
    tRow gen = lineSolve(puzzle, true, puzzle->columns[i], i, puzzle->cells[0].size(), lineQueue);
    for(size_t j = 0; j < gen.size(); j++) {
      if (gen[j] != (*solution)[i][j] && gen[j] < 2) {
        auto&& scopedLock = std::lock_guard< std::mutex >(solutionMutex);
        (void)scopedLock;
        (*solution)[i][j] = gen[j];
        (*rowResults)[i] = 1;
        updateSolvePreview(i, j, gen[j]);
        if(!std::count((*intersects).begin(), (*intersects).end(), j)) {
          auto&& scopedLock = std::lock_guard< std::mutex >(intersectMutex);
          (void)scopedLock;
          (*intersects).push_back(j);
        }
      }
    }
    if ((*rowResults)[i] == 2){
      auto&& scopedLock = std::lock_guard< std::mutex >(solutionMutex);
      (void)scopedLock;
      (*rowResults)[i] = 0;
    }
    render();
  }
}

tRow Solver::calcClues(tRow row)
{
  int run = 0;
  tRow fills;
  for(size_t i = 0; i < row.size(); i++) {
    if (row[i] == 1) run++;
    else if (run > 0) {
      fills.push_back(run);
      run = 0;
    }
    if (i == row.size()-1) {
      if (row[i] == 1 || fills.size() == 0) {
        fills.push_back(run);
        run = 0;
      }
    }
  }
  return fills;
}

std::vector<tRow> Solver::calcSafeRuns(tRow row)
{
  int freeSpace = 0;
  int run = 0;
  bool canCalc = true;
  std::vector<tRow> result;
  tRow fills;
  for(size_t i = 0; i < row.size(); i++) {
    if (row[i] == 0) canCalc = true;
    if (row[i] == 1) {
      freeSpace = 0;
      if (canCalc)
        run++;
    }
    else if (run > 0) {
      if (row[i] != 2) {
        fills.push_back(run);
      } else {
        canCalc = false;
        if (fills.size() > 0) {
          result.push_back(fills);
          fills.clear();
        }
      }
      run = 0;
      freeSpace = 0;
    } else if (row[i] == 2) {
      if (fills.size() > 0) {
        result.push_back(fills);
        fills.clear();
      }
      run = 0;
      canCalc = false;
      freeSpace++;
    }
    if (i == row.size()-1) {
      if (freeSpace > 0) {
      } else
      if (row[i] == 1 || fills.size() == 0) {
        if (canCalc) {
          fills.push_back(run);
          run = 0;
        }
      }
      if (fills.size() > 0 && canCalc) {
        result.push_back(fills);
        fills.clear();
      }
    }
  }
  // log(row);
  // log(fills);
  return result;
}

tRow Solver::calcSafeClues(tRow row)
{
  int freeSpace = 0;
  int run = 0;
  bool canCalc = true;
  tRow fills;
  for(size_t i = 0; i < row.size(); i++) {
    if (row[i] == 0) canCalc = true;
    if (row[i] == 1) {
      if (freeSpace > 0) {
        fills.push_back(freeSpace*1000);
      }
      freeSpace = 0;
      if (canCalc)
        run++;
    }
    else if (run > 0) {
      if (row[i] != 2) {
        fills.push_back(run);
      } else {
        canCalc = false;
      }
      run = 0;
      freeSpace = 0;
    } else if (row[i] == 2) {
      run = 0;
      canCalc = false;
      freeSpace++;
    }
    if (i == row.size()-1) {
      if (freeSpace > 0) {
        fills.push_back(freeSpace*1000);
      } else
      if (row[i] == 1 || fills.size() == 0) {
        if (canCalc) {
          fills.push_back(run);
          run = 0;
        }
      }
    }
  }
  // log(row);
  // log(fills);
  return fills;
}


tRow Solver::getClues(bool isColumn, int rowIndex)
{
  int run = 0;
  tRow fills;
  if (!isColumn) {
    for(size_t i = 0; i < (*solution).size(); i++) {
      if ((*solution)[i][rowIndex] == 1) run++;
      else if ((*solution)[i][rowIndex] != 1 && run > 0) {
        fills.push_back(run);
        run = 0;
      }
      if (i == (*solution).size()-1) {
        if ((*solution)[i][rowIndex] == 1 || fills.size() == 0) {
          fills.push_back(run);
          run = 0;
        }
      }
    }
  } else {
    for(size_t i = 0; i < (*solution)[0].size(); i++) {
      if ((*solution)[rowIndex][i] == 1) run++;
      else if ((*solution)[rowIndex][i] != 1 && run > 0) {
        fills.push_back(run);
        run = 0;
      }
      if (i == (*solution)[0].size()-1) {
        if ((*solution)[rowIndex][i] == 1 || fills.size() == 0) {
          fills.push_back(run);
          run = 0;
        }
      }
    }
  }
  return fills;
}

bool Solver::clueScan(Nonogram *puzzle)
{
  bool resolved = false;
  for(size_t j = 0; j < (*solution)[0].size(); j++) {
    int run = 0;
    tRow fills = getClues(false, j);

    if (fills == puzzle->rows[j]) {
      for(size_t i = 0; i < (*solution).size(); i++) {
        if ((*solution)[i][j] == 2) {
          auto&& scopedLock = std::lock_guard< std::mutex >(solutionMutex);
          (void)scopedLock;
          resolved = true;
          (*solution)[i][j] = 0;
          updateSolvePreview(i,j,0);
        }
      }
    }
    //if (resolved)
    //std::cout << "row " << j << " resolved" << std::endl;
  }
  for(size_t j = 0; j < (*solution).size(); j++) {
    int run = 0;
    tRow fills = getClues(true, j);
    if (fills == puzzle->columns[j]) {
      for(size_t i = 0; i < (*solution)[0].size(); i++) {
        if ((*solution)[j][i] == 2) {
          auto&& scopedLock = std::lock_guard< std::mutex >(solutionMutex);
          (void)scopedLock;
          resolved = true;
          (*solution)[j][i] = 0;
          updateSolvePreview(j,i,0);
        }
      }
    }
    //if (resolved)
    //std::cout << "column " << j << " resolved" << std::endl;
  }
  return resolved;
}

void Solver::updateSolvePreview(int x, int y, int val)
{
  // std::cout << (val==3) << std::endl;
  auto&& scopedLock = std::lock_guard< std::mutex >(renderMutex);
  (void)scopedLock;
  const char* oldHint = SDL_GetHint(SDL_HINT_RENDER_SCALE_QUALITY);
  progress->setAsRenderTarget();
  SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest");
  progress->scale(1);
  SDL_Color colors[3] = {
    SDL_Color{255,255,255,255},
    SDL_Color{0,0,0,255},
    SDL_Color{245,176,66,255},
  };
  SDL_SetRenderDrawColor(
    renderer,
    colors[val].r,
    colors[val].g,
    colors[val].b,
    colors[val].a
  );
  SDL_RenderDrawPoint(renderer, x, y);
  progress->scale(scale);
  SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, oldHint);
  SDL_SetRenderTarget(renderer, NULL);
}

tRow getFirst(std::vector<tRow> t)
{
  if (t.size() > 0) return t[0];
  return tRow();
}

bool Solver::checkConflict(bool isColumn, int rowIndex, int index, int value)
{
  int comparison = isColumn ? (*solution)[rowIndex][index] : (*solution)[index][rowIndex];
  if (value == 3 && comparison != 2) return true;
  if (value != 3 && comparison != 2 && value != 2 && comparison != value)
  return true;
  return false;
}

bool Solver::checkConflictCross(Nonogram *puzzle, bool isColumn, int rowIndex, int index, int val)
{
  int totalLength = isColumn ? (*solution).size() : (*solution)[0].size();
  tRow comparison = (isColumn ? puzzle->rows[index] : puzzle->columns[index]);
  bool unsolved = false;
  tRow tempRow(totalLength, 3);
  if (!isColumn) {
    for(int i = 0; i < totalLength; i++) {
      if ((*solution)[index][i] == 2) unsolved = true;
      tempRow[i] = (*solution)[index][i];
    }
    tempRow[rowIndex] = val;
    tRow newClues = calcSafeClues(tempRow);
    tRow confirmedClues;
    for (int n : newClues) {
      if (n < 1000) {
        confirmedClues.push_back(n);
      }
    }
    if (!unsolved && confirmedClues.size() < comparison.size()) return true;
    else if (confirmedClues.size() > comparison.size()) return true;
    for(int n : confirmedClues) {
      if (std::find(comparison.begin(), comparison.end(), n) == comparison.end()) return true;
    }
  } else {
    for(int i = 0; i < totalLength; i++) {
      if ((*solution)[i][index] == 2) unsolved = true;
      tempRow[i] = (*solution)[i][index];
    }
    tempRow[rowIndex] = val;
    tRow newClues = calcSafeClues(tempRow);
    tRow confirmedClues;
    for (int n : newClues) {
      if (n < 1000) {
        confirmedClues.push_back(n);
      }
    }
    if (!unsolved && confirmedClues.size() < comparison.size()) return true;
    else if (confirmedClues.size() > comparison.size()) return true;
    for(int n : confirmedClues) {
      if (std::find(comparison.begin(), comparison.end(), n) == comparison.end()) return true;
    }
  }
  return false;
}

bool Solver::edgeFill(Nonogram *puzzle)
{
  log("edge fill");
  bool resolved = false;
  for (size_t i = 0; i < puzzle->columns.size(); i++) {
    int index = 0;
    int inset = 0;
    int inClue = false;
    for (size_t j = 0; j < puzzle->rows.size(); j++) {
      if ((*solution)[i][j] == 0 && inClue){
        index++;
        inClue = false;
      } else if((*solution)[i][j] == 1 && !inClue){
        inClue = true;
        inset = j;
      }
      else if ((*solution)[i][j] == 2 && inClue) {
        for(int n = 0; n < puzzle->columns[i][index]; n++) {
          if ((*solution)[i][inset+n] == 2) {
            resolved = true;
            (*solution)[i][inset+n] = 1;
            updateSolvePreview(i, inset+n, 1);
          }
        }
        if (
          inset+puzzle->columns[i][index] < puzzle->rows.size() &&
          (*solution)[i][inset+puzzle->columns[i][index]] == 2
        ) {
          resolved = true;
          (*solution)[i][inset+puzzle->columns[i][index]] = 0;
          updateSolvePreview(i, inset+puzzle->columns[i][index], 0);
        }
        break;
      } else if ((*solution)[i][j] == 2) break;
    }
    index = puzzle->columns[i].size()-1;
    inset = puzzle->rows.size()-1;
    inClue = false;
    for (size_t j = puzzle->rows.size()-1; j > 0; j--) {
      if ((*solution)[i][j] == 0 && inClue){
        index--;
        inClue = false;
      } else if ((*solution)[i][j] == 1 && !inClue) {
        inClue = true;
        inset = j;
      }
      else if ((*solution)[i][j] == 2 && inClue) {
        for(int n = 0; n < puzzle->columns[i][index]; n++) {
          if ((*solution)[i][inset-n] == 2) {
            resolved = true;
            (*solution)[i][inset-n] = 1;
            updateSolvePreview(i, inset-n, 1);
          }
        }
        if (
          inset+puzzle->columns[i][index] > 0 &&
          (*solution)[i][inset-puzzle->columns[i][index]] == 2
        ) {
          resolved = true;
          (*solution)[i][inset-puzzle->columns[i][index]] = 0;
          updateSolvePreview(i, inset-puzzle->columns[i][index], 0);
        }
        break;
      } else if ((*solution)[i][j] == 2) break;
    }
  }
  for (size_t j = 0; j < puzzle->rows.size(); j++) {
    int index = 0;
    int inset = 0;
    int inClue = false;
    for (size_t i = 0; i < puzzle->columns.size(); i++) {
      if ((*solution)[i][j] == 0 && inClue){
        index++;
        inClue = false;
      } else if((*solution)[i][j] == 1 && !inClue){
        inClue = true;
        inset = i;
      }
      else if ((*solution)[i][j] == 2 && inClue) {
        for(int n = 0; n < puzzle->rows[j][index]; n++) {
          if ((*solution)[inset+n][j] == 2) {
            resolved = true;
            (*solution)[inset+n][j] = 1;
            updateSolvePreview(inset+n, j, 1);
          }
        }
        if (
          inset+puzzle->rows[j][index] < puzzle->columns.size() &&
          (*solution)[inset+puzzle->rows[j][index]][j] == 2
        ) {
          resolved = true;
          (*solution)[inset+puzzle->rows[j][index]][j] = 0;
          updateSolvePreview(inset+puzzle->rows[j][index], j, 0);
        }
        break;
      } else if ((*solution)[i][j] == 2) break;
    }
    index = puzzle->rows[j].size()-1;
    inset = puzzle->columns.size()-1;
    inClue = false;
    for (size_t i = puzzle->columns.size()-1; i > 0; i--) {
      if ((*solution)[i][j] == 0 && inClue){
        index--;
        inClue = false;
      } else if((*solution)[i][j] == 1 && !inClue){
        inClue = true;
        inset = i;
      }
      else if ((*solution)[i][j] == 2 && inClue) {
        for(int n = 0; n < puzzle->rows[j][index]; n++) {
          if ((*solution)[inset-n][j] == 2) {
            resolved = true;
            (*solution)[inset-n][j] = 1;
            updateSolvePreview(inset-n, j, 1);
          }
        }
        if (
          inset-puzzle->rows[j][index] > 0 &&
          (*solution)[inset-puzzle->rows[j][index]][j] == 2
        ) {
          resolved = true;
          (*solution)[inset-puzzle->rows[j][index]][j] = 0;
          updateSolvePreview(inset-puzzle->rows[j][index], j, 0);
        }
        break;
      } else if ((*solution)[i][j] == 2) break;
    }
  }
  return resolved;
}

int Solver::getEdgeClueIndex(bool isColumn, int rowIndex, int dir)
{
  int run = 0;
  int index = 0;
  int totalLength = isColumn ? (*solution)[0].size() : (*solution).size();
  if (!isColumn) {
    if (dir > 0) for(int i = 0; i < totalLength; i++) {
      if ((*solution)[i][rowIndex] == 2) return index;
      else if ((*solution)[i][rowIndex] == 1) run++;
      else if ((*solution)[i][rowIndex] == 0 && run > 0) {
        run = 0;
        index++;
      }
    } else for(int i = totalLength-1; i > 0; i--) {
      if ((*solution)[i][rowIndex] == 2) return index;
      else if ((*solution)[i][rowIndex] == 1) run++;
      else if ((*solution)[i][rowIndex] == 0 && run > 0) {
        run = 0;
        index--;
      }
    }
  } else {
    if (dir > 0) for(int i = 0; i < totalLength; i++) {
      if ((*solution)[rowIndex][i] == 2) return index;
      else if ((*solution)[rowIndex][i] == 1) run++;
      else if ((*solution)[rowIndex][i] == 0 && run > 0) {
        run = 0;
        index++;
      }
    } else for(int i = totalLength-1; i > 0; i--) {
      if ((*solution)[rowIndex][i] == 2) return index;
      else if ((*solution)[rowIndex][i] == 1) run++;
      else if ((*solution)[rowIndex][i] == 0 && run > 0) {
        run = 0;
        index--;
      }
    }
  }
  return index;
}



bool Solver::checkConflictMulti(Nonogram *puzzle, bool isColumn, int rowIndex, tRow row, int dir)
{
  if (std::find(row.begin(), row.end(), 3) != row.end()) return false;

  tRow comparison = (isColumn ? puzzle->columns[rowIndex+dir] : puzzle->rows[rowIndex+dir]);

  int sum = 0;
  int compSum = 0;
  for(int n : row) {
    sum += n;
  }
  if (sum <= 1) return false;
  for(int n : comparison) {
    compSum += n;
  }

  if (compSum > sum) return false;

  tRow tempRow(row.size(),3);

  if (!isColumn) {
    for(size_t i = 0; i < row.size(); i++) {
      tempRow[i] = (*solution)[i][rowIndex+dir];
    }
    for(size_t i = 0; i < row.size(); i++) {
      tRow crossRow = puzzle->columns[i];
      int clueComp = crossRow[(dir > 0 ? 0 : crossRow.size()-1) + getEdgeClueIndex(true, i, dir)];
      if (row[i] == 1) {
        int tally = 1;
        for (int n = 1; n < clueComp; n++) {
          if (rowIndex+(n*(-dir)) < 0 || rowIndex+(n*(-dir)) >= puzzle->rows.size()) break;
          if ((*solution)[i][rowIndex+(n*(-dir))] != 1) break;
          tally++;
        }
        if (clueComp > tally) {
          if (tempRow[i] == 0) return true;
          tempRow[i] = 1;
        }
        else if (tempRow[i] == 1) return true;
        else {
          if (clueComp == tally)
            tempRow[i] = 0;
          else return true;
        }
      }
    }
    tRow newClues = calcSafeClues(tempRow);
    tRow runs = calcClues(tempRow);
    std::vector<tRow> runVector = calcSafeRuns(tempRow);
    tRow confirmedClues;
    tRow unsolvedClues = comparison;
    int largestSpace = 0;
    int largestClue = 0;
    for(int n : newClues) {
      if (n < 1000) {
        confirmedClues.push_back(n);
        unsolvedClues.erase(std::remove(unsolvedClues.begin(), unsolvedClues.end(), n), unsolvedClues.end());
      } else {
        if (n >= 1000 && largestSpace*1000 < n) largestSpace = n/1000;
      }
    }
    int trueFillCount = 0;
    int tempFillCount = 0;
    for(int n : comparison) {
      if (largestClue < n) largestClue = n;
      trueFillCount += n;
    }

    for(int n : runs) {
      if(n > largestClue) return true;
      tempFillCount += n;
    }

    for(tRow run : runVector) {
      if (!IsSubset(comparison,run))
        return true;
    }

    if (tempFillCount > trueFillCount) return true;

    bool spaceFree = true;
    for(int n : confirmedClues) {
      if (std::find(comparison.begin(), comparison.end(), n) == comparison.end()) {
        return true;
      }
    }
    if(confirmedClues.size() > comparison.size()) return true;
  }
  else {
    for(size_t i = 0; i < row.size(); i++) {
      tempRow[i] = (*solution)[rowIndex+dir][i];
    }
    for(size_t i = 0; i < row.size(); i++) {
      tRow crossRow = puzzle->rows[i];
      int clueComp = crossRow[(dir > 0 ? 0 : crossRow.size()-1) + getEdgeClueIndex(false, i, dir)];
      if (row[i] == 1) {
        int tally = 1;
        for (int n = 1; n < clueComp; n++) {
          if (rowIndex+(n*(-dir)) < 0 || rowIndex+(n*(-dir)) >= puzzle->columns.size()) break;
          if ((*solution)[rowIndex+(n*(-dir))][i] != 1) break;
          tally++;
        }
        if (clueComp > tally) {
          if (tempRow[i] == 0) return true;
          tempRow[i] = 1;
        }
        else if (tempRow[i] == 1) return true;
        else {
          if (clueComp == tally)
            tempRow[i] = 0;
          else return true;
        }
      }
    }
    tRow newClues = calcSafeClues(tempRow);
    tRow runs = calcClues(tempRow);
    std::vector<tRow> runVector = calcSafeRuns(tempRow);
    tRow confirmedClues;
    tRow unsolvedClues = comparison;
    int largestSpace = 0;
    int largestClue = 0;
    for(int n : newClues) {
      if (n < 1000) {
        confirmedClues.push_back(n);
      } else {
        if (n >= 1000 && largestSpace*1000 < n) largestSpace = n/1000;
      }
    }
    int trueFillCount = 0;
    int tempFillCount = 0;
    for(int n : comparison) {
      if (largestClue < n) largestClue = n;
      trueFillCount += n;
    }

    for(int n : runs) {
      if(n > largestClue) return true;
      tempFillCount += n;
    }

    for(tRow run : runVector) {
      if (!IsSubset(comparison,run))
        return true;
    }

    if (tempFillCount > trueFillCount) return true;
    bool spaceFree = true;
    for(int n : confirmedClues) {
      if (std::find(comparison.begin(), comparison.end(), n) == comparison.end()) {
        return true;
      }
    }
    if(confirmedClues.size() > comparison.size()) return true;
  }
  return false;
}

bool Solver::checkConflictEdge(Nonogram *puzzle, bool isColumn, int rowIndex, tRow row)
{
  if (std::find(row.begin(), row.end(), 3) != row.end()) return false;

  int dir = (rowIndex > 0 ? -1 : 1);
  tRow comparison = (isColumn ? puzzle->columns[rowIndex+dir] : puzzle->rows[rowIndex+dir]);

  int sum = 0;
  int compSum = 0;
  for(int n : row) {
    sum += n;
  }
  if (sum <= 1) return false;
  for(int n : comparison) {
    compSum += n;
  }

  if (compSum > sum) return false;

  tRow tempRow(row.size(),3);
  if (!isColumn) {
    for(size_t i = 0; i < row.size(); i++) {
      tempRow[i] = (*solution)[i][rowIndex+dir];
    }
    for(size_t i = 0; i < row.size(); i++) {
      tRow crossRow = puzzle->columns[i];
      int clueComp = (dir > 0 ? crossRow.front() : crossRow.back());
      if (row[i] == 1) {
        if (clueComp > 1) {
          if (tempRow[i] == 0) return true;
          tempRow[i] = 1;
        }
        else if (tempRow[i] == 1) return true;
        else {
          if (clueComp == 1)
            tempRow[i] = 0;
          else return true;
        }
      }
    }
    tRow newClues = calcSafeClues(tempRow);
    tRow runs = calcClues(tempRow);
    std::vector<tRow> runVector = calcSafeRuns(tempRow);
    tRow confirmedClues;
    tRow unsolvedClues = comparison;
    int largestSpace = 0;
    int largestClue = 0;
    for(int n : newClues) {
      if (n < 1000) {
        confirmedClues.push_back(n);
        unsolvedClues.erase(std::remove(unsolvedClues.begin(), unsolvedClues.end(), n), unsolvedClues.end());
      } else {
        if (n >= 1000 && largestSpace*1000 < n) largestSpace = n/1000;
      }
    }
    int trueFillCount = 0;
    int tempFillCount = 0;
    for(int n : comparison) {
      if (largestClue < n) largestClue = n;
      trueFillCount += n;
    }

    for(int n : runs) {
      if(n > largestClue) return true;
      tempFillCount += n;
    }

    for(tRow run : runVector) {
      if (!IsSubset(comparison,run))
        return true;
    }

    if (tempFillCount > trueFillCount) return true;

    bool spaceFree = true;
    for(int n : confirmedClues) {
      if (std::find(comparison.begin(), comparison.end(), n) == comparison.end()) {
        return true;
      }
    }
    if(confirmedClues.size() > comparison.size()) return true;
  }
  else {
    for(size_t i = 0; i < row.size(); i++) {
      tempRow[i] = (*solution)[rowIndex+dir][i];
    }
    for(size_t i = 0; i < row.size(); i++) {
      tRow crossRow = puzzle->rows[i];
      int clueComp = (dir > 0 ? crossRow.front() : crossRow.back());
      if (row[i] == 1) {
        if (clueComp > 1) {
          if (tempRow[i] == 0) return true;
          tempRow[i] = 1;
        }
        else if (tempRow[i] == 1) return true;
        else {
          if (clueComp == 1)
            tempRow[i] = 0;
          else return true;
        }
      }
    }
    tRow newClues = calcSafeClues(tempRow);
    tRow runs = calcClues(tempRow);
    std::vector<tRow> runVector = calcSafeRuns(tempRow);
    tRow confirmedClues;
    tRow unsolvedClues = comparison;
    int largestSpace = 0;
    int largestClue = 0;
    for(int n : newClues) {
      if (n < 1000) {
        confirmedClues.push_back(n);
        unsolvedClues.erase(std::remove(unsolvedClues.begin(), unsolvedClues.end(), n), unsolvedClues.end());
      } else {
        if (n >= 1000 && largestSpace*1000 < n) largestSpace = n/1000;
      }
    }
    int trueFillCount = 0;
    int tempFillCount = 0;
    for(int n : comparison) {
      if (largestClue < n) largestClue = n;
      trueFillCount += n;
    }

    for(int n : runs) {
      if(n > largestClue) return true;
      tempFillCount += n;
    }

    for(tRow run : runVector) {
      if (!IsSubset(comparison,run))
        return true;
    }

    if (tempFillCount > trueFillCount) return true;
    bool spaceFree = true;
    for(int n : confirmedClues) {
      if (std::find(comparison.begin(), comparison.end(), n) == comparison.end()) {
        return true;
      }
    }
    if(confirmedClues.size() > comparison.size()) return true;
  }
  return false;
}

bool Solver::appendRow(
  Nonogram *puzzle, bool isColumn, tRow init, const std::vector<int> pendingNums,
  unsigned int rowSize, int rowIndex, tRow &lastCombo, ctpl::thread_pool *lineQueue
)
{
  int totalLength = (isColumn ? (*solution)[0].size() : (*solution).size());
  if (pendingNums.size() <= 0){
    while (init.size() < (isColumn ? (*solution)[0].size() : (*solution).size()))
      init.push_back(0);
    if(multiLine == true && rowIndex > 0 && rowIndex < (!isColumn ? (*solution)[0].size()-1 : (*solution).size()-1)) {
      if (!isColumn) {
        if (
          rowIndex == getBound(SOLVER_BOUND_TOP) &&
          checkConflictMulti(puzzle, false, rowIndex, init, 1)
        ) {
          log(rowIndex);
          log("invalid edge row looking downward");
          return false;
        }
        else if (
          rowIndex == getBound(SOLVER_BOUND_BOTTOM) &&
          checkConflictMulti(puzzle, false, rowIndex, init, -1)
        ) {
          log(rowIndex);
          log("invalid edge row looking upward");
          return false;
        }
      }
      else {
        if (
          rowIndex == getBound(SOLVER_BOUND_LEFT) &&
          checkConflictMulti(puzzle, isColumn, rowIndex, init, 1)
        ) {
          log(rowIndex);
          log("invalid edge col looking right");
          return false;
        }
        else if (
          rowIndex == getBound(SOLVER_BOUND_RIGHT) &&
          checkConflictMulti(puzzle, isColumn, rowIndex, init, -1)
        ) {
          log(rowIndex);
          log("invalid edge col looking left");
          return false;
        }
      }
    }

    for(int i = 0; i < totalLength; i++) {
      if (checkConflict(isColumn, rowIndex, i, init[i])){
        return false;
      }
      if (multiLine)
        if (checkConflictCross(puzzle, isColumn, rowIndex, i, init[i]))
          return false;
      if (init[i] != lastCombo[i] && lastCombo[i] != 3) lastCombo[i] = 2;
      else {
        lastCombo[i] = init[i];
      }
    }
    return false;
  }
  if (pendingNums[0] == totalLength) {
    lastCombo = tRow(totalLength, 1);
    return false;
  }
  int cellsRequired = requiredCells(pendingNums);
  if (cellsRequired > rowSize){
    return false;   // There are no combinations
  }
  tRow prefix;
  int gapSize = 0;
  std::vector<int> pNumsAux = pendingNums;
  pNumsAux.erase(pNumsAux.begin());
  unsigned int space = rowSize;
  while ((gapSize + cellsRequired) <= rowSize){
    space = rowSize;
    space -= gapSize;
    prefix.clear();
    prefix = init;
    bool conflict = false;
    int index = prefix.size();
    for (int i = 0; i < gapSize; ++i){
      conflict = !conflict ? checkConflict(isColumn, rowIndex, index, 0) : conflict;
      if (!conflict) {
        prefix.push_back(0);
      }
      ++index;
    }
    for (int i = 0; i < pendingNums[0]; ++i){
      conflict = !conflict ? checkConflict(isColumn, rowIndex, index, 1) : conflict;
      if (!conflict) {
        prefix.push_back(1);
      }
      ++index;
      space--;
    }
    if (space > 0){
      conflict = !conflict ? checkConflict(isColumn, rowIndex, index, 0) : conflict;
      if (!conflict) {
        prefix.push_back(0);
      }
      ++index;
      space--;
    }
    if (!conflict)
      appendRow(puzzle, isColumn, prefix, pNumsAux, space, rowIndex, lastCombo, lineQueue);
    ++gapSize;
  }
  return true;
}

bool Solver::appendRowEdge(Nonogram *puzzle, bool isColumn, tRow init, const std::vector<int> pendingNums, unsigned int rowSize, int rowIndex, tRow &lastCombo){
  int totalLength = (isColumn ? (*solution)[0].size() : (*solution).size());
  if (pendingNums.size() <= 0){
    while (init.size() < (isColumn ? (*solution)[0].size() : (*solution).size()))
      init.push_back(0);
    if (checkConflictEdge(puzzle, isColumn, rowIndex, init)) {
      return false;
    }
    for(int i = 0; i < totalLength; i++) {
      if (checkConflict(isColumn, rowIndex, i, init[i])) {
        return false;
      }
      // if (checkConflictCross(puzzle, isColumn, rowIndex, i, init[i]));
      //   return false;
      if (init[i] != lastCombo[i] && lastCombo[i] != 3) lastCombo[i] = 2;
      else {
        lastCombo[i] = init[i];
      }
    }
    // log(init);
    return false;
  }
  if (pendingNums[0] == totalLength) {
    lastCombo = tRow(totalLength, 1);
    return false;
  }
  int cellsRequired = requiredCells(pendingNums);
  if (cellsRequired > rowSize){
    return false;   // There are no combinations
  }
  tRow prefix;
  int gapSize = 0;
  std::vector<int> pNumsAux = pendingNums;
  pNumsAux.erase(pNumsAux.begin());
  unsigned int space = rowSize;
  while ((gapSize + cellsRequired) <= rowSize){
    space = rowSize;
    space -= gapSize;
    prefix.clear();
    prefix = init;
    bool conflict = false;
    int index = prefix.size();
    if (index >= totalLength) break;
    for (int i = 0; i < gapSize; ++i){
      conflict = !conflict ? checkConflict(isColumn, rowIndex, index, 0) : conflict;
      if (!conflict) {
        prefix.push_back(0);
      }
      ++index;
    }
    for (int i = 0; i < pendingNums[0]; ++i){
      conflict = !conflict ? checkConflict(isColumn, rowIndex, index, 1) : conflict;
      if (!conflict) {
        prefix.push_back(1);
      }
      ++index;
      space--;
    }
    if (space > 0){
      conflict = !conflict ? checkConflict(isColumn, rowIndex, index, 0) : conflict;
      if (!conflict) {
        prefix.push_back(0);
      }
      ++index;
      space--;
    }
    if (!conflict)
      appendRowEdge(puzzle, isColumn, prefix, pNumsAux, space, rowIndex, lastCombo);
    ++gapSize;
  }
  return true;
}

Texture *Solver::generateBitmap(std::vector<std::vector<int>> *solution)
{
  float scaleX, scaleY;
  SDL_RenderGetScale(renderer, &scaleX, &scaleY);

  Texture *t = new Texture();
  const char* oldHint = SDL_GetHint(SDL_HINT_RENDER_SCALE_QUALITY);
  SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest");
  t->createContext(renderer);
  t->createBlank((*solution).size(), (*solution)[0].size(), SDL_TEXTUREACCESS_TARGET);

  t->setAsRenderTarget();
  SDL_RenderClear(renderer);
  SDL_RenderSetScale(renderer,1,1);

  SDL_Color colors[3] = {
    SDL_Color{255,255,255,255},
    SDL_Color{0,0,0,255},
    SDL_Color{245,176,66,255},
  };

  for (size_t i = 0; i < (*solution).size(); i++) {
    for(size_t j = 0; j < (*solution)[i].size(); j++) {
      SDL_SetRenderDrawColor(
        renderer,
        colors[(*solution)[i][j]].r,
        colors[(*solution)[i][j]].g,
        colors[(*solution)[i][j]].b,
        colors[(*solution)[i][j]].a
      );
      SDL_RenderDrawPoint(renderer, i, j);
    }
  }
  SDL_Point p = SDL_Point{0,0};
  SDL_SetRenderTarget(renderer,NULL);
  SDL_RenderSetScale(renderer, scaleX,scaleY);
  t->render(0, 0, NULL, 0.0);
  SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, oldHint);
  return t;
}

void Solver::render()
{
  auto&& scopedLock = std::lock_guard< std::mutex >(renderMutex);
  (void)scopedLock;
  progress->render(0, 0);
  SDL_RenderPresent(renderer);
}

tRow Solver::lineSolve(Nonogram *puzzle, bool isColumn, const std::vector<int> row, int rowIndex, unsigned int rowSize, ctpl::thread_pool *lineQueue) {
  tRow lastCombo((isColumn ? (*solution)[0].size() : (*solution).size()), 3);
  tRow init;
  appendRow(puzzle, isColumn, init, row, rowSize, rowIndex, lastCombo, lineQueue);
  // bool done = false;
  // while(!done) {
  //   SDL_PumpEvents();
  //   if (taskCount <= 0) done = true;
  //   SDL_Delay(5);
  // }
  return lastCombo;
}


tRow Solver::edgeLogic(Nonogram *puzzle, bool isColumn, const std::vector<int> row, int rowIndex, unsigned int rowSize)
{
  tRow lastCombo((isColumn ? (*solution)[0].size() : (*solution).size()), 3);
  tRow init;
  appendRowEdge(puzzle, isColumn, init, row, rowSize, rowIndex, lastCombo);
  return lastCombo;
}
