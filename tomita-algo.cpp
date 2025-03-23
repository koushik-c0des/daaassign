#include <iostream>
#include <vector>
#include <unordered_set>
#include <map>
#include <fstream>
#include <sstream>
#include <chrono>

using namespace std;
using namespace std::chrono;

class MaximalCliquesFinder {
private:
    int vertexCount;  // Number of vertices
    vector<unordered_set<int>> neighbors;  // Fast adjacency testing
    int largestCliqueSize = 0;
    int totalCliques = 0;
    map<int, int> cliqueSizeDistribution;

public:
    MaximalCliquesFinder() : vertexCount(0) {}

    void detectMaximalCliques() {
        unordered_set<int> currentClique, candidateVertices, excludedVertices;

        // Initialize candidate vertices (only include vertices with neighbors)
        int vertexIndex = 0;
while (vertexIndex < vertexCount) {
    if (!neighbors[vertexIndex].empty())
        candidateVertices.insert(vertexIndex);
    vertexIndex++;
}
        // excludedVertices starts empty
        auto startTime = high_resolution_clock::now();
       
        // Use single implementation for all graph sizes
        expandClique(excludedVertices, candidateVertices, currentClique);
       
        auto endTime = high_resolution_clock::now();
       
        // Display results
        cout << "Number of cliques: " << totalCliques << endl;
        cout << "Maximum clique size: " << largestCliqueSize << endl;
        cout << "Clique size distribution:\n";
       
        auto timeElapsed = duration_cast<milliseconds>(endTime - startTime);
       
        for (auto distIter = cliqueSizeDistribution.rbegin(); distIter != cliqueSizeDistribution.rend(); ++distIter) {
            cout << "Size " << distIter->first << ": " << distIter->second << " cliques" << endl;
        }
       
        cout << "Execution time: " << timeElapsed.count() << " ms" << endl;
    }
   
    void readGraphData(const string& inputFileName) {
        ifstream dataFile(inputFileName);
        if (!dataFile.is_open()) {
            cerr << "Error opening file: " << inputFileName << endl;
            return;
        }

        string textLine;
        int edgeCount = 0, maxVertex = -1;
        bool metadataFound = false;

        // Find header
        while (getline(dataFile, textLine)) {
            // Skip empty lines and comments
            if (textLine.empty() || textLine[0] == '#') {
                // Check if this comment line contains metadata
                if (textLine.find("Nodes:") != string::npos && textLine.find("Edges:") != string::npos) {
                    // Parse metadata line to extract vertex and edge counts
                    stringstream parser(textLine);
                    string skipWord;
                    parser >> skipWord >> skipWord >> vertexCount >> skipWord >> edgeCount;
                    metadataFound = true;
                }
            }
            // If we reach a non-empty, non-comment line, exit the loop
            else {
                break;
            }
        }

        if (!metadataFound) {
            cerr << "Could not find header with node and edge count! Attempting to determine from data." << endl;
            // We'll proceed by inferring vertex count from input data
        }

        // Determine actual max node ID
        dataFile.clear();
        dataFile.seekg(0, ios::beg);
       
        // Continue reading the file line by line
        while (getline(dataFile, textLine)) {
            // Skip lines that are:
            // 1. Empty
            // 2. Start with # (comments)
            // 3. Contain column headers
            if (textLine.empty() ||
                textLine[0] == '#' ||
                textLine.find("FromNodeId") != string::npos) {
                continue;
            }
           
            // Process valid data lines
            stringstream parser(textLine);
            int src, dst;
           
            // Extract source and destination vertices and check if extraction was successful
            if (parser >> src >> dst) {
                // Update the maximum vertex ID if extraction was successful
                maxVertex = max(maxVertex, max(src, dst));
            }
        }

        if (maxVertex < 0) {
            cerr << "No valid edges found in file!" << endl;
            return;
        }

        // Resize adjacency list
        vertexCount = max(vertexCount, maxVertex + 1);
        neighbors.resize(vertexCount);

        // Reset file position
        dataFile.clear();
        dataFile.seekg(0, ios::beg);

        // Read edges
        int validEdgeCount = 0;
        int selfLoopCount = 0;
        int invalidEdgeCount = 0;
       
        while (getline(dataFile, textLine)) {
            // Skip lines that aren't data
            bool skipLine = textLine.empty() ||
                            textLine[0] == '#' ||
                            textLine.find("FromNodeId") != string::npos;
            if (skipLine) continue;
           
            // Parse the edge information
            stringstream parser(textLine);
            int src;
            int dst;
            bool parseSuccess = (parser >> src >> dst) ? true : false;
           
            // Process valid edge data
            if (parseSuccess) {
                // Handle self-loops
                if (src == dst) {
                    selfLoopCount++;
                    continue;
                }
               
                // Handle out-of-range vertices
                bool verticesValid = src >= 0 && dst >= 0 &&
                                    src < vertexCount && dst < vertexCount;
                if (!verticesValid) {
                    invalidEdgeCount++;
                    continue;
                }
               
                // Add valid edge to the graph
                neighbors[src].insert(dst);
                neighbors[dst].insert(src);
                validEdgeCount++;
            }
        }

        cout << "Graph loaded: " << vertexCount << " nodes, " << validEdgeCount << " edges." << endl;
       
        if (invalidEdgeCount > 0)
            cout << "Warning: " << invalidEdgeCount << " edges with invalid vertex IDs were ignored." << endl;
           
        if (selfLoopCount > 0)
            cout << "Note: " << selfLoopCount << " self-loops were ignored." << endl;
           
        dataFile.close();
    }

private:
    void recordClique(const unordered_set<int>& clique) {
        totalCliques++;
        int size = clique.size();

        if (size > largestCliqueSize) largestCliqueSize = size;
        cliqueSizeDistribution[size]++;
    }
   
