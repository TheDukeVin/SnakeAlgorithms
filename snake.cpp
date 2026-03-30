
/*
g++ -O2 -std=c++20 snake.cpp && ./a.out

-fsanitize=address -fsanitize=undefined -fno-sanitize-recover=all -fsanitize=float-divide-by-zero -fsanitize=float-cast-overflow -fno-sanitize=null -fno-sanitize=alignment
*/

#include <iostream>
#include <fstream>
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

const int boardSize = 10;
const int area = boardSize * boardSize;

const int EMPTY = -1;
const int INF = 1 << 20;

struct Pos{
    int x, y;

    bool operator == (const Pos& other) const {
        return x == other.x && y == other.y;
    }

    bool operator != (const Pos& other) const{
        return !(*this == other);
    }
};

int ID(Pos p){
    return p.x*boardSize + p.y;
}
int ID(int x, int y){
    return x*boardSize + y;
}
Pos fromID(int id){
    return Pos{id / boardSize, id % boardSize};
}

int dir[4][2] = {{0, 1}, {1, 0}, {0, -1}, {-1, 0}};

bool isValid(Pos p){
    return 0 <= p.x && p.x < boardSize && 0 <= p.y && p.y < boardSize;
}

Pos shiftPos(const Pos& p, const int& d){
    return Pos{p.x + dir[d][0], p.y + dir[d][1]};
}

int manhattanDist(Pos p, Pos q){
    return abs(p.x - q.x) + abs(p.y - q.y);
}

vector<Pos> validNeighs(Pos p){
    vector<Pos> ans;
    for(int d=0; d<4; d++){
        Pos newPos = shiftPos(p, d);
        if(isValid(newPos)) ans.push_back(newPos);
    }
    return ans;
}

vector<string> split(string text, string token){
    vector<string> ans;
    int pos;
    while((pos=text.find(token)) != string::npos){
        ans.push_back(text.substr(0, pos));
        text.erase(0, pos + token.size());
    }
    ans.push_back(text);
    return ans;
}

class Snake{
public:
    vector<int> body;
    Pos head, tail, apple;
    int timer;

    Snake(){
        body = vector<int>(area, EMPTY);
        timer = 0;

        head = Pos{boardSize/2, 1};
        tail = Pos{boardSize/2, 0};
        body[ID(tail)] = 0;
        // body[ID(head)] = 4;
        randomizeApple();
    }

    Pos CodeToPos(string code) const{
        vector<string> parts = split(code, "x");
        return Pos{stoi(parts[0]), stoi(parts[1])};
    }

    Snake(string code){
        vector<string> sections = split(code, "_");
        head = CodeToPos(sections[0]);
        tail = CodeToPos(sections[1]);
        apple = CodeToPos(sections[2]);
        timer = stoi(sections[3]);
        for(int i=0; i<area/2; i++){
            int c = sections[4][i] - 97;
            body.push_back(c/5-1);
            body.push_back(c%5-1);
        }
        // body[ID(head)] = 4;
    }

    vector<Pos> emptyNeighs(Pos p) const{
        vector<Pos> ans;
        for(int d=0; d<4; d++){
            Pos newPos = shiftPos(p, d);
            if(isValid(newPos) && body[ID(newPos)] == -1) ans.push_back(newPos);
        }
        return ans;
    }

    vector<int> validMoves() const{
        vector<int> moves;
        for(int d=0; d<4; d++){
            Pos newHead = shiftPos(head, d);
            if(isValid(newHead) && body[ID(newHead)] == -1){
                moves.push_back(d);
            }
        }
        return moves;
    }

    bool move(int d){ // returns whether apple was achieved
        Pos newHead = shiftPos(head, d);
        assert(isValid(newHead) && body[ID(newHead)] == -1);

        body[ID(head)] = d;
        head = newHead;
        // body[ID(head)] = 4;

        bool appleAchieved = false;

        if(head == apple){
            appleAchieved = true;
        }
        else{
            int tailDir = body[ID(tail)];
            body[ID(tail)] = -1;
            tail = shiftPos(tail, tailDir);
        }
        timer += 1;
        return appleAchieved;
    }

