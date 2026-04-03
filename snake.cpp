
/*
g++ -O2 -std=c++23 snake.cpp && ./a.out

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
#include <queue>
using namespace std;

typedef pair<int, int> pii;

const int boardSize = 30;
const int area = boardSize * boardSize;

const int EMPTY = -1;
const int INF = 1 << 20;

// const double diffusiveTime = 50; // approximate for how long it takes to reach each apple.
// const double hamConstant = 0.1; // how strongly endgame algorithm follows ham cycle
// const int endGameCutoff = 700;
// const double decayRate = 0.01;

const double diffusiveTime = 50; // approximate for how long it takes to reach each apple.
const double hamConstant = 0.1; // how strongly endgame algorithm follows ham cycle
const int endGameCutoff = 700;
const double decayRate = 0.02;

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

    vector<int> releaseTimes;
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
        computeReleaseTimes();
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
        computeReleaseTimes();
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
        computeReleaseTimes();
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

    void computeReleaseTimes(){
        releaseTimes = vector<int>(area, -1);
        Pos curr_tail = tail;
        int t = 0;
        while(curr_tail != head){
            releaseTimes[ID(curr_tail)] = t++;
            curr_tail = shiftPos(curr_tail, body[ID(curr_tail)]);
        }
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
                    int r = releaseTimes[ID(Pos{i/2, j})];
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

    string toString() const{
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

class Features{
public:

    TarjanDecomposition decomp;
    vector<int> maxTimes;
    vector<int> freeTimes; // minimum time at which you can be at a cell and escape

    Features(){}

    Features(const Snake& env){

        decomp = TarjanDecomposition(env);

        visitedComps = unordered_set<Component, ComponentHash>();
        parent = unordered_map<Component, Component, ComponentHash>();
        Component initComp = compOfCell(ID(env.head));
        visitedComps.insert(initComp);
        maxTimes.assign(area, -1);
        maxTimes[ID(env.head)] = 0;
        getMaxTimes(initComp, ID(env.head));



        computeFreeTimes(env);
        
    }

private:
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

    Component compOfCell(int cell){
        if(decomp.isAP[cell]){
            return Component{AP_TYPE, cell};
        }
        else if(decomp.cellComps[cell] < decomp.BCCs.size()){
            return Component{BCC_TYPE, decomp.cellComps[cell]};
        }
        return Component{CC_TYPE, decomp.cellComps[cell] - (int) decomp.BCCs.size()};
    }

    void computeFreeTimes(const Snake& env){
        freeTimes.assign(area, INF);

        Pos curr_tail = env.tail;
        unordered_set<Component, ComponentHash> freedComponents;

        for(int t=0; t<area; t++){
            for(const Pos& p : validNeighs(curr_tail)){
                
                Component comp = compOfCell(ID(p));
                if(comp.type == CC_TYPE && freeTimes[ID(p)] == INF){
                    for(const int& x : decomp.connectedComps[comp.id]){
                        freeTimes[x] = max(0, t - (int) decomp.connectedComps[comp.id].size());
                    }
                }

                
                // Free path of components to head

                if(maxTimes[ID(p)] != -1){
                    Component currComp = compOfCell(ID(p));
                    vector<Component> BCCsToRoot;
                    while(true){
                        if(freedComponents.contains(currComp)){
                            break;
                        }
                        freedComponents.insert(currComp);
                        if(currComp.type == BCC_TYPE){
                            BCCsToRoot.push_back(currComp);
                        }
                        if(!parent.contains(currComp)) break;
                        currComp = parent[currComp];
                    }
                    int size = 1;
                    for(int i=BCCsToRoot.size()-1; i>=0; i--){
                        Component currComp = BCCsToRoot[i];
                        size += decomp.BCCs[currComp.id].size() - 1;
                        for(const int& x : decomp.BCCs[currComp.id]){
                            freeTimes[x] = min(freeTimes[x], max(0, t - size));
                        }
                    }
                }
            }
            if(curr_tail != env.head){
                curr_tail = shiftPos(curr_tail, env.body[ID(curr_tail)]);
            }
        }
    }

};

class Solver{
public:
    vector<int> hamCycle;
    vector<int> hamCycle2;

    vector<int> convertToCycle(vector<int> moves){
        Pos start{0, 0};
        vector<int> ans(area, -1);
        for(int i=0; i<area; i++){
            ans[ID(start)] = moves[i];
            start = shiftPos(start, moves[i]);
        }
        return ans;
    }

    Solver(){

        vector<int> moves;
        moves.append_range(vector<int>(boardSize-1, 1));
        for(int i=0; i<boardSize/2-1; i++){
            moves.push_back(0);
            moves.append_range(vector<int>(boardSize-2, 3));
            moves.push_back(0);
            moves.append_range(vector<int>(boardSize-2, 1));
        }
        moves.push_back(0);
        moves.append_range(vector<int>(boardSize-1, 3));
        moves.append_range(vector<int>(boardSize-1, 2));

        hamCycle = convertToCycle(moves);

        // moves = vector<int>();
        // moves.append_range(vector<int>(boardSize-1, 1));
        // moves.append_range(vector<int>(boardSize-1, 0));
        // moves.append_range(vector<int>(boardSize-1, 3));
        // for(int i=0; i<boardSize/2-1; i++){
        //     moves.push_back(2);
        //     moves.append_range(vector<int>(boardSize-2, 1));
        //     moves.push_back(2);
        //     moves.append_range(vector<int>(boardSize-2, 3));
        // }
        // moves.push_back(2);

        moves = vector<int>();
        moves.append_range(vector<int>(boardSize-1, 0));
        for(int i=0; i<boardSize/2-1; i++){
            moves.append_range(vector<int>{1, 2, 1, 0});
        }
        moves.append_range(vector<int>{1, 2});
        for(int i=0; i<boardSize/2-2; i++){
            moves.push_back(2);
            moves.append_range(vector<int>(boardSize-2, 3));
            moves.push_back(2);
            moves.append_range(vector<int>(boardSize-2, 1));
        }
        moves.append_range(vector<int>{2, 2});
        for(int i=0; i<boardSize/2-1; i++){
            moves.append_range(vector<int>{3, 0, 3, 2});
        }
        moves.push_back(3);

        hamCycle2 = convertToCycle(moves);
        

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


    bool getMinTimes(const Snake& env, const Features& features, int safety, bool print=false){


        vector<bool> retractibleBody(area, false);
        for(int i=0; i<area; i++){
            if(env.body[i] != -1){
                retractibleBody[i] = true;
            }
        }

        vector<bool> visited(area, false);
        unordered_set<Pos, PosHash> queue{env.head};
        visited[ID(env.head)] = true;

        Pos curr_tail = env.tail;

        bool canEscape = false;

        minTimes.assign(area, INF);
        vector<int> rawMinTimes = vector<int>(area, INF);


        for(int t=0; t<area; t++){
            unordered_set<Pos, PosHash> next_queue;

            if(t >= safety){
                for(const Pos& p : validNeighs(curr_tail)){
                    if(!retractibleBody[ID(p)] && visited[ID(p)] && features.maxTimes[ID(p)] > t){
                        next_queue.insert(p);
                    }
                    if(features.maxTimes[ID(p)] > t){
                        canEscape = true;
                    }
                }
                retractibleBody[ID(curr_tail)] = false;
                if(curr_tail != env.head){
                    curr_tail = shiftPos(curr_tail, env.body[ID(curr_tail)]);
                }
            }
            
            for(const Pos& p : queue){
                if(features.maxTimes[ID(p)] == -1){
                    minTimes[ID(p)] = min(minTimes[ID(p)], t);
                }
                else{
                    rawMinTimes[ID(p)] = min(rawMinTimes[ID(p)], t);
                }
                
                if((t - manhattanDist(p, env.head)) % 2 == 0){
                    for(const Pos& q : validNeighs(p)){
                        if(!retractibleBody[ID(q)] && !visited[ID(q)]){
                            visited[ID(q)] = true;
                            next_queue.insert(q);
                        }
                        if(!retractibleBody[ID(q)] && features.freeTimes[ID(q)] < t-safety && minTimes[ID(q)] == INF){
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
            displayArray(env, features.freeTimes);
            cout << "\nMax Times:\n";
            displayArray(env, features.maxTimes);
            cout << "\nCell comps:\n";
            displayArray(env, features.decomp.cellComps);
            cout << features.decomp.toString() << '\n';
        }
        

        for(int i=0; i<area; i++){
            if(features.maxTimes[i] != -1){
                minTimes[i] = min(minTimes[i], max(rawMinTimes[i], features.freeTimes[i]));
            }
        }


        return canEscape;

    }

    vector<double> distanceHeuristic(const Snake& env, const Features& features, int safety, double distFactor=1){ // returns minDist and weighted freeTime heuristic.
        if(env.validMoves().size() == 0) return vector<double>{INF, INF};

        assert(features.maxTimes.size() > 0);

        bool canEscape = getMinTimes(env, features, safety, false);
        if(!canEscape) return vector<double>{INF, INF};

        int immediate_dist = minTimes[ID(env.apple)];

        double sumWeights = 0;
        double sumTotal = 0;

        for(int i=0; i<area; i++){
            if(env.body[i] == -1){
                double weight = features.freeTimes[i] + diffusiveTime;
                sumWeights += weight;
                sumTotal += max(0., features.freeTimes[i] - immediate_dist * distFactor)* weight;
            }
        }

        return vector<double>{(double) immediate_dist, (double) sumTotal / sumWeights};
    }

    /*
    Path casework:

    Idea: try multiple different paths to the apple and choose the best one according to some metric, say, length of path plus weighted freeTime heuristic

    Border-touching path: while the head and apple are in the same component and the current Manhattan distance to apple is greater than the minimum Manhattan distance of a reachable border cell to the apple, follow the border.
        - A border cell has one of its 8 surrounding cells either the board edge or a body cell other than the head or the one behind the head
            - a cell where one of its 8 surrounding cells is the tail is NOT a border cell
        - Reachable border cells are the ones that are reachable by a path of border cells from the head
    
    Border-following distance metric:
        Locate reachable border cells that are closest to the apple
        What is the shortest path of border cells to one of these closest points?
    
    */


    vector<bool> isBorder;
    vector<int> borderDist;

    pii computeBorderAndDist(const Snake& env){

        isBorder.assign(area, false);
        for(int i=0; i<area; i++){
            if(env.body[i] != -1) continue;
            Pos p = fromID(i);
            bool tailNeigh = false;
            for(int a=-1; a<=1; a++){
                for(int b=-1; b<=1; b++){
                    Pos q{p.x + a, p.y + b};
                    if(!isValid(q) || (env.body[ID(q)] != -1 && env.releaseTimes[ID(q)] < env.getSize() - 2)){
                        isBorder[i] = true;
                    }
                    if(q == env.tail){
                        tailNeigh = true;
                    }
                }
            }
            if(tailNeigh) isBorder[i] = false;
        }

        // BFS to find distance from head while following border
        borderDist.assign(area, INF);
        queue<pii> queue;
        queue.push(make_pair(ID(env.head), 0));

        vector<bool> visited(area, false);
        visited[ID(env.head)] = true;

        while(queue.size() > 0){
            pii top = queue.front();
            queue.pop();
            borderDist[top.first] = top.second;
            for(const Pos& p : validNeighs(fromID(top.first))){
                if(!visited[ID(p)] && isBorder[ID(p)]){
                    visited[ID(p)] = true;
                    queue.push(make_pair(ID(p), top.second + 1));
                }
            }
        }

        int minDist = INF;
        for(int i=0; i<area; i++){
            if(borderDist[i] != INF){
                minDist = min(minDist, manhattanDist(fromID(i), env.apple));
            }
        }

        int closestMin = INF;
        for(int i=0; i<area; i++){
            if(manhattanDist(fromID(i), env.apple) == minDist){
                closestMin = min(closestMin, borderDist[i]);
            }
        }

        // displayArray(env, vector<int>(isBorder.begin(), isBorder.end()));
        // displayArray(env, borderDist);
        return make_pair(minDist, closestMin);
    }

    int freePolicy(const Snake& env, int safety, bool printMode=false){
        double minDist = INF;
        int bestAction = -1;
        for(int d : env.validMoves()){
            Snake newEnv = env;
            newEnv.move(d);
            vector<double> heuristic = distanceHeuristic(newEnv, neighFeatures[d], safety);
            double dist = heuristic[0] + heuristic[1];
            if(printMode){
                cout << "Action " << d << " dist: " << dist << '\n';
            }
            if(minDist > dist){
                minDist = dist;
                bestAction = d;
            }
        }
        
        return bestAction;
    }

    int borderPolicy(const Snake& env, int safety, bool printMode=false){
        // Check if head and apple are in same component
        TarjanDecomposition decomp(env);
        bool sameComp = false;
        for(const auto& comp : decomp.BCCs){
            if(comp.contains(ID(env.head)) && comp.contains(ID(env.apple))){
                sameComp = true;
            }
        }
        if(!sameComp) return freePolicy(env, safety);

        pii borderDists = computeBorderAndDist(env);

        if(borderDists.second == 0) return freePolicy(env, safety);

        // vector<int> borderFollowingMoves;
        vector<bool> borderFollowingMoves(4, false);
        int counter = 0;
        for(const int& a : env.validMoves()){
            if(isBorder[ID(shiftPos(env.head, a))]){
                borderFollowingMoves[a] = true;
                counter += 1;
            }
        }

        if(counter == 0) return freePolicy(env, safety);

        // Dist is determined by dist from border to apple and secondary distance metrics.
        pair<int, double> minDist{INF, INF};
        int bestAction = -1;
        for(const int& a : env.validMoves()){
            if(!borderFollowingMoves[a]) continue;
            Snake newEnv = env;
            newEnv.move(a);
            pii borderDists = computeBorderAndDist(newEnv);

            vector<double> heuristic = distanceHeuristic(newEnv, neighFeatures[a], safety);
            if(heuristic[0] == INF) continue;

            pair<int, double> candDist{borderDists.first, borderDists.second + heuristic[1]};
            // pair<int, double> candDist{borderDists.first + (borderFollowingMoves[a] ? 0 : INF/2), borderDists.second + heuristic[1]};
            if(printMode){
                cout << "Action " << a << " dists: " << borderDists.first << ' ' << borderDists.second << ' ' << heuristic[1] << '\n';
            }
            if(minDist.first > candDist.first || (minDist.first == candDist.first && minDist.second > candDist.second)){
                minDist = candDist;
                bestAction = a;
            }
        }
        return bestAction;
    }

    int endgamePolicy(const Snake& env, int safety, bool printMode=false){

        // whichever ham cycle it currently shares the most segments with, follow that one

        int maxCount = -1;
        vector<int> bestCycle;
        for(const auto& cycle : vector<vector<int>>{hamCycle, hamCycle2}){
            int counter = 0;
            for(int i=0; i<area; i++){
                if(env.body[i] == cycle[i]) counter ++;
            }
            if(maxCount < counter){
                maxCount = counter;
                bestCycle = cycle;
            }
        }

        double minDist = INF;
        int bestAction = -1;

        // double hamFactor = 1 - exp(-decayRate * (env.getSize() - endGameCutoff));
        double hamFactor = min(1., decayRate * (env.getSize() - endGameCutoff));
        
        for(const int& a : env.validMoves()){
            Snake newEnv = env;
            newEnv.move(a);
            vector<double> heuristic = distanceHeuristic(newEnv, neighFeatures[a], safety, 0);
            double candDist = heuristic[0] * (1-hamFactor) + (a != bestCycle[ID(env.head)]) * (area * hamConstant * hamFactor) + heuristic[1];
            // if(heuristic[1] == INF) continue;
            // double candDist = a != bestCycle[ID(env.head)];
            if(printMode){
                cout << "Action " << a << " dists: " << (a != bestCycle[ID(env.head)]) << ' ' << heuristic[1] << '\n';
                // displayArray(newEnv, freeTimes);
            }
            if(minDist > candDist){
                minDist = candDist;
                bestAction = a;
            }
        }
        return bestAction;
    }

    const int FREE_POLICY = 0;
    const int BORDER_POLICY = 1;
    const int ENDGAME_POLICY = 2;

    Features neighFeatures[4];

    vector<int> shortestPath(const Snake& env, int policy, bool printMode=false, int maxTime=area){

        vector<int> moves;
        Snake curr_env = env;

        for(int i=0; i<maxTime; i++){
            if(printMode){
                cout << curr_env.toCode() << '\n';
                curr_env.display();
            }

            for(const int& a : curr_env.validMoves()){
                Snake newEnv = curr_env;
                newEnv.move(a);
                neighFeatures[a] = Features(newEnv);
            }

            int action = -1;
            for(int safety=3; safety>=0; safety--){
                if(policy == FREE_POLICY){
                    action = freePolicy(curr_env, safety, printMode);
                }
                else if(policy == BORDER_POLICY){
                    action = borderPolicy(curr_env, safety, printMode);
                }
                else if(policy == ENDGAME_POLICY){
                    action = endgamePolicy(curr_env, safety, printMode);
                }
                else{
                    assert(false);
                }
                if(action != -1) break;
            }
            
        
            if(action == -1){
                action = curr_env.validMoves()[0];
            }

            
            curr_env.move(action);
            moves.push_back(action);
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

    int simulate(Snake env, int printMode=0, int fastForward=INF, string logFile=""){ // return end cause
        string s;

        int num_repeats = 0;

        appleTimes = vector<int>();

        ofstream fout(logFile);

        for(int i=0; i<area*2; i++){
            

            vector<vector<int>> paths;
            if(env.getSize() < endGameCutoff){
                vector<int> free_path = shortestPath(env, FREE_POLICY, printMode>=2);
                vector<int> border_path = shortestPath(env, BORDER_POLICY, printMode>=2);
                paths = vector<vector<int>>{free_path, border_path};
            }
            else{
                vector<int> endgame_path = shortestPath(env, ENDGAME_POLICY, printMode>=2);
                paths = vector<vector<int>>{endgame_path};
            }
            
            vector<Snake> envs;
            for(const auto& path : paths){
                Snake curr_env = env;
                for(int d : path){
                    curr_env.move(d);
                }
                envs.push_back(curr_env);
            }


            double minDist = INF;
            int bestPath = -1;
            for(int safety=3; safety>=0; safety--){
                for(int j=0; j<paths.size(); j++){
                    vector<double> heuristic = distanceHeuristic(envs[j], Features(envs[j]), safety);
                    double candDist = paths[j].size() + heuristic[0] + heuristic[1];
                    if(printMode >= 1){
                        cout << "Safety " << safety << " Path " << j << " Dist: " << candDist << '\n';
                    }
                    if(minDist > candDist){
                        minDist = candDist;
                        bestPath = j;
                    }
                }
                if(bestPath != -1){
                    break;
                }
            }
            if(bestPath == -1) bestPath = 0;
            vector<int> path = paths[bestPath];
            
            Snake newEnv = env;
            vector<int> headLoc;
            for(int d : path){
                newEnv.move(d);
                headLoc.push_back(ID(newEnv.head));
            }

            if(printMode >= 2){
                cout << "Calling minTimes in printing\n";
                getMinTimes(env, Features(env), 0, true);
                cout << "minTimes:\n";
                displayArray(env, minTimes);
            }
            if(printMode >= 1){
                TarjanDecomposition tj_decomp(env);
                cout << tj_decomp.toString();
                cout << env.toCode() << '\n';
                env.display(path, headLoc);
            }

            if(logFile != ""){
                fout << env.toCode() << '\n';
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

            if(env.getSize() >= fastForward)
                getline(cin, s);
        }
        return TRAPPED;
    }

    void runTrials(int numTrials, string outFile, string codeFile){
        ofstream fout(outFile);
        ofstream codeOut(codeFile);

        int totalSize = 0;
        int totalTime = 0;
        int numWins = 0;
        int winTime = 0;
        cout << "Running " << numTrials << " trials...\n";
        for(int i=0; i<numTrials; i++){
            int endCause = simulate(Snake(), 0, INF, "state_logs/game_" + to_string(i) + ".txt");

            for(const int& t : appleTimes){
                fout << t << ' ';
            }
            fout << '\n';
            codeOut << lastState.toCode() << '\n';

            int size = 2 + appleTimes.size();
            int time = appleTimes[appleTimes.size()-1];
            totalSize += size;
            totalTime += time;
            if(size == area){
                numWins ++;
                winTime += time;
            }
            cout << "Game " << i << " size: " << size << " time: " << time << " termination: " << endCause << '\n';
        }
        cout << "Average size: " << ((double) totalSize / numTrials) << '\n';
        cout << "Average time: " << ((double) totalTime / numTrials) << '\n';
        cout << "Win rate: " << ((double) numWins / numTrials) << '\n';
        cout << "Average win time: " << ((double) winTime / numWins) << '\n';
    }
};




int main(){
    srand(1234);

    unsigned long start_time = time(0);

    Solver s;

    // Snake env;

    // s.displayArray(env, s.hamCycle);
    // cout << '\n';
    // s.displayArray(env, s.hamCycle2);
    // cout << '\n';

    // Snake env("6x8_9x8_6x9_302_aaaaaaaaaaaaaaaaaaaaaaaaaaaaaabgggaesssagggjaxsssp");
    // env.move(0);
    // env.move(0);
    // env.move(0);
    // env.move(3);

    // Snake env("1x8_6x0_8x0_47993_ggggggglgggggghxaaadnnjnnnnnnnjaaaeoonoooooohxaaaeooooooooonjaaaeooooooooohxaaaeooooooooonjaaaeooooooooohaaaaeooooooooonaaaaeooooooooohbggloooooooooonjnnooooooootttpxooooooooogggghjoooootootsssscxoooolotlllllonjooootlotooooohxooolooloooooonjooooooooooooohxoooooooooojjjnjoooooooooonnnhxooooooooooooonjooooooooooooohxooooooooooooonjooooooooooooohxooooooooooooonjooooooooooooohxooooooooooooonjooooooooooooohxooooooooooooonjoojjjjjjjjooohxttssssssssttts");
    // Snake env("20x29_28x12_12x29_46509_gggggggggggggghxnnnnnnnnnnnnnnjooooooooooooohxooooooooooooonjooooooooooooohxooooooooooooonjooooooooooooohxooooooooooooonjooooooooooooohxooooooooooooonjooooooooooooohxooooooooooooonjooooooooooootpxooooooootttqlkjooooottqgggjjhxoooolggjnssssnjoooojnnsqlllohxoootnollooooonjooketttoooooohxookaabooooooonjookaaeooooooofxookaaeoooooooajookaaeoooooooaxookaaeoooooooajookaaeoooooooaxookaaeoooooooajookaaeoooooooaxookaaeoooooooajookaajjjjjjjjaxttpaaaaaaaaaaa");
    // Snake env("4x20_4x18_10x29_49574_gggggggggggggghxnnnnnnnnannnnnjooooooooaoooohxooottoooaoooonjooggooookeooohxoonnooookeooonjooooooookeooohxooooooookeooonjooooooookeooohxooooooookeooonjoooootttpetttpxoooogggggglllhjoooonnnnnnjoonxoooooooooonoohjooooooooooooonxooooooooooooohjooooooooooooonxooooooooooooohjooooooooooooonxooooooooooooohjooooooooooooonxooooooooooooohjooooooooooooonxooooooooooooohjooooooooooooonxooooooooooooohjooooooooooooonxooooooooooooohjooooooooooojjcxtttttttttttsss");
    // Snake env("0x18_3x19_5x29_48088_ggggghgggaaaaaaxnnnnnxnnnaaaaajoooolooooaaaaaxoooooooooaaaaajooooooookaaaaaxooottoookaablkjoolloooopaboohxooooooolgkeoonjoooooooonkeoohxooooooooolooonjooooooooojjoohxooooooooonnoonjooooooooooooohxooooooooooooonjooooooooooooohxoooooootooooonjoooojjgjooooohxoooonnnnooooonjooooooooooooohxooooooooooooonjootjooooooooohxoqjnooooooooonjgjnoooooooooohxnnooooooooooonjooooooooooooohxooooooooooooonjooooooooooooohxooooooooooooonjooooooooooojjhxtttttttttttsss");
    // Snake env("3x14_1x9_17x29_45323_gggggggggggggghxnnnnaannnnnnnnjoookaaooooooohxoookaaeoooooonjoookaaeoooooohxoookaaeoooooonjooopaaeoooooohxoogkaaeoooooonjoonkaaeoooooohxoookaaeoooooonjoookaaeoooooohxooogkaeoooooonjooonkaetttttohxoooogggggggoonjoooonnnnnnnothxooooooooooolonjooooooooooooopxoooooooooooolkjooooooooooooohxooooooooooooonjooooooooooooohxooooooooooooonjooooooooooooohxooooooooooooonjooooooooooooohxooooooooooooonjooooooooooooohxooooooooooooonjooooooooooojjhxtttttttttttsss");
    // env.move(2);
    
    // env.move(1);
    // env.move(0);
    // env.move(0);
    // env.move(0);

    // cout << s.simulate(env, 1, 1000) << "\n";

    // s.endgamePolicy(env, true);


    // env.display();
    // s.shortestPath(env, s.BORDER_POLICY, true);
    // s.computeBorder(env);

    // s.displayArray(env, env.body, true);

    // TarjanDecomposition tj_decomp(env);
    // cout << tj_decomp.toString() << '\n';


    // s.getMinTimes(env, Features(env), 1, true);
    // cout << "Min times:\n";
    // s.displayArray(env, s.minTimes);
    // vector<double> heuristic = s.distanceHeuristic(env, Features(env), 1);
    // cout << heuristic[0] << ' ' << heuristic[1] << '\n';

    // s.displayArray(env, s.maxTimes);

    s.runTrials(50, "score.out", "code.out");

    cout << "Ellapsed time: " << (time(0) - start_time) << '\n';
}