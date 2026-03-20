
/*
g++ -O2 -std=c++11 -fsanitize=address tarjan.cpp && ./a.out
*/

#include <iostream>
#include <cassert>
#include <cstdlib>
#include <ctime>
#include <vector>
#include <deque>
#include <stack>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <random>
#include <cmath>
#include <optional>
#include <utility>
#include <fstream>
#include <thread>
#include <chrono>
using namespace std;

struct Edge{
    int x, y;

    Edge(int x_, int y_){
        if(x_ < y_){
            x = x_; y = y_;
        }
        else{
            x = y_; y = x_;
        }
    }

    bool operator == (const Edge& other) const{
        return x == other.x && y == other.y;
    }
};

struct EdgeHash {
    size_t operator()(const Edge& p) const {
        size_t h1 = std::hash<int>{}(p.x);
        size_t h2 = std::hash<int>{}(p.y);
        return h1 ^ (h2 << 1); // combine hashes
    }
};


class Graph{
public:
    vector<vector<int>> adjList;
    int size;

    Graph(int size_, bool randomize=true){
        size = size_;
        adjList = vector<vector<int>>(size, vector<int>());
        if(!randomize) return;
        while(numComponents() > 1){
            int x = rand() % size;
            int y = rand() % size;
            if(x == y) continue;
            if(find(adjList[x].begin(), adjList[x].end(), y) != adjList[x].end()) continue;
            adjList[x].push_back(y);
            adjList[y].push_back(x);
        }
    }

    int numComponents(){
        vector<bool> visited(size, false);
        int ans = 0;
        for(int i=0; i<size; i++){
            if(visited[i]) continue;
            queue<int> q;
            q.push(i);
            visited[i] = true;
            ans ++;

            while(q.size() > 0){
                int top = q.front();
                q.pop();
                for(int& neigh : adjList[top]){
                    if(!visited[neigh]){
                        q.push(neigh);
                        visited[neigh] = true;
                    }
                }
            }
        }
        return ans;
    }

    void print(){
        for(int i=0; i<size; i++){
            cout << i << ": ";
            for(int& neigh : adjList[i]){
                cout << neigh << ' ';
            }
            cout << '\n';
        }
    }

    vector<int> APbruteForce(){
        vector<int> ans;
        for(int i=0; i<size; i++){
            vector<int> neighs = adjList[i];
            adjList[i] = vector<int>();
            for(int& neigh : neighs){
                auto it = find(adjList[neigh].begin(), adjList[neigh].end(), i);
                adjList[neigh].erase(it);
            }
            if(numComponents() > 2){
                ans.push_back(i);
            }
            adjList[i] = neighs;
            for(int& neigh : neighs){
                adjList[neigh].push_back(i);
            }
        }
        return ans;
    }

    vector<int> isAP;
    vector<int> timeIn;
    vector<int> minConnTime;
    vector<Edge> edgeQueue;
    int tarjan_timer;

    // unordered_set<Edge, EdgeHash> usedEdges;

    vector<unordered_set<int>> BCCs;

    void flushQueue(int queueIndex){
        unordered_set<int> bcc;
        while(edgeQueue.size() > queueIndex){
            bcc.insert(edgeQueue[edgeQueue.size()-1].x);
            bcc.insert(edgeQueue[edgeQueue.size()-1].y);
            edgeQueue.pop_back();
        }
        BCCs.push_back(bcc);
    }

    // Claude code:

    void dfs(int node, int p=-1){
        minConnTime[node] = timeIn[node] = tarjan_timer++;
        int n_children = 0;
        for(int& neigh : adjList[node]){
            if(neigh == p) continue;

            int queueIndex = edgeQueue.size();
            edgeQueue.push_back(Edge{node, neigh});

            if(timeIn[neigh] == -1){
                n_children++;
                dfs(neigh, node);
                minConnTime[node] = min(minConnTime[node], minConnTime[neigh]);

                if(p != -1 && minConnTime[neigh] >= timeIn[node]){
                    isAP[node] = true;
                    flushQueue(queueIndex);
                }
                if(p == -1){
                    flushQueue(queueIndex);  // was incorrectly 0
                }
            }
            else if(timeIn[neigh] < timeIn[node]){ // only process back-edges, not forward
                minConnTime[node] = min(minConnTime[node], timeIn[neigh]);
            }
            else{
                edgeQueue.pop_back(); // forward cross-edge, don't keep it
            }
        }
        if(p == -1 && n_children > 1){
            isAP[node] = true;
        }
    }

    // void dfs(int node, int p=-1){
    //     minConnTime[node] = timeIn[node] = tarjan_timer;
    //     tarjan_timer ++;
    //     int n_children = 0;
    //     for(int& neigh : adjList[node]){
    //         if(neigh == p) continue;

    //         int queueIndex = edgeQueue.size();
    //         Edge e{node, neigh};
    //         if(usedEdges.find(e) == usedEdges.end()){
    //             edgeQueue.push_back(e);
    //             usedEdges.insert(e);
    //         }

    //         if(timeIn[neigh] == -1){
    //             n_children += 1;
    //             dfs(neigh, node);
    //             if(p != -1 && minConnTime[neigh] >= timeIn[node]){
    //                 isAP[node] = true;
    //                 flushQueue(queueIndex);
    //             }
    //             if(p == -1){
    //                 flushQueue(0);
    //             }
    //             minConnTime[node] = min(minConnTime[node], minConnTime[neigh]);
    //         }
    //         else{
    //             minConnTime[node] = min(minConnTime[node], timeIn[neigh]);
    //         }
    //     }
    //     if(p == -1 && n_children > 1){
    //         isAP[node] = true;
    //     }
    // }