    void randomizeApple(){
        vector<int> emptyCells;
        for(int i=0; i<area; i++){
            if(body[i] == EMPTY && i != ID(head)) emptyCells.push_back(i);
        }
        assert(emptyCells.size() > 0);
        apple = fromID(emptyCells[rand() % emptyCells.size()]);
    }

    string PosToCode(Pos p) const{
        return to_string(p.x) + 'x' + to_string(p.y);
    }

    string toCode() const{
        string s = PosToCode(head) + '_' + PosToCode(tail) + '_' + PosToCode(apple) + '_' + to_string(timer) + '_';
        // vector<int> body_ = body;
        // body_[ID(head)] = -1;
        for(int i=0; i<area/2; i++){
            s += (char) (97 + (body[2*i]+1)*5 + (body[2*i+1]+1));
        }
        return s;
    }

    int getSize() const{
        return count_if(body.begin(),body.end(),[](int v){return v!=-1;})+1;
    }

    void display(vector<int> path=vector<int>(), vector<int> locs=vector<int>()) const{
        int H = 2*boardSize-1;
        vector<string> grid(H, string(H,' '));

        for(int i=0;i<boardSize;i++){
            for(int j=0;j<boardSize;j++){
                if(head.x==i && head.y==j){
                    grid[2*i][2*j]='H';
                } else if(body[ID(i,j)]!=-1){
                    int d=body[ID(i,j)];
                    grid[2*i][2*j]='o';
                    grid[2*i+dir[d][0]][2*j+dir[d][1]] = (d%2==0 ? '-' : '|');
                } else if(apple.x==i && apple.y==j){
                    grid[2*i][2*j]='@';
                    for(int d=0; d<4; d++){
                        if(isValid(shiftPos(Pos{i, j}, d))){
                            grid[2*i+dir[d][0]][2*j+dir[d][1]] = '@';
                        }
                    }
                } else {
                    grid[2*i][2*j]='.';
                }
            }
        }

        for(int i=0; i<path.size(); i++){
            int d = path[i];
            Pos p = fromID(locs[i]);
            grid[2*p.x][2*p.y] = 'x' - 32 * (grid[2*p.x][2*p.y] != '.');
            grid[2*p.x - dir[d][0]][2*p.y - dir[d][1]] = (d%2==0 ? 'd' : 'v') - 32 * (grid[2*p.x - dir[d][0]][2*p.y - dir[d][1]] != ' ');
        }

        // system("clear"); // mac/linux (use "cls" on Windows)

        cout<<"Timer: "<<timer<<"\n";
        cout<<"Head: "<<head.x<<","<<head.y<<"\n";
        cout<<"Apple: "<<apple.x<<","<<apple.y<<"\n";
        cout<<"Size: "<<getSize()<<"\n";

        vector<vector<int>> releaseTimes(boardSize, vector<int>(boardSize, -1));

        Pos curr_tail = tail;
        int t = 0;
        while(curr_tail != head){
            releaseTimes[curr_tail.x][curr_tail.y] = t++;
            curr_tail = shiftPos(curr_tail, body[ID(curr_tail)]);
        }

        for(int i=9; i<H; i+=10){
            for(int j=9; j<H; j+=10){
                grid[i][j] = '#';
            }
        }

        for(int i=0; i<H; i++){
            for(int j=0; j<H; j++){
                char c = grid[i][j];
                if(c=='@') cout<<"\033[31m@\033[0m"; // red apple
                else if(c=='x') cout<<"\033[31mx\033[0m"; // red path
                else if(c=='d') cout<<"\033[31m-\033[0m"; // red horizontal path
                else if(c=='v') cout<<"\033[31m|\033[0m"; // red vertical path
                else if(c=='X') cout<<"\033[34mx\033[0m"; // red path
                else if(c=='D') cout<<"\033[34m-\033[0m"; // red horizontal path
                else if(c=='V') cout<<"\033[34m|\033[0m"; // red vertical path
                else cout<<c;
            }
            cout << "\t\t";
            if(i % 2 == 0){
                for(int j=0; j<boardSize; j++){
                    int r = releaseTimes[i/2][j];
                    if(r == -1) cout << "  .";
                    else cout << string(3 - to_string(r).size(), ' ') << r;
                }
            }
            cout<<"\n";
        }

        // cout.flush();
    }
};

