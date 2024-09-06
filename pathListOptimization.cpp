#include <iostream>
#include <vector>
#include <random>
#include <tuple>
#include <functional>
#include <ctime>
#include <chrono>
#include <array>

using namespace std;
using namespace std::chrono;

// Edit these values
const int globalSize = 40;
const bool strictLines = true;
const bool debugMode = false;
// From 1-100%
const int density = 100;


class Cell {
public:
    int x;
    int y;
    int state;
    bool blocked;
};


class Maze {
public:
    int sideLength;
    vector<vector<Cell>> cells;
    vector<int> startPos;


    void populateMaze() {
        cells.resize(sideLength);
        for (int y = 0; y < sideLength; ++y) {
            for (int x = 0; x < sideLength; ++x) {
                Cell newCell;
                newCell.x = x;
                newCell.y = y;
                newCell.state = 0;
                if ((x == 0) && (y == 0)) {
                    newCell.blocked = true;
                }
                else if ((x == 0) || (y == 0) || (x == sideLength - 1) || (y == sideLength - 1)) {
                    newCell.blocked = true;
                }
                else {
                    newCell.blocked = false;
                }
                cells[y].push_back(newCell);
            }
        }
        if (strictLines) {
            for (int y = 0; y < sideLength; y += 2) {
                for (int x = 0; x < sideLength; x += 2) {
                    cells[y][x].blocked = true;
                }
            }
        }
    }

    void printMaze() {
        int rowNum = 0;
        for (const auto& row : cells) {
            for (const auto& cell : row) {
                if (cell.state == 1) {
                    if (rowNum == sideLength - 1) {
                        // Goal
                        cout << "🟨";
                    }
                    else if (rowNum == 0) {
                        // Start
                        cout << "🟦";
                    }
                    else {
                        cout << "⬜";
                    }
                }
                else {
                    if (cell.blocked) {
                        if (debugMode) {
                            cout << "❎";
                        }
                        else {
                            cout << "⬛";
                        }

                    }
                    else if (cell.state == 0) {
                        cout << "⬛";
                    }
                    else {
                        // B for broken
                        cout << "🟥";
                    }
                }
                cout << " ";
            }
            cout << "\n";
            ++rowNum;
        }
        cout << "\n\n\n";
    }

    void pickStart() {
        int startY = 0;
        int startX = rand() % sideLength;
        cells[startY][startX].state = 1;
        cells[startY][startX].blocked = false;
        startPos = {startY, startX};
    }

    int calculateDensity() {
        // It doesn't count the blocked cells on the edges
        int amountOfCells = (globalSize - 2) * (globalSize - 2);
        // To not count the blocked cells in between
        if (strictLines) {
            amountOfCells = (amountOfCells * 3) / 4;
        }
        int amountOfPaths = (amountOfCells * density) / 100;
        return amountOfPaths;
    }

    vector<pair<Cell*, int>> checkSurroundings(Cell& cell) {
        // Arbitrary order: clockwise from top: up, right, down, left
        vector<pair<Cell*, int>> report;
        vector<pair<int, int>> directions = {
            {-1, 0},   // nn (north)
            {0, 1},    // ee (east)
            {1, 0},    // ss (south)
            {0, -1}    // ww (west)
        };

        for (const auto& direction : directions) {
            int newY = cell.y + direction.first;
            int newX = cell.x + direction.second;

            // Check if the new position is within bounds
            if (newY >= 0 && newY < sideLength && newX >= 0 && newX < sideLength) {
                Cell* surroundingCell = &cells[newY][newX];
                int state = surroundingCell->blocked ? 3 : surroundingCell->state;
                report.push_back({surroundingCell, state});
            }
        }

        return report;
    }

    bool suitableCarving(Cell& wall) {
        bool suitable = true;
        vector<pair<Cell*, int>> wallStatus = checkSurroundings(wall);
        int surroundingPaths = 0;
        for (const auto& packet : wallStatus) {
            if (packet.second == 1) {
                ++surroundingPaths;
            }
        }
        // If a wall has two or more paths adjacent, it means a loop will be created by carving
        if (surroundingPaths > 1) {
            suitable = false;
        }
        return suitable;
    }

    void carvePath(int depthGoal) {
        vector<array<int, 2>> pathList = {};
        int depth = 0;
        int startY = startPos[0];
        int startX = startPos[1];

        Cell* prevCell = &cells[startY][startX];
        Cell* currCell = &cells[startY][startX];

        while (depth < depthGoal) {
            vector<Cell*> pathCandidates;
            vector<pair<Cell*, int>> surroundingStatus = checkSurroundings(*currCell);

            for (auto& adjacent : surroundingStatus) {
                if (adjacent.first != prevCell && adjacent.second != 3) {
                    bool suitable = suitableCarving(*adjacent.first);
                    if (suitable) {
                        pathCandidates.push_back(adjacent.first);
                    }
                }
            }

            if (!pathCandidates.empty()) {
                Cell* pathChosen = pathCandidates[rand() % pathCandidates.size()];
                int yDiff = pathChosen->y - currCell->y;
                int xDiff = pathChosen->x - currCell->x;
                cells[currCell->y + yDiff][currCell->x + xDiff].state = 1;
                array<int, 2> currCellCoords = {currCell->y + yDiff, currCell->x + xDiff};
                pathList.push_back(currCellCoords);
                prevCell = currCell;
                currCell = pathChosen;
                ++depth;
            }
            else {
                if (!pathList.empty()) {
                    array<int, 2> newStartCellCoords = pathList[rand() % pathList.size()];
                    prevCell = currCell;
                    currCell = &cells[newStartCellCoords[0]][newStartCellCoords[1]];
                } else {
                    break;
                }
            }
        }
    }

    void linkGoal() {
        Cell goalConnectionPoint;
        goalConnectionPoint.x = 999;
        for (int row = sideLength - 1; row >= 0; --row) {
            for (int cellPos = 0; cellPos <= sideLength; ++cellPos) {
                if (cells[row][cellPos].state == 1) {
                    goalConnectionPoint.y = row;
                    goalConnectionPoint.x = cellPos;
                    break;
                }
            }
            if (goalConnectionPoint.x != 999) {
                break;
            }
        }

        for (int yCoord = goalConnectionPoint.y; yCoord < sideLength; ++yCoord) {
            cells[yCoord][goalConnectionPoint.x].state = 1;
        }
    }
};


void generateMaze(int sideLength) {
    // Start measuring time
    auto start = high_resolution_clock::now();

    Maze maze;
    maze.sideLength = sideLength;
    maze.populateMaze();
    if (debugMode) {
        maze.printMaze();
    }
    maze.pickStart();
    maze.carvePath(maze.calculateDensity());
    maze.linkGoal();

    auto stop = high_resolution_clock::now();

    // Not calculate the printing time
    maze.printMaze();

    // Calculate the duration
    auto duration = duration_cast<milliseconds>(stop - start);

    // Print the duration in milliseconds
    cout << "Time taken to generate the maze: " << duration.count() << " ms" << endl;
}

int main() {
    srand(static_cast<unsigned int>(time(0)));
    generateMaze(globalSize);
}
