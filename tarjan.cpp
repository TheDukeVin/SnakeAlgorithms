
/*
g++ -O2 -std=c++11 -fsanitize=address tarjan.cpp && ./a.out
*/

#include <iostream>
#include <vector>
#include <ctime>
#include <stdlib.h>
#include <queue>
#include <cassert>
using namespace std;

class Graph{
public:
    vector<vector<int>> adjList;
    int size;

    Graph(int size_){
        size = size_;
        adjList = vector<vector<int>>(size, vector<int>());
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
    int tarjan_timer;

    void dfs(int node, int p=-1){
        minConnTime[node] = timeIn[node] = tarjan_timer;
        tarjan_timer ++;
        int n_children = 0;
        for(int& neigh : adjList[node]){
            if(neigh == p) continue;
            if(timeIn[neigh] == -1){
                n_children += 1;
                dfs(neigh, node);
                if(p != -1 && minConnTime[neigh] >= timeIn[node]){
                    isAP[node] = true;
                }
                minConnTime[node] = min(minConnTime[node], minConnTime[neigh]);
            }
            else{
                minConnTime[node] = min(minConnTime[node], timeIn[neigh]);
            }
        }
        if(p == -1 && n_children > 1){
            isAP[node] = true;
        }
    }

    vector<int> APtarjan(){
        isAP = vector<int>(size, false);
        timeIn = vector<int>(size, -1);
        minConnTime = vector<int>(size, -1);
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

    bool isConsistent(bool printMode=false){
        vector<int> ans1 = APbruteForce();
        vector<int> ans2 = APtarjan();
        if(printMode){
            print();
            cout << "Brute Force: ";
            for(int& x : ans1){
                cout << x << ' ';
            }
            cout << '\n';
            cout << "Tarjan: ";
            for(int& x : ans2){
                cout << x << ' ';
            }
            cout << '\n';
        }
        if(ans1.size() != ans2.size()) return false;
        for(int i=0; i<ans1.size(); i++){
            if(ans1[i] != ans2[i]) return false;
        }
        return true;
    }
};

int main(){
    srand(1456);
    for(int i=0; i<1000; i++){
        Graph g(50);
        assert(g.isConsistent());
    }
}