const int AP_TYPE = 0;
const int BCC_TYPE = 1;
const int CC_TYPE = 2;

struct Component{
    int type;
    int id;

    bool operator == (const Component& other) const{
        return type == other.type && id == other.id;
    }
};

struct Edge{
    int a, b;

    Edge(int a_, int b_){
        if(a_ < b_){
            a = a_; b = b_;
        }
        else{
            a = b_; b = a_;
        }
    }

    bool operator == (const Edge& other) const{
        return a == other.a && b == other.b;
    }
};

struct EdgeHash {
    size_t operator()(const Edge& p) const {
        size_t h1 = std::hash<int>{}(p.a);
        size_t h2 = std::hash<int>{}(p.b);
        return h1 ^ (h2 << 1); // combine hashes
    }
};

// Custom hash
struct PosHash {
    size_t operator()(const Pos& p) const {
        size_t h1 = std::hash<int>{}(p.x);
        size_t h2 = std::hash<int>{}(p.y);
        return h1 ^ (h2 << 1); // combine hashes
    }
};

// Custom hash
struct ComponentHash {
    size_t operator()(const Component& p) const {
        size_t h1 = std::hash<int>{}(p.type);
        size_t h2 = std::hash<int>{}(p.id);
        return h1 ^ (h2 << 1); // combine hashes
    }
};

string compToString(const Component& comp){
    return (comp.type == AP_TYPE ? 'A' : 'C') + to_string(comp.id);
}


class TarjanDecomposition{
public:
    Snake env;

    TarjanDecomposition(){}

    TarjanDecomposition(Snake env_){
        env = env_;
        visitTime.assign(area, -1);
        minConnTime.assign(area, -1);
        isAP.assign(area, false);
        tarjan_timer = 0;

        // for(const Pos& neigh : env.emptyNeighs(env.head)){
        //     if(visitTime[ID(neigh)] == -1){
        //         assert(edgeQueue.size() == 0);
        //         tarjan(ID(neigh));
        //     }
        // }
        tarjan(ID(env.head));

        cellComps.assign(area, -1);
        getGraph();

        getRemainingComps();
    }

    vector<int> visitTime;
    vector<int> minConnTime;
    vector<bool> isAP;
    int tarjan_timer;

    
    vector<Edge> edgeQueue;
    vector<unordered_set<int>> BCCs;

    unordered_map<Component, unordered_set<Component, ComponentHash>, ComponentHash> compGraph;
    vector<int> cellComps;

    vector<unordered_set<int>> connectedComps;

    void dfs(int node, unordered_set<int>& lst){
        lst.insert(node);
        for(Pos neigh : env.emptyNeighs(fromID(node))){
            if(visitTime[ID(neigh)] == -1){
                visitTime[ID(neigh)] = INF;
                dfs(ID(neigh), lst);
            }
        }
    }

    void getRemainingComps(){
        int counter = BCCs.size();
        for(int i=0; i<area; i++){
            if(visitTime[i] == -1){
                unordered_set<int> comp;
                visitTime[i] = INF;
                dfs(i, comp);
                connectedComps.push_back(comp);
                for(const int& x : comp){
                    cellComps[x] = counter;
                }
                counter ++;
            }
        }
    }

