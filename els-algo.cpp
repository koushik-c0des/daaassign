#include <iostream>
#include <vector>
#include <set>
#include <unordered_set>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <string>
#include <chrono>
#include <queue>
#include <map>
#include <climits>
#include <unordered_map>

using namespace std;
using namespace std::chrono;

class GraphCliqueAnalyzer {
public:
    GraphCliqueAnalyzer() : nodeCount(0), largestCliqueSize(0), totalCliques(0) {}
   
    void buildGraphFromFile(const string& dataFile) {
        ifstream inputStream(dataFile);
        if (!inputStream) {
            cerr << "Error opening file: " << dataFile << endl;
            return;
        }

        string textLine;
        int linkCount = 0;
        bool headerDetected = false;
        idMapper.clear();

        // Look for header with node and edge count
        while (getline(inputStream, textLine)) {
    // Skip empty lines
    if (textLine.empty()) {
        continue;
    }
   
    // Check if line is a comment
    if (textLine[0] == '#') {
        // Look for the specific header format in the comment
        size_t nodesPos = textLine.find("Nodes:");
        size_t edgesPos = textLine.find("Edges:");
       
        if (nodesPos != string::npos && edgesPos != string::npos) {
            // Extract the numbers using substring operations instead of stringstream
            string nodeStr = textLine.substr(nodesPos + 6);  // "Nodes:" is 6 characters
            string edgeStr = textLine.substr(edgesPos + 6);  // "Edges:" is 6 characters
           
            // Convert strings to integers
            nodeCount = stoi(nodeStr);
            linkCount = stoi(edgeStr);
            headerDetected = true;
        }
        continue;
    }
   
    // If we reach here, we've found a non-comment, non-empty line
    // Exit the loop as per original code
    break;
}
// Attempt to read header using primary format
if (!headerDetected) {
    // Reset stream position to beginning
    inputStream.clear();
    inputStream.seekg(0, std::ios::beg);
   
    // Try to extract node count and link count directly
    std::string line;
    if (std::getline(inputStream, line)) {
        std::istringstream lineStream(line);
        if (lineStream >> nodeCount >> linkCount) {
            headerDetected = true;
        }
    }
}


// Validate node count
if (nodeCount <= 0) {
    std::cerr << "Error: Node count must be positive, found: " << nodeCount << std::endl;
    return;
}

// Check if header detection was successful
if (!headerDetected) {
    std::cerr << "Error: Failed to detect header containing node and edge counts" << std::endl;
    return;
}



        // Initialize graph
        connections.clear();
        connections.resize(nodeCount);

        // Read all connections
        inputStream.clear();
        inputStream.seekg(0, ios::beg);
       
        int actualLinkCount = 0;
        bool skipFirstLine = true;
       
   while (std::getline(inputStream, textLine)) {
    // Ignore empty lines, comments, and metadata lines
    if (textLine.empty() || textLine[0] == '#') {
        continue;
    }
   
    // Skip lines containing header information
    if (textLine.find("Nodes:") != std::string::npos ||
        textLine.find("Edges:") != std::string::npos ||
        textLine.find("FromNodeId") != std::string::npos) {
        continue;
    }
   
    // Check if this is potentially a header line with node/edge counts
    if (skipFirstLine) {
        std::istringstream headerCheck(textLine);
        int nodeCountCheck, edgeCountCheck;
       
        // If line contains node count that matches expected, skip it
        if (headerCheck >> nodeCountCheck >> edgeCountCheck &&
            nodeCountCheck == nodeCount) {
            skipFirstLine = false;
            continue;
        }
       
        // Not a header line, process normally but don't check again
        skipFirstLine = false;
    }
   
    // Process edge data
    std::istringstream edgeParser(textLine);
    int sourceNode, targetNode;
   
    if (edgeParser >> sourceNode >> targetNode) {
        connectNodes(sourceNode, targetNode);
        actualLinkCount++;
    }
}

        inputStream.close();
       
        // Update node count based on actual data
        nodeCount = connections.size();
        cout << "Loaded graph with " << nodeCount << " nodes and " << actualLinkCount << " edges." << endl;
    }
   
