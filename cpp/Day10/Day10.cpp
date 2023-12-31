#include "Grid.cpp"
#include "Vec2.h"
#include "Direction.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <deque>
#include <set>
#include <regex>
#include <map>



std::vector<Vec2> GetPipeNeighbourOffsets(char& pipeType) {
    std::vector<Vec2> neighbours;
    switch (pipeType) {
    case '|': return std::vector<Vec2>{Vec2(0, -1), Vec2(0, 1) };
    case '-': return std::vector<Vec2>{Vec2(-1, 0), Vec2(1, 0)};
    case 'L': return std::vector<Vec2>{Vec2(0, -1), Vec2(1, 0)};
    case 'J': return std::vector<Vec2>{Vec2(-1, 0), Vec2(0, -1)};
    case '7': return std::vector<Vec2>{Vec2(-1, 0), Vec2(0, 1)};
    case 'F': return std::vector<Vec2>{Vec2(1, 0), Vec2(0, 1)};
    case '.': return std::vector<Vec2>{};
    case 'S': return std::vector<Vec2>{};
    default: return std::vector<Vec2>{};
    }
}

Vec2 FindStartCoordinate(Grid& grid) {
    Vec2 startCoord = Vec2(0, 0);
    for (int x = 0; x < grid.GetWidth(); x++) {
        for (int y = 0; y < grid.GetHeight(); y++) {
            auto cell = grid.GetCellAt(Vec2(x, y));
            if (cell->content == 'S') {
                startCoord = Vec2(x, y);
            }
        }
    }

    return startCoord;
}

bool DoesCellConnectToCell(Cell* cellA, Cell* cellB, Grid& grid) {
    char charA = cellA->content;
    auto offsets = GetPipeNeighbourOffsets(charA);
    for (auto& offset : offsets) {
        Vec2 resultantCoord = offset + cellA->position;
        if (resultantCoord == cellB->position) {
            return true;
        }
    }

    return false;
}

std::vector<Cell*> GetCellsConnectedTo(Vec2 center, Grid& grid) {
    Cell* centerCell = grid.GetCellAt(center);
    char centerChar = centerCell->content;

    std::vector<Cell*> pipeNeighbours{};
    auto neighbourOffsets = GetPipeNeighbourOffsets(centerChar);
    for (auto offset : neighbourOffsets) {
        auto resultantCoord = center + offset;
        auto resultantCell = grid.GetCellAt(resultantCoord);
        pipeNeighbours.push_back(resultantCell);
    }

    return pipeNeighbours;
}

Cell* GetCellConnectedToCenterExcluding(Vec2 center, Grid& grid, Vec2 exclude) {
    auto connectedNeighbours = GetCellsConnectedTo(center, grid);
    std::vector<Cell*> connectedNeighbourPostExclusion{};
    for (auto neighbour : connectedNeighbours)
    {
        if (neighbour->position != exclude)
            connectedNeighbourPostExclusion.push_back(neighbour);
    }
    
    if (connectedNeighbours.size() == 0) {
        throw "Should always have at least one pipe neighbour ahead of us...";
    }
    return connectedNeighbourPostExclusion[0];
}

class PipeWalker {
public:
    Vec2 position;
    Vec2 previousPosition;
    long long steps = 1;

    PipeWalker(Vec2 position, Vec2 previousPosition, Grid& grid) : position(position), previousPosition(previousPosition), grid(grid) {}

    long long StepForward() {
        auto nextPosition = PreviewNextPosition();
        previousPosition = position;
        position = nextPosition;
        steps++;       

        return steps;
    }

    Vec2 PreviewNextPosition() {
        auto nextCell = GetCellConnectedToCenterExcluding(position, grid, previousPosition);
        return nextCell->position;
    }

private:
    Grid& grid;
};

enum class CollinearState {
    None,
    EnteredFromBottom,
    EnteredFromTop
};