    void flushQueue(int queueIndex){
        unordered_set<int> bcc;
        while(edgeQueue.size() > queueIndex){
            bcc.insert(edgeQueue[edgeQueue.size()-1].a);
            bcc.insert(edgeQueue[edgeQueue.size()-1].b);
            edgeQueue.pop_back();
        }
        BCCs.push_back(bcc);
    }

    void tarjan(int node, int parent=-1){
        visitTime[node] = minConnTime[node] = tarjan_timer ++;
        int n_children = 0;
        for(Pos neigh : env.emptyNeighs(fromID(node))){
            if(ID(neigh) == parent) continue;

            int queueIndex = edgeQueue.size();
            edgeQueue.push_back(Edge(node, ID(neigh)));

            if(visitTime[ID(neigh)] == -1){
                tarjan(ID(neigh), node);
                n_children += 1;
                if(parent != -1 && visitTime[node] <= minConnTime[ID(neigh)]){
                    isAP[node] = true;
                    flushQueue(queueIndex);
                }
                if(parent == -1){
                    flushQueue(queueIndex);
                }
                minConnTime[node] = min(minConnTime[node], minConnTime[ID(neigh)]);
            }
            else if(visitTime[ID(neigh)] < visitTime[node]){
                minConnTime[node] = min(minConnTime[node], visitTime[ID(neigh)]);
            }
            else{
                edgeQueue.pop_back();
            }
        }
        if(parent == -1 && n_children > 1){
            isAP[node] = true;
        }
        if(parent == -1 && n_children == 0){
            BCCs.push_back(unordered_set<int>{node});
        }
    }

    void getGraph(){
        for(int c=0; c<BCCs.size(); c++){
            Component comp = Component{BCC_TYPE, c};
            for(const int& x : BCCs[c]){
                if(isAP[x]){
                    Component neigh = Component{AP_TYPE, x};
                    compGraph[comp].insert(neigh);
                    compGraph[neigh].insert(comp);
                }
                else{
                    cellComps[x] = c;
                }
            }
        }
    }

    string toString(){
        string s = "";
        for(auto& [comp, adj] : compGraph){
            s += compToString(comp) + ": ";
            for(const Component& neigh : adj){
                s += " " + compToString(neigh);
            }
            s += '\n';
        }
        return s;
    }
};

class Solver{
public:

    Solver(){
        
    }

    /*
    Part I. To get the distance to apple:

    Step 1: compute max time snake could be at each point, assuming tail no longer retracted

    - Perform Tarjan's to get decomposition into BCCs connected by APs to form the block-cut tree

    - Note: there's a bug here:

        x . . x
        . . . H
        . . . x
        . x x x

        In this case, parity checks work, but the BCC of size 8 you can't actually visit all 8 cells

    - Perform DFS on the BCCs in the block cut tree to get max visit times, keeping track of parity counts in each block

    Step 2: Perform dynamic BFS where tail cells are freed at time t if it has a visited neighbor whose max time is > t



    Part II. Get distance to apple where we don't die

    Given: TJ decomp, maxTimes. 

    Perform dynamic BFS with tail retraction, filling rawMinTimes with min time can be at square without considering survival. As tail retracts, neighboring cells whose maxTimes > t are "freed" and minTimes of cells on the path of BCCs from it to the head are locked in to minTimes. cells whose maxTimes are <= t are added back to the BFS queue and considered unvisited.

    As an extension, we can also "free" cells outside the head component: free other connected components whenever the tail is neighboring it.


    Border-following heuristic


    */


    TarjanDecomposition decomp;
    vector<int> maxTimes;

    unordered_set<Component, ComponentHash> visitedComps;
    unordered_map<Component, Component, ComponentHash> parent;


