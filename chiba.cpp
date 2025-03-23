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

using namespace std;
using namespace std::chrono;

class MaximalCliquesFinder {
private:
    vector<vector<int>> adjacencyList;
    int numVertices;
    int largestCliqueSize;
    int totalCliques;
    map<int, int> cliqueSizeDistribution;

    vector<int> computeDegeneracyOrdering() {
        vector<int> ordering;
        vector<int> degrees(numVertices);
        vector<bool> processed(numVertices, false);
       
        for (int v = 0; v < numVertices; v++) {
            degrees[v] = adjacencyList[v].size();
        }
       
        int maxDegree = 0;
        for (int d : degrees) {
            maxDegree = max(maxDegree, d);
        }
       
        vector<vector<int>> buckets(maxDegree + 1);
        for (int v = 0; v < numVertices; v++) {
            buckets[degrees[v]].push_back(v);
        }
       
        for (int d = 0; d <= maxDegree; d++) {
            while (!buckets[d].empty()) {
                int v = buckets[d].back();
                buckets[d].pop_back();
               
                if (processed[v]) continue;
               
                ordering.push_back(v);
                processed[v] = true;
               
                for (int neighbor : adjacencyList[v]) {
                    if (!processed[neighbor]) {
                        int oldDegree = degrees[neighbor];
                        degrees[neighbor]--;
                       
                        if (oldDegree > degrees[neighbor]) {
                            buckets[degrees[neighbor]].push_back(neighbor);
                        }
                    }
                }
            }
        }
       
        return ordering;
    }

    vector<int> getNeighborsInSet(int v, const vector<int>& s) {
        vector<int> result;
       
        if (s.size() <= 10) {
            for (int neighbor : adjacencyList[v]) {
                if (find(s.begin(), s.end(), neighbor) != s.end()) {
                    result.push_back(neighbor);
                }
            }
        } else {
            unordered_set<int> sSet(s.begin(), s.end());
            for (int neighbor : adjacencyList[v]) {
                if (sSet.find(neighbor) != sSet.end()) {
                    result.push_back(neighbor);
                }
            }
        }
       
        return result;
    }

    void code(vector<int> R, vector<int> P, vector<int> X) {
        if (P.empty() && X.empty()) {
            int cliqueSize = R.size();
            totalCliques++;
            cliqueSizeDistribution[cliqueSize]++;
            largestCliqueSize = max(largestCliqueSize, cliqueSize);
            return;
        }
       
        vector<int> PUnionX;
        PUnionX.insert(PUnionX.end(), P.begin(), P.end());
        PUnionX.insert(PUnionX.end(), X.begin(), X.end());
       
        int pivotVertex = -1;
        int maxIntersectionSize = -1;
       
        for (int u : PUnionX) {
            vector<int> neighbors = getNeighborsInSet(u, P);
            if (static_cast<int>(neighbors.size()) > maxIntersectionSize) {
                maxIntersectionSize = neighbors.size();
                pivotVertex = u;
            }
        }
       
        vector<int> pivotNeighbors = getNeighborsInSet(pivotVertex, P);
       
        vector<int> PCopy = P;
       
        for (int v : PCopy) {
            if (find(pivotNeighbors.begin(), pivotNeighbors.end(), v) == pivotNeighbors.end()) {
                vector<int> Rprime = R;
                Rprime.push_back(v);
               
                vector<int> Pprime = getNeighborsInSet(v, P);
                vector<int> Xprime = getNeighborsInSet(v, X);
               
                code(Rprime, Pprime, Xprime);
               
                P.erase(remove(P.begin(), P.end(), v), P.end());
                X.push_back(v);
            }
        }
    }

    void findMaximalCliquesKose() {
        vector<int> vertexOrder(numVertices);
        vector<int> S(numVertices, 0);
        vector<int> T(numVertices, 0);
        vector<int> C;
       
        vector<pair<int, int>> degreeVertexPairs;
        for (int i = 0; i < numVertices; i++) {
            degreeVertexPairs.push_back({adjacencyList[i].size(), i});
        }
        sort(degreeVertexPairs.begin(), degreeVertexPairs.end());
       
        for (int i = 0; i < numVertices; i++) {
            vertexOrder[i] = degreeVertexPairs[i].second;
        }
       
        updateKose(2, C, vertexOrder, S, T);
    }
   