int main()
{
    std::cout << "Advent of Code - Day 10!\n";
    
    //std::ifstream ifs("test_input.txt");
    std::ifstream ifs("input.txt");

    std::vector<std::string> parsedLines;
    std::string parsedLine;
    while (ifs.good()) {
        std::getline(ifs, parsedLine);
        parsedLines.push_back(parsedLine);
    }

    Grid grid(parsedLines);
    grid.PrintGrid(false);
    
    Vec2 startCoord = FindStartCoordinate(grid);
    Cell* startCell = grid.GetCellAt(startCoord);
    startCell->isMainLoop = true;
    std::cout << "Found starting coord: " << startCoord << "\n";

    std::vector<Cell*> startingNeighbours = grid.GetNeighbourCells(startCoord);
    std::vector<Cell*> cellsConnectedToStart;
    std::copy_if(startingNeighbours.begin(), startingNeighbours.end(), std::back_inserter(cellsConnectedToStart), [&startCell, &grid](Cell* c) {return DoesCellConnectToCell(c, startCell, grid); });
    std::vector<PipeWalker> pipeWalkers{};
    for (auto cell : cellsConnectedToStart) {
        cell->isMainLoop = true;
        PipeWalker walker(cell->position, startCoord, grid);
        pipeWalkers.push_back(walker);
    }
            
    while (!(pipeWalkers[0].position == pipeWalkers[1].position)) {
        pipeWalkers[0].StepForward();
        pipeWalkers[1].StepForward(); 
        auto cell0 = grid.GetCellAt(pipeWalkers[0].position);
        cell0->isMainLoop = true;
        auto cell1 = grid.GetCellAt(pipeWalkers[1].position);
        cell1->isMainLoop = true;
    }

    grid.PrintGrid(true);

    std::cout << "PART 1 ANSWER - How far does each pipe walker travel? " << pipeWalkers[0].steps << "\n";

    // Part 2 - Counting interior cells
    long long interiorCellCount = 0;
    bool scannerInside = false;
    CollinearState collinearState = CollinearState::None;
    for (int y = 0; y < grid.GetHeight(); y++) {
        collinearState = CollinearState::None;
        scannerInside = false;
        for (int x = 0; x < grid.GetWidth(); x++) {
            Cell* cell = grid.GetCellAt(Vec2(x, y));
            if (cell->isMainLoop) {
                // Consider crossings
                switch (cell->content) {
                case '|': {
                    scannerInside = !scannerInside;
                    break;
                }
                case 'F': {
                    if (collinearState == CollinearState::None)
                        collinearState = CollinearState::EnteredFromBottom;
                    else if (collinearState == CollinearState::EnteredFromBottom)
                        std::cout << "F--F This shouldn't be possible.\n";
                    else if (collinearState == CollinearState::EnteredFromTop)
                        std::cout << "J--F shouldn't be possible.\n";
                    break;
                }
                case 'L': {
                    if (collinearState == CollinearState::None)
                        collinearState = CollinearState::EnteredFromTop;
                    else
                        std::cout << "Something is wrong.\n";
                }
                case '-':
                    if (collinearState == CollinearState::None)
                        std::cout << ".-- should not be possible.\n";
                    break;
                case 'J': {
                    if (collinearState == CollinearState::EnteredFromBottom) {
                        scannerInside = !scannerInside;
                        collinearState = CollinearState::None;
                    }
                    else if (collinearState == CollinearState::EnteredFromTop) {
                        collinearState = CollinearState::None;
                    }
                    break;
                }
                case '7':
                    if (collinearState == CollinearState::EnteredFromTop) {
                        scannerInside = !scannerInside;
                        collinearState = CollinearState::None;
                    }
                    else if (collinearState == CollinearState::EnteredFromBottom) {
                        collinearState = CollinearState::None;
                    }

                }
            }
            else {
                if (scannerInside)
                    interiorCellCount++;
            }
        }
    }

    std::cout << "PART 2 ANSWER - How many cells are contained inside the main loop? " << interiorCellCount << "\n";

    return 0;
}