    void getMaxTimes(Component comp, int start){
        if(comp.type == BCC_TYPE){
            int parityCounts[2] = {0, 0};
            for(const int& x : decomp.BCCs[comp.id]){
                parityCounts[manhattanDist(fromID(x), fromID(start)) % 2] ++;
            }
            for(const int& x : decomp.BCCs[comp.id]){
                if(x == start) continue;
                int parity = manhattanDist(fromID(x), fromID(start)) % 2;
                int m;
                if(parity == 0){
                    m = 2 * min(parityCounts[1], parityCounts[0]-1);
                }
                else{
                    m = 2 * min(parityCounts[1], parityCounts[0]) - 1;
                }
                maxTimes[x] = maxTimes[start] + m;
            }
        }
        for(const Component& neigh : decomp.compGraph[comp]){
            if(visitedComps.find(neigh) == visitedComps.end()){
                visitedComps.insert(neigh);
                parent[neigh] = comp;
                getMaxTimes(neigh, neigh.type == AP_TYPE ? -1 : comp.id);
            }
        }
    }

    void displayArray(const Snake& env, const vector<int>& arr, bool bodyDir=false){
        for(int i=0; i<area; i++){
            if(env.body[i] != -1){
                if(bodyDir) cout << "  " << env.body[i];
                else cout << "  x";
            }
            else if(arr[i] == -1) cout << "  .";
            else if(arr[i] == INF) cout << "INF";
            else cout << string(max(0, 3 - (int) (to_string(arr[i]).size())), ' ') << arr[i];
            if((i+1) % boardSize == 0) cout << '\n';
        }
    }
    
    vector<int> minTimes;

    Component compOfCell(int cell, const TarjanDecomposition& decomp){
        if(decomp.isAP[cell]){
            return Component{AP_TYPE, cell};
        }
        else if(decomp.cellComps[cell] < decomp.BCCs.size()){
            return Component{BCC_TYPE, decomp.cellComps[cell]};
        }
        return Component{CC_TYPE, decomp.cellComps[cell] - (int) decomp.BCCs.size()};
    }

    vector<int> freeTimes; // minimum time at which you can be at a cell and escape