    // Find a pivot vertex that maximizes connections
    int selectPivot(const unordered_set<int>& candidateVertices, const unordered_set<int>& excludedVertices) {
        int bestVertex = -1, maxConnections = -1;

        // Check candidates for best pivot
        // Check candidates for best pivot
auto candidateIter = candidateVertices.begin();
while (candidateIter != candidateVertices.end()) {
    int vertex = *candidateIter;
    int connectionCount = 0;
   
    auto innerIter = candidateVertices.begin();
    while (innerIter != candidateVertices.end()) {
        if (neighbors[vertex].count(*innerIter))
            connectionCount++;
        innerIter++;
    }
   
    if (connectionCount > maxConnections) {
        maxConnections = connectionCount;
        bestVertex = vertex;
    }
   
    candidateIter++;
}

        // Check excluded vertices
        // Check excluded vertices
auto excludedIter = excludedVertices.begin();
while (excludedIter != excludedVertices.end()) {
    int vertex = *excludedIter;
    int connectionCount = 0;
   
    auto innerIter = candidateVertices.begin();
    while (innerIter != candidateVertices.end()) {
        if (neighbors[vertex].count(*innerIter))
            connectionCount++;
        innerIter++;
    }
   
    if (connectionCount > maxConnections) {
        maxConnections = connectionCount;
        bestVertex = vertex;
    }
   
    excludedIter++;
}

        // Provide fallback if no proper pivot was found but sets aren't empty
        // Provide fallback if no proper pivot was found but sets aren't empty
if (bestVertex == -1) {
    auto candidateIter = candidateVertices.begin();
    if (candidateIter != candidateVertices.end())
        return *candidateIter;
   
    auto excludedIter = excludedVertices.begin();
    if (excludedIter != excludedVertices.end())
        return *excludedIter;
}
        return bestVertex;
    }
   
    // Bron-Kerbosch with pivoting (recursive)
    void expandClique(unordered_set<int>& excludedVertices, unordered_set<int>& candidateVertices, unordered_set<int>& currentClique) {
        if (candidateVertices.empty() && excludedVertices.empty()) {
            recordClique(currentClique);
            return;
        }

        // Choose pivot (only if both sets aren't empty)
        int pivotVertex = -1;
        if (!candidateVertices.empty() || !excludedVertices.empty()) {
            pivotVertex = selectPivot(candidateVertices, excludedVertices);
        }

        // Convert set to vector for safe iteration
        vector<int> vertexList(candidateVertices.begin(), candidateVertices.end());
       
        for (int vertex : vertexList) {
            // Skip vertices connected to pivot (pivoting optimization)
            if (pivotVertex != -1 && pivotVertex >= 0 && pivotVertex < vertexCount &&
                neighbors[pivotVertex].count(vertex)) {
                continue;
            }

            // Add vertex to clique
            currentClique.insert(vertex);

            // Create new candidate and excluded sets
            unordered_set<int> newExcluded, newCandidates;
           
           
            // Create neighbor intersection for candidate vertices
            for (int v : candidateVertices) {
                if (vertex != v && neighbors[vertex].count(v))
                    newCandidates.insert(v);
            }
           
            // Create neighbor intersection for excluded vertices
            for (int v : excludedVertices) {
                if (neighbors[vertex].count(v))
                    newExcluded.insert(v);
            }
           
           
           

            // Recursive call
            expandClique(newExcluded, newCandidates, currentClique);

            // Move vertex
            currentClique.erase(vertex);
            candidateVertices.erase(vertex);
            excludedVertices.insert(vertex);
        }
    }
};

int main() {
    MaximalCliquesFinder cliqueDetector;
    string dataFile = "Email-Enron.txt";
    cout << "Loading graph from file: " << dataFile << endl;
    cliqueDetector.readGraphData(dataFile);
    cliqueDetector.detectMaximalCliques();
    return 0;
}