    void updateKose(int i, vector<int>& C, const vector<int>& vertexOrder, vector<int>& S, vector<int>& T) {
        if (i == numVertices + 1) {
            int cliqueSize = C.size();
            totalCliques++;
            cliqueSizeDistribution[cliqueSize]++;
            largestCliqueSize = max(largestCliqueSize, cliqueSize);
            return;
        }
       
        int vertex = vertexOrder[i-1];
       
        vector<int> intersection = getNeighborsInClique(vertex, C);
       
        if (!intersection.empty()) {
            updateKose(i + 1, C, vertexOrder, S, T);
        }
       
        computeT(vertex, C, T);
       
        computeS(vertex, C, S);
       
        bool FLAG = true;
       
        FLAG = checkMaximality(vertex, C, T, vertexOrder);
       
        if (FLAG) {
            FLAG = checkLexicographicalOrder(vertex, C, S, T, vertexOrder);
        }
       
        resetST(vertex, C, S, T);
       
        if (FLAG) {
            vector<int> SAVE = getDifference(C, getNeighborsInSet(vertex, C));
            C = getUnion(intersection, {vertex});
            updateKose(i + 1, C, vertexOrder, S, T);
            C = getUnion(getDifference(C, {vertex}), SAVE);
        }
    }
   
    vector<int> getNeighborsInClique(int v, const vector<int>& clique) {
        vector<int> result;
        for (int u : clique) {
            if (find(adjacencyList[v].begin(), adjacencyList[v].end(), u) != adjacencyList[v].end()) {
                result.push_back(u);
            }
        }
        return result;
    }
   
    void computeT(int vertex, const vector<int>& C, vector<int>& T) {
        vector<int> intersection = getNeighborsInClique(vertex, C);
        for (int x : intersection) {
            for (int y : adjacencyList[x]) {
                if (find(C.begin(), C.end(), y) == C.end() && y != vertex) {
                    T[y]++;
                }
            }
        }
    }
   
    void computeS(int vertex, const vector<int>& C, vector<int>& S) {
        vector<int> difference;
        for (int x : C) {
            if (find(adjacencyList[vertex].begin(), adjacencyList[vertex].end(), x) == adjacencyList[vertex].end()) {
                difference.push_back(x);
            }
        }
       
        for (int x : difference) {
            for (int y : adjacencyList[x]) {
                if (find(C.begin(), C.end(), y) == C.end()) {
                    S[y]++;
                }
            }
        }
    }
   
    bool checkMaximality(int vertex, const vector<int>& C, const vector<int>& T, const vector<int>& vertexOrder) {
        int intersectionSize = getNeighborsInClique(vertex, C).size();
       
        for (int y : adjacencyList[vertex]) {
            if (find(C.begin(), C.end(), y) == C.end() &&
                getVertexIndex(y, vertexOrder) < getVertexIndex(vertex, vertexOrder) &&
                T[y] == intersectionSize) {
                return false;
            }
        }
       
        return true;
    }
   
    bool checkLexicographicalOrder(int vertex, const vector<int>& C, const vector<int>& S, const vector<int>& T, const vector<int>& vertexOrder) {
        return true;
    }
   
    void resetST(int vertex, const vector<int>& C, vector<int>& S, vector<int>& T) {
        vector<int> intersection = getNeighborsInClique(vertex, C);
        for (int x : intersection) {
            for (int y : adjacencyList[x]) {
                if (find(C.begin(), C.end(), y) == C.end() && y != vertex) {
                    T[y] = 0;
                }
            }
        }
       
        vector<int> difference;
        for (int x : C) {
            if (find(adjacencyList[vertex].begin(), adjacencyList[vertex].end(), x) == adjacencyList[vertex].end()) {
                difference.push_back(x);
            }
        }
       
        for (int x : difference) {
            for (int y : adjacencyList[x]) {
                if (find(C.begin(), C.end(), y) == C.end()) {
                    S[y] = 0;
                }
            }
        }
    }
   
    int getVertexIndex(int vertex, const vector<int>& vertexOrder) {
        for (int i = 0; i < numVertices; i++) {
            if (vertexOrder[i] == vertex) {
                return i;
            }
        }
        return -1;
    }
   
    vector<int> getDifference(const vector<int>& A, const vector<int>& B) {
        vector<int> result;
        for (int a : A) {
            if (find(B.begin(), B.end(), a) == B.end()) {
                result.push_back(a);
            }
        }
        return result;
    }
   
    vector<int> getUnion(const vector<int>& A, const vector<int>& B) {
        vector<int> result = A;
        for (int b : B) {
            if (find(result.begin(), result.end(), b) == result.end()) {
                result.push_back(b);
            }
        }
        return result;
    }