    bool getMinTimes(const Snake& env, int start, int safety, bool print=false){
        // cout << "Calling...\n";

        // Get max times

        decomp = TarjanDecomposition(env);

        visitedComps = unordered_set<Component, ComponentHash>();
        parent = unordered_map<Component, Component, ComponentHash>();

        // Component initComp;
        // if(decomp.isAP[start]){
        //     initComp = Component{AP_TYPE, start};
        // }
        // else{
        //     initComp = Component{BCC_TYPE, decomp.cellComps[start]};
        // }
        Component initComp = compOfCell(start, decomp);
        visitedComps.insert(initComp);
        maxTimes.assign(area, -1);
        maxTimes[start] = 0;
        getMaxTimes(initComp, start);


        

        // Get visibility of tail and apple

        bool tail_vis = false;
        bool apple_vis = false;
        for(const auto& bcc : decomp.BCCs){
            for(const int& x : bcc){
                if(x == ID(env.apple)) apple_vis = true;
                for(const Pos& p : validNeighs(fromID(x))){
                    if(p == env.tail && x != ID(env.head)) tail_vis = true;
                }
            }
        }

        vector<bool> retractibleBody(area, false);
        for(int i=0; i<area; i++){
            if(env.body[i] != -1){
                retractibleBody[i] = true;
            }
        }

        vector<bool> visited(area, false);
        unordered_set<Pos, PosHash> queue{fromID(start)};
        visited[start] = true;

        Pos curr_tail = env.tail;

        // int shortestDist = INF;
        bool canEscape = false;

        minTimes.assign(area, INF);
        vector<int> rawMinTimes = vector<int>(area, INF);
        freeTimes.assign(area, INF);
        // vector<bool> reenterable = vector<bool>(area, true);
        // for(int i=0; i<area; i++){
        //     if(maxTimes[i] != -1) reenterable[i] = false;
        // }

        unordered_set<Component, ComponentHash> freedComponents;
        // unordered_set<Component, ComponentHash> reenterableComponents;

        for(int t=0; t<area; t++){
            // if(queue.find(env.apple) != queue.end()){
            //     shortestDist = min(shortestDist, t);
            // }
            // if(queue.size() > 0){
            //     cout << t << ": ";
            //     for(const Pos& p : queue){
            //         cout << env.PosToCode(p) << ' ';
            //     }
            //     cout << '\n';
            // }
            unordered_set<Pos, PosHash> next_queue;

            if(t >= safety){
                // cout << env.PosToCode(curr_tail) << '\n';
                for(const Pos& p : validNeighs(curr_tail)){
                    // if(!retractibleBody[ID(p)] && visited[ID(p)]){
                    if(!retractibleBody[ID(p)] && visited[ID(p)] && maxTimes[ID(p)] > t){
                        // cout << "Step " << t << " Inserting " + env.PosToCode(p) << '\n';
                        next_queue.insert(p);
                    }

                    if(maxTimes[ID(p)] > t){
                        canEscape = true;
                    }
                    // Free path of components to head
                    
                    Component comp = compOfCell(ID(p), decomp);
                    if(comp.type == CC_TYPE && freeTimes[ID(p)] == INF){
                        for(const int& x : decomp.connectedComps[comp.id]){
                            freeTimes[x] = t - safety;
                        }
                    }

                    if(maxTimes[ID(p)] != -1){
                        // Component currComp = compOfCell(ID(p), decomp);
                        // while(true){
                        //     if(reenterableComponents.contains(currComp)) break;
                        //     reenterableComponents.insert(currComp);
                        //     if(currComp.type == BCC_TYPE){
                        //         cout << "Step " << t << " Pos: " << env.PosToCode(p) << " Setting reenterable: " << compToString(currComp) << " size: " << decomp.BCCs[currComp.id].size() << '\n';
                        //         for(const int& x : decomp.BCCs[currComp.id]){
                        //             reenterable[x] = true;
                        //         }
                        //     }
                        //     if(!parent.contains(currComp)) break;
                        //     currComp = parent[currComp];
                        // }
                        Component currComp = compOfCell(ID(p), decomp);
                        while(true){
                            if(freedComponents.contains(currComp)){
                                break;
                            }
                            freedComponents.insert(currComp);
                            if(currComp.type == BCC_TYPE){
                                // cout << "Step " << t << " Pos: " << env.PosToCode(p) << " Setting free: " << compToString(currComp) << " size: " << decomp.BCCs[currComp.id].size() << '\n';
                                for(const int& x : decomp.BCCs[currComp.id]){
                                    freeTimes[x] = min(freeTimes[x], max(0, (t - safety) - max(0, maxTimes[ID(p)] - maxTimes[x])));
                                }
                            }
                            if(!parent.contains(currComp)) break;
                            currComp = parent[currComp];
                        }
                    }

                    
                    // if(maxTimes[ID(p)] > t){
                    //     // cout << "Step " << t << " Freed " + env.PosToCode(p) << '\n';
                    //     // canEscape = true;
                    // }
                }
                retractibleBody[ID(curr_tail)] = false;
                if(curr_tail != env.head){
                    curr_tail = shiftPos(curr_tail, env.body[ID(curr_tail)]);
                }
            }
            
            for(const Pos& p : queue){
                if(maxTimes[ID(p)] == -1){
                    minTimes[ID(p)] = min(minTimes[ID(p)], t);
                }
                else{
                    rawMinTimes[ID(p)] = min(rawMinTimes[ID(p)], t);
                    // if(freed[ID(p)]){
                    //     minTimes[ID(p)] = min(minTimes[ID(p)], rawMinTimes[ID(p)]);
                    // }
                }
                
                if((t - manhattanDist(p, fromID(start))) % 2 == 0){
                    for(const Pos& q : validNeighs(p)){
                        if(!retractibleBody[ID(q)] && !visited[ID(q)]){
                            visited[ID(q)] = true;
                            next_queue.insert(q);
                        }
                        if(!retractibleBody[ID(q)] && freeTimes[ID(q)] != INF && minTimes[ID(q)] == INF){
                            minTimes[ID(q)] = min(minTimes[ID(q)], t+1);
                            next_queue.insert(q);
                        }
                    }
                }
                else{ // for off-parity insertions in the retraction
                    next_queue.insert(p);
                }
            }
            
            
            queue = next_queue;
        }

        if(print){
            cout << "\nRaw min times:\n";
            displayArray(env, rawMinTimes);
            cout << "\nFree Times:\n";
            displayArray(env, freeTimes);
            cout << "\nMax Times:\n";
            displayArray(env, maxTimes);
            cout << "\nCell comps:\n";
            displayArray(env, decomp.cellComps);
            cout << decomp.toString() << '\n';
        }
        

        for(int i=0; i<area; i++){
            if(maxTimes[i] != -1){
                minTimes[i] = min(minTimes[i], max(rawMinTimes[i], freeTimes[i]));
            }
        }

        return canEscape;

        // if(!canEscape){
        //     for(int i=0; i<area; i++){
        //         minTimes[i] = INF;
        //     }
        // }

        // return shortestDist;

        // return manhattanDist(env.head, env.apple);
        
    }