    void analyzeGraph() {
        auto startMoment = high_resolution_clock::now();
       
        // Get optimal ordering
        vector<int> nodeOrder = calculateOptimalOrder();
       
        // Create lookup for quick position finding
        vector<int> nodePositions(nodeCount);
       
        int idx = 0;
        while (idx < nodeCount) {
            nodePositions[nodeOrder[idx]] = idx;
            idx++;
        }
       
        // Working vectors
        vector<int> cliqueMembers;
        vector<int> candidateNodes;
        vector<int> excludedNodes;
        vector<int> tempList;
       
        // Process each node in order
        idx = 0;
        while (idx < nodeCount) {
            int currentNode = nodeOrder[idx];
           
            // Find later neighbors
            candidateNodes.clear();
            auto neighborIter = connections[currentNode].begin();
            while (neighborIter != connections[currentNode].end()) {
                int adjNode = *neighborIter;
                if (nodePositions[adjNode] > idx) {
                    candidateNodes.push_back(adjNode);
                }
                ++neighborIter;
            }
           
            // Find earlier neighbors
            excludedNodes.clear();
            neighborIter = connections[currentNode].begin();
            while (neighborIter != connections[currentNode].end()) {
                int adjNode = *neighborIter;
                if (nodePositions[adjNode] < idx) {
                    excludedNodes.push_back(adjNode);
                }
                ++neighborIter;
            }
           
            // Start recursive search
            cliqueMembers.clear();
            cliqueMembers.push_back(currentNode);
            findCliquesRecursive(cliqueMembers, candidateNodes, excludedNodes);
            idx++;
        }
       
        auto endMoment = high_resolution_clock::now();
        auto timeElapsed = duration_cast<milliseconds>(endMoment - startMoment);
       
        // Print results
        cout << "1. Largest Clique Size: " << largestCliqueSize << endl;
        cout << "2. Total Number of Maximal Cliques: " << totalCliques << endl;
        cout << "3. Execution Time: " << timeElapsed.count() << " ms" << endl;
        cout << "4. Distribution of Clique Sizes:" << endl;
       
        int size = 1;
        while (size <= largestCliqueSize) {
            cout << "   - Cliques of size " << size << ": " << (sizeStats.count(size) ? sizeStats[size] : 0) << endl;
            size++;
        }
    }

private:
    // Add a connection to the graph
    void connectNodes(int origFirst, int origSecond) {
        // Map to internal IDs
        int first = getInternalId(origFirst);
        int second = getInternalId(origSecond);
       
        if (first < 0 || second < 0 || first >= nodeCount || second >= nodeCount) return;
       
        // Avoid duplicates
        if (find(connections[first].begin(), connections[first].end(), second) == connections[first].end()) {
            connections[first].push_back(second);
            connections[second].push_back(first); // Undirected graph
        }
    }
   
    // Get or create internal ID
    int getInternalId(int externalId) {
        if (idMapper.find(externalId) == idMapper.end()) {
            int newId = idMapper.size();
            if (newId >= nodeCount) {
                // Resize if needed
                nodeCount = newId + 1;
                connections.resize(nodeCount);
            }
            idMapper[externalId] = newId;
            return newId;
        }
        return idMapper[externalId];
    }

    // Recursive clique finding algorithm
    void findCliquesRecursive(vector<int>& currentClique, vector<int>& candidateNodes, vector<int>& excludedNodes) {
        // Base case: found a maximal clique
        if (candidateNodes.empty() && excludedNodes.empty()) {
            int cliqueSize = currentClique.size();
            totalCliques++;
            sizeStats[cliqueSize]++;
            largestCliqueSize = max(largestCliqueSize, cliqueSize);
            return;
        }
       
        // Select pivot for optimization
        vector<int> combinedSet;
        combinedSet.insert(combinedSet.end(), candidateNodes.begin(), candidateNodes.end());
        combinedSet.insert(combinedSet.end(), excludedNodes.begin(), excludedNodes.end());
       
        int pivotNode = -1;
        int bestCoverage = -1;
        vector<int> tempNodes; // Reusable storage
       
        auto nodeIter = combinedSet.begin();
        while (nodeIter != combinedSet.end()) {
            int testNode = *nodeIter;
            findCommonNodes(testNode, candidateNodes, tempNodes);
            if (static_cast<int>(tempNodes.size()) > bestCoverage) {
                bestCoverage = tempNodes.size();
                pivotNode = testNode;
            }
            ++nodeIter;
        }
       
        // Find candidates not adjacent to pivot
        vector<int> pivotAdjacent;
        findCommonNodes(pivotNode, candidateNodes, pivotAdjacent);
       
        // Make a copy for iteration
        vector<int> candidatesCopy = candidateNodes;
       
        // Work vectors for recursive calls
        vector<int> expandedClique, filteredCandidates, filteredExcluded;
       
        // Process each relevant vertex
        auto vertexIter = candidatesCopy.begin();
        while (vertexIter != candidatesCopy.end()) {
            int vertex = *vertexIter;
            if (find(pivotAdjacent.begin(), pivotAdjacent.end(), vertex) == pivotAdjacent.end()) {
                // Prepare next recursive call parameters
                expandedClique = currentClique;
                expandedClique.push_back(vertex);
               
                // Filter candidates by adjacency
                findCommonNodes(vertex, candidateNodes, filteredCandidates);
               
                // Filter excluded by adjacency
                findCommonNodes(vertex, excludedNodes, filteredExcluded);
               
                findCliquesRecursive(expandedClique, filteredCandidates, filteredExcluded);
               
                // Move processed vertex to excluded
                candidateNodes.erase(remove(candidateNodes.begin(), candidateNodes.end(), vertex), candidateNodes.end());
                excludedNodes.push_back(vertex);
            }
            ++vertexIter;
        }
    }