    vector<int> APtarjan(){
        isAP = vector<int>(size, false);
        timeIn = vector<int>(size, -1);
        minConnTime = vector<int>(size, -1);
        edgeQueue = vector<Edge>();
        BCCs = vector<unordered_set<int>>();
        tarjan_timer = 0;
        dfs(0);
        vector<int> ans;
        for(int i=0; i<size; i++){
            if(isAP[i]){
                ans.push_back(i);
            }
        }
        return ans;
    }

    Graph generateSubgraph(vector<int> nodes){
        Graph g(nodes.size(), false);
        vector<int> indices(size, -1);
        for(int i=0; i<nodes.size(); i++){
            indices[nodes[i]] = i;
        }
        for(int i=0; i<nodes.size(); i++){
            for(int j=0; j<adjList[nodes[i]].size(); j++){
                int index = indices[adjList[nodes[i]][j]];
                if(index != -1) g.adjList[i].push_back(index);
            }
        }
        return g;
    }

    bool isConsistent(bool printMode=false){
        vector<int> ans1 = APbruteForce();
        vector<int> ans2 = APtarjan();

        if(printMode){
            print();
            cout << "Brute Force APs: ";
            for(int& x : ans1) cout << x << ' ';
            cout << '\n';
            cout << "Tarjan APs:      ";
            for(int& x : ans2) cout << x << ' ';
            cout << '\n';
        }

        if(ans1.size() != ans2.size()) return false;
        for(int i=0; i<(int)ans1.size(); i++){
            if(ans1[i] != ans2[i]) return false;
        }

        // Build the set of all edges in the graph
        unordered_set<Edge, EdgeHash> allEdges;
        for(int u=0; u<size; u++){
            for(int v : adjList[u]){
                allEdges.insert(Edge{u, v});
            }
        }

        // Check validity of BCC decomposition
        unordered_set<Edge, EdgeHash> coveredEdges;
        vector<bool> coveredNodes(size, false);

        for(const auto& bcc : BCCs){
            // Each BCC must have no APs internally
            vector<int> bcc_nodes(bcc.begin(), bcc.end());
            Graph subgraph = generateSubgraph(bcc_nodes);
            if(subgraph.APtarjan().size() > 0) return false;

            // Adding any external neighbor must create an AP (maximality check)
            for(const int& x : bcc){
                for(const int& y : adjList[x]){
                    if(bcc.find(y) != bcc.end()) continue;
                    vector<int> cpy = bcc_nodes;
                    cpy.push_back(y);
                    if(generateSubgraph(cpy).APtarjan().size() == 0) return false;
                }
            }

            // Accumulate covered nodes and edges
            for(const int& x : bcc){
                coveredNodes[x] = true;
                for(const int& y : adjList[x]){
                    if(bcc.find(y) != bcc.end()){
                        coveredEdges.insert(Edge{x, y});
                    }
                }
            }
        }

        // Every node must be in at least one BCC
        for(int i=0; i<size; i++){
            if(!coveredNodes[i]) return false;
        }

        // Every edge must be in at least one BCC
        if(coveredEdges != allEdges) return false;

        return true;
    }

    // bool isConsistent(bool printMode=false){
    //     vector<int> ans1 = APbruteForce();
    //     vector<int> ans2 = APtarjan();
    //     if(printMode){
    //         print();
    //         cout << "Brute Force: ";
    //         for(int& x : ans1){
    //             cout << x << ' ';
    //         }
    //         cout << '\n';
    //         cout << "Tarjan: ";
    //         for(int& x : ans2){
    //             cout << x << ' ';
    //         }
    //         cout << '\n';
    //     }
    //     if(ans1.size() != ans2.size()) return false;
    //     for(int i=0; i<ans1.size(); i++){
    //         if(ans1[i] != ans2[i]) return false;
    //     }

    //     // Check validity of BCC decomposition

    //     vector<bool> covered(size, false);
    //     for(const auto& bcc : BCCs){
    //         // cout << "Lmao ";
    //         for(const int& x : bcc){
    //             // cout << x << ' ';
    //             covered[x] = true;
    //         }
    //         // cout << "\n";
    //         vector<int> bcc_nodes(bcc.begin(), bcc.end());
    //         Graph subgraph = generateSubgraph(bcc_nodes);
    //         if(subgraph.APtarjan().size() > 0){
    //             // cout << "Yo\n";
    //             return false;
    //         } 
    //         for(const int& x : bcc){
    //             for(const int& y : adjList[x]){
    //                 if(bcc.find(y) != bcc.end()) continue;
    //                 // cout << "Cheking " << y << '\n';
    //                 vector<int> cpy = bcc_nodes;
    //                 cpy.push_back(y);
    //                 if(generateSubgraph(cpy).APtarjan().size() == 0){
    //                     // cout << "Bruy\n";
    //                     return false;
    //                 } 
    //             }
    //         }
    //     }
    //     for(int i=0; i<size; i++){
    //         if(!covered[i]) return false;
    //     }
    //     return true;
    // }
};

int main(){
    srand(1456);

    // Graph g(10);

    // cout << g.isConsistent(true) << '\n';

    // cout << "BCCs\n";

    // for(const auto& bcc : g.BCCs){
    //     for(const int& x : bcc){
    //         cout << x << ' ';
    //     }
    //     cout << '\n';
    // }

    for(int i=0; i<1000; i++){
        Graph g(50);
        // g.print();
        assert(g.isConsistent());
    }
}