    double distanceHeuristic(const Snake& env, int safety){
        if(env.validMoves().size() == 0) return INF;

        bool canEscape = getMinTimes(env, ID(env.head), safety, false);
        if(!canEscape) return INF;

        int immediate_dist = minTimes[ID(env.apple)];
        // cout << "Immediate dist: " << immediate_dist << '\n';

        // return immediate_dist;


        // Snake env_ = env;

        // for(int i=0; i<immediate_dist-1; i++){
        //     env_.body[ID(env_.tail)] = -1;
        //     if(env_.tail != env.head){
        //         env_.tail = shiftPos(env_.tail, env.body[ID(env_.tail)]);
        //     }
        // }

        // env_.body[ID(env.apple)] = -1;

        // env_.display();

        // getMinTimes(env_, ID(env.apple), 0);

        // displayArray(env_, minTimes);




        const double diffusiveTime = 12; // approximate for how long it takes to reach each apple.

        double sumWeights = 0;
        double sumTotal = 0;

        for(int i=0; i<area; i++){
            if(env.body[i] == -1){
                double weight = 1;
                // double weight = minTimes[i] + diffusiveTime;
                sumWeights += weight;
                sumTotal += max(0., freeTimes[i] - (diffusiveTime + immediate_dist)) * weight;
            }
        }

        return immediate_dist + (double) sumTotal / sumWeights;
    }

    vector<int> shortestPath(const Snake& env, bool printMode=false){
        int maxTime = area;

        vector<int> moves;
        Snake curr_env = env;

        for(int i=0; i<maxTime; i++){
            if(printMode){
                cout << curr_env.toCode() << '\n';
                curr_env.display();
            }

            double minDist = INF;
            int bestAction = -1;
            // cout << "Finding dists...\n";
            for(int safety=3; safety>=0; safety--){
                for(int d : curr_env.validMoves()){
                    Snake newEnv = curr_env;
                    newEnv.move(d);
                    double dist = distanceHeuristic(newEnv, safety);
                    if(printMode)
                        cout << "Action " << d << " dist: " << dist << '\n';
                    if(minDist > dist){
                        minDist = dist;
                        bestAction = d;
                    }
                }
                if(minDist != INF){
                    // cout << "Required safety: " << safety << '\n';
                    break;
                }
            }
            
            if(bestAction == -1){
                bestAction = curr_env.validMoves()[0];
                // break;
            }
            curr_env.move(bestAction);
            moves.push_back(bestAction);
            if(curr_env.head == curr_env.apple || curr_env.validMoves().size() == 0){
                break;
            }
        }
        return moves;
    }

    vector<int> appleTimes;
    Snake lastState;

    int COMPLETE = 0;
    int TRAPPED = 1;
    int LOOPED = 2;
    int USR_TERM = 3;