    void addEdge(int u, int v) {
        if (u < 0 || v < 0 || u >= numVertices || v >= numVertices) return;
        if (find(adjacencyList[u].begin(), adjacencyList[u].end(), v) == adjacencyList[u].end()) {
            adjacencyList[u].push_back(v);
            adjacencyList[v].push_back(u);
        }
    }

public:
    MaximalCliquesFinder() : numVertices(0), largestCliqueSize(0), totalCliques(0) {}
   
    void loadGraphFromFile(const string& filename) {
        ifstream infile(filename);
        if (!infile) {
            cerr << "Error opening file: " << filename << endl;
            return;
        }

        string line;
        int edges = 0;
        bool header_found = false;

        while (getline(infile, line)) {
            if (line.empty() || line[0] == '#') {
                if (line.find("Nodes:") != string::npos && line.find("Edges:") != string::npos) {
                    stringstream ss(line);
                    string temp;
                    ss >> temp >> temp >> numVertices >> temp >> edges;
                    header_found = true;
                }
                continue;
            }
            break;
        }

        if (!header_found) {
            infile.clear();
            infile.seekg(0, ios::beg);
            if (getline(infile, line)) {
                stringstream ss(line);
                if (ss >> numVertices >> edges) {
                    header_found = true;
                }
            }
        }

        if (!header_found) {
            cerr << "Could not find header with node and edge count!" << endl;
            return;
        }

        if (numVertices <= 0) {
            cerr << "Invalid number of nodes: " << numVertices << endl;
            return;
        }

        adjacencyList.clear();
        adjacencyList.resize(numVertices);

        infile.clear();
        infile.seekg(0, ios::beg);
       
        int edgeCount = 0;
        bool skipHeader = true;
       
        while (getline(infile, line)) {
            if (line.empty() || line[0] == '#') continue;
            if (line.find("Nodes:") != string::npos && line.find("Edges:") != string::npos) continue;
            if (line.find("FromNodeId") != string::npos) continue;
           
            if (skipHeader) {
                stringstream ss(line);
                int n, e;
                if (ss >> n >> e && n == numVertices) {
                    skipHeader = false;
                    continue;
                }
                skipHeader = false;
            }
           
            stringstream ss(line);
            int u, v;
            if (ss >> u >> v) {
                addEdge(u, v);
                edgeCount++;
            }
        }

        infile.close();
        cout << "Loaded graph with " << numVertices << " nodes and " << edgeCount << " edges." << endl;
    }
   
    void findMaximalCliques() {
        largestCliqueSize = 0;
        totalCliques = 0;
        cliqueSizeDistribution.clear();
       
        auto startTime = high_resolution_clock::now();
       
        vector<int> ordering = computeDegeneracyOrdering();
       
        vector<int> orderingIndex(numVertices);
        for (int i = 0; i < numVertices; i++) {
            orderingIndex[ordering[i]] = i;
        }
       
        for (int i = 0; i < numVertices; i++) {
            int v = ordering[i];
           
            vector<int> P;
            for (int neighbor : adjacencyList[v]) {
                if (orderingIndex[neighbor] > i) {
                    P.push_back(neighbor);
                }
            }
           
            vector<int> X;
            for (int neighbor : adjacencyList[v]) {
                if (orderingIndex[neighbor] < i) {
                    X.push_back(neighbor);
                }
            }
           
            vector<int> R = {v};
            code(R, P, X);
        }
       
        auto endTime = high_resolution_clock::now();
        auto duration = duration_cast<milliseconds>(endTime - startTime);
       
        cout << "1. Largest Clique Size: " << largestCliqueSize << endl;
        cout << "2. Total Number of Maximal Cliques: " << totalCliques << endl;
        cout << "3. Execution Time: " << duration.count() << " ms" << endl;
        cout << "4. Distribution of Clique Sizes:" << endl;
       
        for (int i = 1; i <= largestCliqueSize; i++) {
            cout << "   - Cliques of size " << i << ": " << (cliqueSizeDistribution.count(i) ? cliqueSizeDistribution[i] : 0) << endl;
        }
    }
};

int main(int argc, char* argv[]) {
    MaximalCliquesFinder finder;
   
    string filename = "Email-Enron.txt";
    if (argc > 1) {
        filename = argv[1];
    }
   
    finder.loadGraphFromFile(filename);
    finder.findMaximalCliques();
   
    return 0;
}