    // Find adjacent nodes that are in a given set
    void findCommonNodes(int vertex, const vector<int>& nodeSet, vector<int>& resultNodes) {
        resultNodes.clear();
       
        // Choose approach based on set size
        if (nodeSet.size() > 10) { // Threshold for optimization
            unordered_set<int> fastLookup(nodeSet.begin(), nodeSet.end());
           
            auto neighborIter = connections[vertex].begin();
            while (neighborIter != connections[vertex].end()) {
                int adjNode = *neighborIter;
                if (fastLookup.find(adjNode) != fastLookup.end()) {
                    resultNodes.push_back(adjNode);
                }
                ++neighborIter;
            }
        } else {
            // For smaller sets, simple search
            auto neighborIter = connections[vertex].begin();
            while (neighborIter != connections[vertex].end()) {
                int adjNode = *neighborIter;
                if (find(nodeSet.begin(), nodeSet.end(), adjNode) != nodeSet.end()) {
                    resultNodes.push_back(adjNode);
                }
                ++neighborIter;
            }
        }
    }

    // Compute the optimal vertex ordering for efficiency
    vector<int> calculateOptimalOrder() {
        vector<int> result;
        vector<int> nodeDegrees(nodeCount);
        vector<bool> alreadyProcessed(nodeCount, false);
       
        // Compute initial node degrees
        int currentNode = 0;
        while (currentNode < nodeCount) {
            nodeDegrees[currentNode] = connections[currentNode].size();
            currentNode++;
        }
       
        // Use counting sort approach for better performance
        int maxDegreeFound = 0;
        vector<int>::iterator degreeIter = nodeDegrees.begin();
        while (degreeIter != nodeDegrees.end()) {
            maxDegreeFound = max(maxDegreeFound, *degreeIter);
            ++degreeIter;
        }
       
        vector<vector<int>> degreeGroups(maxDegreeFound + 1);
        currentNode = 0;
        while (currentNode < nodeCount) {
            degreeGroups[nodeDegrees[currentNode]].push_back(currentNode);
            currentNode++;
        }
       
        // Process nodes by ascending degree
        int currentDegree = 0;
        while (currentDegree <= maxDegreeFound) {
            while (!degreeGroups[currentDegree].empty()) {
                int vertex = degreeGroups[currentDegree].back();
                degreeGroups[currentDegree].pop_back();
               
                if (alreadyProcessed[vertex]) continue;
               
                // Add to result and mark
                result.push_back(vertex);
                alreadyProcessed[vertex] = true;
               
                // Update degrees of remaining neighbors
                auto neighborIter = connections[vertex].begin();
                while (neighborIter != connections[vertex].end()) {
                    int adjacentNode = *neighborIter;
                    if (!alreadyProcessed[adjacentNode]) {
                        int prevDegree = nodeDegrees[adjacentNode];
                        nodeDegrees[adjacentNode]--;
                       
                        if (prevDegree > nodeDegrees[adjacentNode]) {
                            // Move to appropriate degree group
                            degreeGroups[nodeDegrees[adjacentNode]].push_back(adjacentNode);
                        }
                    }
                    ++neighborIter;
                }
            }
            currentDegree++;
        }
       
        return result;
    }

    // Data members
    vector<vector<int>> connections;
    int nodeCount;
    int largestCliqueSize;
    int totalCliques;
    map<int, int> sizeStats;
    unordered_map<int, int> idMapper; // For mapping original IDs to zero-based indices
};

int main(int argc, char* argv[]) {
    GraphCliqueAnalyzer analyzer;
   
    string inputFile = "Email-Enron.txt";  // Default filename
    if (argc > 1) {
        inputFile = argv[1];
    }
   
    analyzer.buildGraphFromFile(inputFile);
    analyzer.analyzeGraph();
   
    return 0;
}