    int simulate(Snake env, bool printMode=false, bool stepMode=false){ // return end cause
        string s;

        int num_repeats = 0;

        appleTimes = vector<int>();

        for(int i=0; i<area*2; i++){
            vector<int> path = shortestPath(env);
            TarjanDecomposition tj_decomp(env);
            Snake newEnv = env;
            vector<int> headLoc;
            for(int d : path){
                newEnv.move(d);
                headLoc.push_back(ID(newEnv.head));
            }

            if(printMode){
                cout << tj_decomp.toString();
                cout << env.toCode() << '\n';
                env.display(path, headLoc);
                getMinTimes(env, ID(env.head), 0, false);
                cout << "minTimes:\n";
                displayArray(env, minTimes);
            }

            env = newEnv;
            if(env.head == env.apple){
                num_repeats = 0;
                appleTimes.push_back(env.timer);
                if(env.getSize() == area) return COMPLETE;
                env.randomizeApple();
            }
            else{
                num_repeats++;
            }
            
            if(env.validMoves().size() == 0 || path.size() == 0) return TRAPPED;
            if(s.size() > 0) return USR_TERM;
            if(num_repeats >= 2) return LOOPED;

            lastState = env;

            if(stepMode)
                getline(cin, s);
        }
        return TRAPPED;
    }

    void runTrials(int numTrials, string outFile, string codeFile){
        ofstream fout(outFile);
        ofstream codeOut(codeFile);

        int totalSize = 0;
        int totalTime = 0;
        cout << "Running " << numTrials << " trials...\n";
        for(int i=0; i<numTrials; i++){
            int endCause = simulate(Snake());

            for(const int& t : appleTimes){
                fout << t << ' ';
            }
            fout << '\n';
            codeOut << lastState.toCode() << '\n';

            int size = 2 + appleTimes.size();
            int time = appleTimes[appleTimes.size()-1];
            totalSize += size;
            totalTime += time;
            cout << "Game " << i << " size: " << size << " time: " << time << " termination: " << endCause << '\n';
        }
        cout << "Average size: " << ((double) totalSize / numTrials) << '\n';
        cout << "Average time: " << ((double) totalTime / numTrials) << '\n';
    }
};




int main(){
    // srand(4392);
    // srand(6937);
    srand(10759);

    Solver s;

    // Snake env;

    // Snake env("6x8_9x8_6x9_302_aaaaaaaaaaaaaaaaaaaaaaaaaaaaaabgggaesssagggjaxsssp");
    // env.move(0);
    // env.move(0);
    // env.move(0);
    // env.move(3);

    // Snake env("19x22_1x7_10x21_2549_aaaaaaaaaaghaaaaaacaaaaaaxraaaaaacaaaaaaawaaaaaacaaaaaaawaaaaaacaaaaaaawaaaaaacaaaaaaawaaaaaacaaaaaaawaaaaaacaaaggggwaaaaaacaaauaaacaaaaaacaaauaaacaaaaaacaaauaaacaaaaaacaaauaaacaaaaaacaaauaaacaaaaaacaaauaaacaaaaaacaaaxsssraaaaaabgggggggwaaaaaaaaaaaaaacaaaaaaaaaagghacaaaaaaaaaauacacaaaaaaaaaauabgcaaaaaaaaaauaaacaaaaaaaaaauaaacaaaaaaaaaaxssssaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");
    // env.move(0);

    // cout << s.simulate(env, true, true) << "\n";


    // env.display();

    // s.displayArray(env, env.body, true);

    // TarjanDecomposition tj_decomp(env);
    // cout << tj_decomp.toString() << '\n';


    // s.getMinTimes(env, ID(env.head), 0, true);
    // cout << "Min times:\n";
    // s.displayArray(env, s.minTimes);
    // cout << s.distanceHeuristic(env, 0) << '\n';

    // s.displayArray(env, s.maxTimes);

    s.runTrials(10, "score.out", "code.out");
}