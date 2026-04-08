
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


// const int boardSize = 10;

// const double diffusiveTime = 10; // approximate for how long it takes to reach each apple.
// const double hamBonus = 100; // how strongly endgame algorithm follows ham cycle
// const int endGameCutoff = 60;
// const double decayRate = 1;
// const double borderBonus = 5;

// const int maxSafety = 4;
// const int ignoreAppleSafetyThreshold = 1;

// const int forceCycleCutoff = 80;


const int boardSize = 30;

const double diffusiveTime = 50; // approximate for how long it takes to reach each apple.
const double hamBonus = 100; // how strongly endgame algorithm follows ham cycle
const int endGameCutoff = 800;
const double decayRate = 1;
const double borderBonus = 10;

const int maxSafety = 6;
const int ignoreAppleSafetyThreshold = 3;
const int endGameSafety = 2;

const int forceCycleCutoff = 850;





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

bool isValid(Pos p){
    return 0 <= p.x && p.x < boardSize && 0 <= p.y && p.y < boardSize;
}
int ID(Pos p){
    assert(isValid(p));
    return p.x*boardSize + p.y;
}
int ID(int x, int y){
    return x*boardSize + y;
}
Pos fromID(int id){
    return Pos{id / boardSize, id % boardSize};
}

string PosToString(Pos p){
    return to_string(p.x) + '*' + to_string(p.y);
}

int dir[4][2] = {{0, 1}, {1, 0}, {0, -1}, {-1, 0}};


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
        assert(sections[4].size() == area / 2);
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
        assert(0 <= d && d < 4);
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
                } else if(body[ID(i, j)]==4){
                    grid[2*i][2*j]='B';
                }
                else if(body[ID(i,j)]!=-1){
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

        for(const Pos& p : env.emptyNeighs(env.head)){
            if(visitTime[ID(p)] == -1){
                tarjan(ID(p));
            }
        }
        

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

class Features{
public:

    TarjanDecomposition full_decomp;
    TarjanDecomposition blocked_decomp;
    vector<int> maxTimes;
    vector<int> freeTimes; // minimum time at which you can be at a cell and escape

    Features(){}

    Features(const Snake& env){

        Snake blocked_head = env;
        blocked_head.body[ID(env.head)] = 4;

        full_decomp = TarjanDecomposition(env);
        blocked_decomp = TarjanDecomposition(blocked_head);

        // env.display();
        // displayArray(env, blocked_decomp.cellComps);
        // cout << blocked_decomp.toString() << '\n';

        // cout << "BCCs\n";
        // for(int i=0; i<blocked_decomp.BCCs.size(); i++){
        //     cout << i << ':';
        //     for(int x : blocked_decomp.BCCs[i]){
        //         cout << x << ' ';
        //     }
        //     cout << '\n';
        // }

        vector<int> cumMaxTimes(area, -1);
        parityCounts = unordered_map<Component, pair<int, int>, ComponentHash>();

        for(const Pos& p : env.emptyNeighs(env.head)){
            visitedComps = unordered_set<Component, ComponentHash>();
            parent = unordered_map<Component, Component, ComponentHash>();
            Component initComp = compOfCell(ID(p), blocked_decomp);
            visitedComps.insert(initComp);
            maxTimes.assign(area, -1);
            maxTimes[ID(p)] = 1;

            // cout << "Hi\n";
            // cout << PosToString(p) << '\n';
            // cout << compToString(compOfCell(ID(p), blocked_decomp)) << '\n';
            getMaxTimes(initComp, ID(p), blocked_decomp);
            
            for(int i=0; i<area; i++){
                cumMaxTimes[i] = max(cumMaxTimes[i], maxTimes[i]);
            }
        }

        visitedComps = unordered_set<Component, ComponentHash>();
        parent = unordered_map<Component, Component, ComponentHash>();
        Component initComp = compOfCell(ID(env.head), full_decomp);
        visitedComps.insert(initComp);
        maxTimes.assign(area, -1);

        // cout << "Yio\n";
        getMaxTimes(initComp, ID(env.head), full_decomp);


        // cout << "Bruh\n";
        // displayArray(env, maxTimes);

        // cout << "Printing stuff\n";
        // for(const auto& [c, p] : parent){
        //     cout << compToString(c) << ' ' << compToString(p) << '\n';
        // }
        
        maxTimes = cumMaxTimes;

        computeFreeTimes(env);
        
    }

private:
    unordered_set<Component, ComponentHash> visitedComps;
    unordered_map<Component, Component, ComponentHash> parent;

    unordered_map<Component, pair<int, int>, ComponentHash> parityCounts;

    void getMaxTimes(Component comp, int start, const TarjanDecomposition& decomp){
        if(comp.type == BCC_TYPE){
            if(!parityCounts.contains(comp)){
                int counts[2] = {0, 0};
                for(const int& x : decomp.BCCs[comp.id]){
                    counts[manhattanDist(fromID(x), fromID(start)) % 2] ++;
                }
                parityCounts[comp] = make_pair(counts[0], counts[1]);
            }
            for(const int& x : decomp.BCCs[comp.id]){
                if(x == start) continue;
                int parity = manhattanDist(fromID(x), fromID(start)) % 2;
                int m;
                if(parity == 0){
                    m = 2 * min(parityCounts[comp].second, parityCounts[comp].first-1);
                }
                else{
                    m = 2 * min(parityCounts[comp].second, parityCounts[comp].first) - 1;
                }
                maxTimes[x] = maxTimes[start] + m;
            }
        }
        if(decomp.compGraph.find(comp) == decomp.compGraph.end()) return;
        for(const Component& neigh : decomp.compGraph.at(comp)){
            if(visitedComps.find(neigh) == visitedComps.end()){
                visitedComps.insert(neigh);
                parent[neigh] = comp;
                getMaxTimes(neigh, neigh.type == AP_TYPE ? -1 : comp.id, decomp);
            }
        }
    }

    Component compOfCell(int cell, const TarjanDecomposition& decomp){
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
                
                Component comp = compOfCell(ID(p), full_decomp);
                if(comp.type == CC_TYPE && freeTimes[ID(p)] == INF){
                    for(const int& x : full_decomp.connectedComps[comp.id]){
                        freeTimes[x] = max(0, t - (int) full_decomp.connectedComps[comp.id].size());
                    }
                }

                
                // Free path of components to head

                if(maxTimes[ID(p)] != -1){
                    Component currComp = compOfCell(ID(p), full_decomp);
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
                        size += full_decomp.BCCs[currComp.id].size() - 1;
                        for(const int& x : full_decomp.BCCs[currComp.id]){
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

const int FREE_POLICY = 0;
const int BORDER_POLICY = 1;

const int LEFT_TURN = 0;
const int RIGHT_TURN = 1;

class Policy{
public:
    int type;
    double hamFactor = -1;
    int turn = -1;

    Policy(int type_, double hamFactor_, int turn_){
        type = type_;
        if(type == FREE_POLICY){
            hamFactor = hamFactor_;
        }
        else if(type == BORDER_POLICY){
            turn = turn_;
        }
        else{
            assert(false);
        }
    }
};

Policy FreePolicy(double hamFactor){
    return Policy(FREE_POLICY, hamFactor, -1);
}

Policy BorderPolicy(int turn){
    return Policy(BORDER_POLICY, -1, turn);
}

class Cycle{
public:
    vector<int> direc;
    int size;

    Cycle(){
        direc.assign(area, -1);
        size = 0;
    }

    string serialize() const{
        string s = "";
        for(int i=0; i<area; i++){
            s += to_string(direc[i]);
        }
        return s;
    }

    bool operator == (const Cycle& other) const{
        return serialize() == other.serialize();
    }

    void fixCycle(){
        size = 0;
        for(int i=0; i<area; i++){
            if(direc[i] != -1){
                size ++;
            }
        }
        if(!verifyCycle()){
            direc.assign(area, -1);
            size = 0;
        }
    }

    bool verifyCycle(){
        int start = -1;
        for(int i=0; i<area; i++){
            if(direc[i] != -1) start = i;
        }
        if(start == -1) return false;
        vector<bool> filled(area, false);

        Pos p = fromID(start);
        for(int i=0; i<area; i++){
            if(direc[ID(p)] == -1) return false;
            filled[ID(p)] = true;
            p = nextCell(p);
            if(p == fromID(start)) break;
        }

        for(int i=0; i<area; i++){
            if(filled[i] != (direc[i] != -1)){
                return false;
            }
        }
        return true;
    }

    void display() const{
        Snake env;
        env.body = direc;
        env.apple = env.head = env.tail = Pos(-1, -1);
        cout << "Cycle size: " << size << '\n';
        cout << env.toCode() << '\n';
        env.display();
        // for(int i=0; i<area; i++){
        //     cout << direc[i] << ' ';
        //     if(i % boardSize == boardSize-1){
        //         cout << '\n';
        //     }
        // }
    }

    void setDir(Pos p, Pos q){
        for(int d=0; d<4; d++){
            if(shiftPos(p, d) == q){
                direc[ID(p)] = d;
                return;
            }
        }
        assert(false);
    }

    Pos nextCell(Pos p) const{
        assert(direc[ID(p)] != -1);
        return shiftPos(p, direc[ID(p)]);
    }

    vector<Cycle> Lmove() const{
        vector<Cycle> ans;
        for(int i=0; i<area; i++){
            if(direc[i] != -1) continue;
            Pos emptyCell = fromID(i);
            for(const Pos& p1 : validNeighs(emptyCell)){
                if(direc[ID(p1)] == -1) continue;

                Pos p2 = nextCell(p1);
                Pos p3 = nextCell(p2);

                if(manhattanDist(p3, emptyCell) == 1){
                    Cycle newCycle = *this;
                    newCycle.setDir(p1, emptyCell);
                    newCycle.setDir(emptyCell, p3);
                    newCycle.direc[ID(p2)] = -1;
                    ans.push_back(newCycle);
                }
            }
        }
        return ans;
    }

    vector<Cycle> retraction() const{
        vector<Cycle> ans;

        for(int i=0; i<area; i++){
            if(direc[i] == -1) continue;
            Pos p1 = fromID(i);
            Pos p2 = nextCell(p1);
            Pos p3 = nextCell(p2);
            Pos p4 = nextCell(p3);
            if(manhattanDist(p1, p4) != 1) continue;
            bool emptyNeigh = false;
            for(const Pos& p : vector<Pos>{p2, p3}){
                for(const Pos& q : validNeighs(p)){
                    if(direc[ID(q)] == -1) emptyNeigh = true;
                }
            }
            if(!emptyNeigh) continue;
            Cycle newCycle = *this;
            newCycle.setDir(p1, p4);
            newCycle.direc[ID(p2)] = -1;
            newCycle.direc[ID(p3)] = -1;
            newCycle.size -= 2;
            ans.push_back(newCycle);
        }

        return ans;
    }

    vector<Cycle> extension() const{
        vector<Cycle> ans;

        for(int i=0; i<area; i++){
            if(direc[i] == -1) continue;
            Pos p1 = fromID(i);
            Pos p2 = nextCell(p1);

            for(const Pos& p : validNeighs(p1)){
                if(p == p2) continue;
                if(direc[ID(p)] != -1) continue;
                for(const Pos& q : validNeighs(p2)){
                    if(q == p1) continue;
                    if(direc[ID(q)] != -1) continue;
                    if(manhattanDist(p, q) != 1) continue;
                    Cycle newCycle = *this;
                    newCycle.setDir(p1, p);
                    newCycle.setDir(p, q);
                    newCycle.setDir(q, p2);
                    newCycle.size += 2;
                    ans.push_back(newCycle);
                }
            }
        }

        return ans;
    }

    Cycle fill();
};

struct CycleHash {
    size_t operator()(const Cycle& c) const {
        return std::hash<string>{}(c.serialize());
    }
};

struct distedCycle{
    int dist;
    Cycle cycle;

    bool operator < (const distedCycle& other) const{
        return dist > other.dist;
    }

    distedCycle(Cycle c){
        cycle = c;
        vector<Pos> emptyCells;
        for(int i=0; i<area; i++){
            if(cycle.direc[i] == -1) emptyCells.push_back(fromID(i));
        }
        dist = 0;
        for(const Pos& p : emptyCells){
            for(const Pos& q : emptyCells){
                dist = max(dist, manhattanDist(p, q));
            }
        }
    }
};

Cycle Cycle::fill(){

    priority_queue<distedCycle> q;

    q.push(distedCycle(*this));

    unordered_set<Cycle, CycleHash> visited;
    visited.insert(*this);

    for(int i=0; i<area*area; i++){
        if(q.size() == 0) break;
        distedCycle top = q.top();
        Cycle cyc = top.cycle;
        int dist = top.dist;
        q.pop();

        // cyc.display();
        // cout << "Hi " << dist << '\n';

        // {
        //     string s;
        //     getline(cin, s);
        // }


        if(cyc.size == area){
            return cyc;
        }

        vector<Cycle> adjCycles;
        for(const Cycle& c : cyc.extension()){
            adjCycles.push_back(c);
        }
        for(const Cycle& c : cyc.Lmove()){
            adjCycles.push_back(c);
        }
        for(const Cycle& c : cyc.retraction()){
            adjCycles.push_back(c);
        }


        for(const Cycle& c : adjCycles){
            if(!visited.contains(c)){
                visited.insert(c);
                // c.display();
                // cout << "New thing\n";

                // {
                //     string s;
                //     getline(cin, s);
                // }
                q.push(distedCycle(c));
            }
        }
    }

    cout << "FATAL: cycle not found\n";

    assert(false);

    return *this;
}



class Solver{
public:
    vector<int> hamCycle;
    vector<int> hamCycle2;

    vector<bool> gridBorder;
    vector<Pos> gridBorderCells;

    int ignoreAppleSafetyThreshold_;
    int maxSafety_;

    vector<int> convertToCycle(vector<int> moves){
        Pos start{0, 0};
        vector<int> ans(area, -1);
        for(int i=0; i<area; i++){
            ans[ID(start)] = moves[i];
            start = shiftPos(start, moves[i]);
        }
        return ans;
    }

    void concat(vector<int>& arr, const vector<int> newNums){
        for(const int& x : newNums){
            arr.push_back(x);
        }
    }

    Solver(){

        gridBorder.assign(area, false);

        for(int i=0; i<area; i++){
            for(int d=0; d<4; d++){
                if(!isValid(shiftPos(fromID(i), d))){
                    gridBorder[i] = true;
                }
            }
            if(gridBorder[i]){
                gridBorderCells.push_back(fromID(i));
            }
        }

        vector<int> moves;
        concat(moves, vector<int>(boardSize-1, 1));
        for(int i=0; i<boardSize/2-1; i++){
            moves.push_back(0);
            concat(moves, vector<int>(boardSize-2, 3));
            moves.push_back(0);
            concat(moves, vector<int>(boardSize-2, 1));
        }
        moves.push_back(0);
        concat(moves, vector<int>(boardSize-1, 3));
        concat(moves, vector<int>(boardSize-1, 2));

        hamCycle = convertToCycle(moves);

        // moves = vector<int>();
        // concat(moves, vector<int>(boardSize-1, 1));
        // concat(moves, vector<int>(boardSize-1, 0));
        // concat(moves, vector<int>(boardSize-1, 3));
        // for(int i=0; i<boardSize/2-1; i++){
        //     moves.push_back(2);
        //     concat(moves, vector<int>(boardSize-2, 1));
        //     moves.push_back(2);
        //     concat(moves, vector<int>(boardSize-2, 3));
        // }
        // moves.push_back(2);

        moves = vector<int>();
        concat(moves, vector<int>(boardSize-1, 0));
        for(int i=0; i<boardSize/2-1; i++){
            concat(moves, vector<int>{1, 2, 1, 0});
        }
        concat(moves, vector<int>{1, 2});
        for(int i=0; i<boardSize/2-2; i++){
            moves.push_back(2);
            concat(moves, vector<int>(boardSize-2, 3));
            moves.push_back(2);
            concat(moves, vector<int>(boardSize-2, 1));
        }
        concat(moves, vector<int>{2, 2});
        for(int i=0; i<boardSize/2-1; i++){
            concat(moves, vector<int>{3, 0, 3, 2});
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
            displayArray(env, features.full_decomp.cellComps);
            cout << features.full_decomp.toString() << '\n';
        }
        

        for(int i=0; i<area; i++){
            if(features.maxTimes[i] != -1){
                minTimes[i] = min(minTimes[i], max(rawMinTimes[i], features.freeTimes[i]));
            }
        }


        return canEscape;

    }


    /*
    Given array of nonnegative integers a_1,...,a_n, we want to compute the expected time f(a_1,...,a_n) for the following:
    1. Choose a random number a_i uniformly
    2. Subtract (a_i + c) from all other numbers, maxing result with zero
    3. Add a_i to the counter
    Repeat until all numbers are zero and return counter.

    Let dp[i] = f(max(0,a_1-i), ..., max(0,a_n-i)). Then,

    dp[i] = (1/n) * [sum_{a_j > i} (a_j + dp[a_j + c]) - #{a_j > i} i + #{a_j <= i} dp[i+c]]
    */
    double computeExpectedFree(vector<int> times, int c){

        int N = times.size();
        sort(times.begin(), times.end());

        int maxT = times[times.size()-1];
        vector<double> dp(maxT+1, 0);
        dp[maxT] = 0;

        int k=N-1;

        double count1 = 0;
        double count2 = 0;
        double count3 = 0;

        for(int i=maxT-1; i>=0; i--){
            while(k>=0 && times[k] > i){
                count1 += times[k];
                count2 += dp[min(times[k] + c, maxT)];
                count3 += 1;
                k --;
            }
            dp[i] = (1.0 / N) * (count1 + count2 - i * count3 + dp[min(i+c, maxT)] * (N-count3));
        }

        return dp[0];

    }



    vector<double> distanceHeuristicPrim(const Snake& env, const Features& features, int safety, double distFactor){ // returns minDist and weighted freeTime heuristic.
        if(env.validMoves().size() == 0) return vector<double>{INF, INF};

        assert(features.maxTimes.size() > 0);

        bool canEscape = getMinTimes(env, features, safety, false);
        if(!canEscape) return vector<double>{INF, INF};

        int immediate_dist = minTimes[ID(env.apple)];

        vector<int> times;
        for(int i=0; i<area; i++){
            if(env.body[i] == -1){
                times.push_back(max(0, (int) (features.freeTimes[i] - immediate_dist*distFactor)));
            }
        }
        return vector<double>{(double) immediate_dist, computeExpectedFree(times, diffusiveTime)};
    }

    vector<double> distanceHeuristic(const Snake& env, const Features& features, int safety, double distFactor=1){
        // env.display();

        // Outer cycle correction
        bool outerCycleCase = true;
        bool hasOuterBlock = false;

        
        vector<bool> isOuterBlock(area, false);

        for(const Pos& p : gridBorderCells){
            if(env.body[ID(p)] != -1){
                outerCycleCase = false;
                break;
            }
            for(int a=-1; a<=1; a++){
                for(int b=-1; b<=1; b++){
                    Pos q{p.x + a, p.y + b};
                    if(isValid(q) && !gridBorder[ID(q)] && env.body[ID(q)] != -1){
                        isOuterBlock[ID(p)] = true;
                        hasOuterBlock = true;
                    }
                }
            }
        }

        // displayArray(env, vector<int>(isBorder.begin(), isBorder.end()));
        // cout << '\n';
        // displayArray(env, vector<int>(isOuterBlock.begin(), isOuterBlock.end()));
        // cout << '\n';

        if(!hasOuterBlock) outerCycleCase = false;
        if(!outerCycleCase) return distanceHeuristicPrim(env, features, safety, distFactor);

        queue<int> q;
        q.push(ID(env.head));

        vector<bool> visited(area, false);
        visited[ID(env.head)] = true;

        vector<int> blocks;

        while(q.size() > 0){
            int top = q.front();
            q.pop();

            if(isOuterBlock[top] && top != ID(env.head)){
                blocks.push_back(top);
                continue;
            }

            for(const Pos& p : validNeighs(fromID(top))){
                if(env.body[ID(p)] != -1) continue;
                if(!visited[ID(p)]){
                    visited[ID(p)] = true;
                    q.push(ID(p));
                }
            }
        }

        assert(blocks.size() <= 2);

        vector<Snake> envs_to_check;

        for(const int& block : blocks){
            Snake blocked_env = env;
            blocked_env.body[block] = 4;
            envs_to_check.push_back(blocked_env);
        }

        if(blocks.size() == 0){
            envs_to_check.push_back(env);
        }

        vector<double> best_heuris{INF, INF};
        double minDist = INF;

        for(const Snake& e : envs_to_check){
            // e.display();
            vector<double> heuris = distanceHeuristicPrim(e, Features(e), safety, distFactor);

            if(minDist > heuris[0]){
                minDist = heuris[0];
                best_heuris = heuris;
            }
        }

        return best_heuris;
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
    
    Problem: an objective "border-following" move is ambiguous. Consider: is moving right a border-following move?
 
    . . . .
    . . H .
    x x x .
    x x . .

    It is if you are following the right-side border. So we need a notion of always turning left or always turning right on the border.

    New border-following rule:
    
    - Compute the border by following the right border around in a loop until you get back to where you started (assuming static body)
    - If current Manhattan dist to apple <= 1 + min Manhattan dist among border, use freePolicy
    - Otherwise, check valid moves as in freePolicy, giving a bonus to the move that follows the directional border.
    
    */


    int borderFollowingMove(const Snake& env, Pos p, int turn, int d){
        // Given you're at position p facing in direction d, what action is the border-following move under border-turn?

        for(int i=0; i<4; i++){
            int a = (d + (1-i) * (turn == RIGHT_TURN ? 1 : -1) + 4) % 4;
            Pos newPos = shiftPos(p, a);
            if(isValid(newPos) && env.body[ID(newPos)] == -1) return a;
        }
        assert(false);
    }

    int getLastMove(const Snake& env){
        for(int i=0; i<4; i++){
            Pos neigh = shiftPos(env.head, i);
            if(isValid(neigh) && env.body[ID(neigh)] != -1 && shiftPos(neigh, env.body[ID(neigh)]) == env.head){
                return env.body[ID(neigh)];
            }
        }
        assert(false);
    }

    int minBorderDist(const Snake& env){

        Pos currPos = env.head;
        int currDir = getLastMove(env);
        int minDist = INF;

        for(int i=0; i<area; i++){
            int d = borderFollowingMove(env, currPos, RIGHT_TURN, currDir);
            currPos = shiftPos(currPos, d);
            currDir = d;
            minDist = min(minDist, manhattanDist(currPos, env.apple));
            if(currPos == env.head) return minDist;
        }

        assert(false);
    }

    int borderPolicy(const Snake& env, int safety, int turn, bool printMode=false){
        // // Check if head and apple are in same component
        // TarjanDecomposition decomp(env);
        // bool sameComp = false;
        // for(const auto& comp : decomp.BCCs){
        //     if(comp.contains(ID(env.head)) && comp.contains(ID(env.apple))){
        //         sameComp = true;
        //     }
        // }
        // if(!sameComp) return freePolicy(env, safety);

        int borderDist = minBorderDist(env);


        if(manhattanDist(env.head, env.apple) <= 1 + borderDist){
            return freePolicy(env, safety, 0, hamCycle);
        }

        double minDist = INF;
        int bestAction = -1;

        int borderMove = borderFollowingMove(env, env.head, turn, getLastMove(env));
        for(const int& a : env.validMoves()){
            Snake newEnv = env;
            newEnv.move(a);

            vector<double> heuristic = distanceHeuristic(newEnv, neighFeatures[a], safety);

            double candDist = (a != borderMove) * borderBonus + heuristic[1];

            if(printMode){
                cout << "Action " << a << " dist: " << (a == borderMove) << ' ' << heuristic[1] << '\n';
            }
            if(minDist > candDist ){
                minDist = candDist;
                bestAction = a;
            }
        }
        return bestAction;
    }

    int freePolicy(const Snake& env, int safety, double hamFactor, vector<int> cycle, bool printMode=false){

        // whichever ham cycle it currently shares the most segments with, follow that one

        // int maxCount = -1;
        // vector<int> bestCycle;
        // for(const auto& cycle : vector<vector<int>>{hamCycle, hamCycle2}){
        //     int counter = 0;
        //     for(int i=0; i<area; i++){
        //         if(env.body[i] == cycle[i]) counter ++;
        //     }
        //     if(maxCount < counter){
        //         maxCount = counter;
        //         bestCycle = cycle;
        //     }
        // }

        double minDist = INF;
        int bestAction = -1;

        
        for(const int& a : env.validMoves()){
            Snake newEnv = env;
            newEnv.move(a);
            vector<double> heuristic = distanceHeuristic(newEnv, neighFeatures[a], safety, hamFactor == 0);
            // If safety <= ignoreAppleSafetyThreshold, ignore apple distance.
            double candDist = heuristic[0] * (1-hamFactor) * (safety > ignoreAppleSafetyThreshold_) + (a != cycle[ID(env.head)]) * (hamBonus * hamFactor) + heuristic[1];
            if(printMode){
                cout << "Action " << a << " immediate dist: " << heuristic[0] << " freeTime: " << heuristic[1] << " Is not Ham: " << (a != cycle[ID(env.head)]) << '\n';
            }
            if(minDist > candDist){
                minDist = candDist;
                bestAction = a;
            }
        }
        if(printMode){
            cout << "Best action: " << bestAction << '\n';
        }
        return bestAction;
    }

    Features neighFeatures[4];

    vector<int> shortestPath(const Snake& env, Policy policy, vector<int> cycle, bool printMode=false, int maxTime=area, bool continuePath=false){

        vector<int> moves;
        Snake curr_env = env;

        for(int i=0; i<maxTime; i++){
            if(printMode){
                cout << "Curr Env:\n";
                cout << curr_env.toCode() << '\n';
                curr_env.display();
            }

            for(const int& a : curr_env.validMoves()){
                Snake newEnv = curr_env;
                newEnv.move(a);
                neighFeatures[a] = Features(newEnv);
            }


            int action = -1;
            for(int safety=maxSafety_; safety>=0; safety--){
                if(printMode){
                    cout << "Checking safety " << safety << '\n';
                }
                if(policy.type == FREE_POLICY){
                    action = freePolicy(curr_env, safety, policy.hamFactor, cycle, printMode);
                }
                else if(policy.type == BORDER_POLICY){
                    action = borderPolicy(curr_env, safety, policy.turn, printMode);
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
            if((curr_env.head == curr_env.apple && !continuePath) || curr_env.validMoves().size() == 0){
                break;
            }
        }
        return moves;
    }

    Cycle cycleFromPath(const Snake& env, vector<int> path){
        Cycle c;
        Pos currPos = env.head;
        // env.display();


        for(int i=0; i<path.size(); i++){
            // cout << PosToString(currPos) << ' ';
            c.direc[ID(currPos)] = path[i];
            currPos = shiftPos(currPos, path[i]);
            if(env.body[ID(currPos)] != -1){
                break;
            }
        }

        for(int i=0; i<area; i++){
            c.direc[ID(currPos)] = env.body[ID(currPos)];
            currPos = shiftPos(currPos, env.body[ID(currPos)]);
            if(currPos == env.head) break;
        }

        // cout << '\n';
        c.fixCycle();
        return c;
    }

    vector<int> getCycle(const Snake& env, bool printMode){
        Cycle cycle1 = cycleFromPath(env, shortestPath(env, FreePolicy(1), hamCycle, false, area, true));
        Cycle cycle2 = cycleFromPath(env, shortestPath(env, FreePolicy(1), hamCycle2, false, area, true));
        Cycle longestCycle = (cycle1.size > cycle2.size) ? cycle1 : cycle2;

        if(printMode){
            cout << "Current cycle:\n";
            longestCycle.display();
        }
        

        Cycle filled = longestCycle.fill();

        if(printMode){
            cout << "Filled cycle:\n";
            filled.display();
        }


        return filled.direc;
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

        {
            ofstream fout(logFile);
            fout.close();
        }
        

        vector<vector<int>> my_cycles{hamCycle, hamCycle2};
        int currCycleID = 0;
        vector<int> currCycle = hamCycle;

        bool forcedCycle = false;

        maxSafety_ = maxSafety;
        ignoreAppleSafetyThreshold_ = ignoreAppleSafetyThreshold;

        for(int i=0; i<area*2; i++){
            // cout << env.toCode() << '\n';

            if(env.getSize() >= endGameCutoff){
                maxSafety_ = endGameSafety;
                ignoreAppleSafetyThreshold_ = 0;
            }

            if(env.getSize() >= forceCycleCutoff && !forcedCycle){
                if(printMode >= 1){
                    cout << "Forcing Ham cycle\n";
                }
                currCycle = getCycle(env, printMode >= 1);
                forcedCycle = true;
            }
            

            vector<vector<int>> paths;
            if(env.getSize() < endGameCutoff){
                vector<int> free_path = shortestPath(env, FreePolicy(0), currCycle, printMode>=2);
                vector<int> right_border_path = shortestPath(env, BorderPolicy(RIGHT_TURN), currCycle, printMode>=2, free_path.size() * 1.5);
                vector<int> left_border_path = shortestPath(env, BorderPolicy(LEFT_TURN), currCycle, printMode>=2, free_path.size() * 1.5);
                paths = vector<vector<int>>{free_path, right_border_path, left_border_path};
            }
            else{
                double hamFactor = min(1., decayRate * (env.getSize() - endGameCutoff));
                vector<int> endgame_path = shortestPath(env, FreePolicy(hamFactor), currCycle, printMode>=2);
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

            for(int safety=maxSafety_; safety>=0; safety--){
                for(int j=0; j<paths.size(); j++){
                    if(envs[j].head != envs[j].apple){
                        if(printMode >= 1) cout << "Path " << j << " didn't reach apple.\n";
                        continue;
                    }
                    vector<double> heuristic = distanceHeuristic(envs[j], Features(envs[j]), safety);

                    // Invert distance metric if safety <= ignoreAppleSafetyThreshold to prefer longer paths
                    double candDist = (double) paths[j].size() * (safety > ignoreAppleSafetyThreshold_ ? 1 : -1) + heuristic[1];
                    if(printMode >= 1){
                        cout << "Safety " << safety << " Path " << j << " Dist: " << candDist << '\n';
                    }
                    if(minDist > candDist){
                        minDist = candDist;
                        bestPath = j;
                    }
                }
                if(bestPath != -1 && safety > ignoreAppleSafetyThreshold_){
                    break;
                }
            }

            if(bestPath == -1) bestPath = 0;
            vector<int> path = paths[bestPath];
            Snake newEnv = envs[bestPath];

            if(printMode >= 1){
                cout << "Using path " << bestPath << '\n';
            }

            // Check to see if apple was eaten. If not, use Hamiltonian path

            if(newEnv.head != newEnv.apple){
                if(printMode >= 1){
                    cout << "Got stuck, using Ham cycle instead.\n";
                }
                path = shortestPath(env, FreePolicy(1), currCycle, printMode>=2, 2*area);

                newEnv = env;
                for(int d : path){
                    newEnv.move(d);
                }
            }

            // If still not, switch to a different ham cycle

            if(newEnv.head != newEnv.apple){
                if(printMode >= 1){
                    cout << "Got stuck, switching to other cycle.\n";
                }
                currCycleID = 1 - currCycleID;
                currCycle = my_cycles[currCycleID];
                path = shortestPath(env, FreePolicy(1), currCycle, printMode>=2, 2*area);

                newEnv = env;
                for(int d : path){
                    newEnv.move(d);
                }
            }

            // If still not, use cycle finding

            if(newEnv.head != newEnv.apple){
                if(printMode >= 1){
                    cout << "Got stuck, calling cycle finding.\n";
                }
                currCycle = getCycle(env, printMode >= 1);
                forcedCycle = true;
                path = shortestPath(env, FreePolicy(1), currCycle, printMode>=2, 2*area);

                newEnv = env;
                for(int d : path){
                    newEnv.move(d);
                }
            }

            
            
            newEnv = env;
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
                ofstream fout(logFile, ios::app);
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

    void log(string mainFile, string s){
        ofstream fout(mainFile, ios::app);
        cout << s;
        fout << s;
    }

    void runTrials(int numTrials, string outFile, string codeFile, string mainFile, bool logGames=false){
        ofstream fout(outFile);
        ofstream codeOut(codeFile);
        {
            ofstream mainOut(mainFile);
            mainOut.close();
        }

        int totalSize = 0;
        int totalTime = 0;
        int numWins = 0;
        int winTime = 0;

        unsigned long start_time = time(0);

        log(mainFile, "Running " + to_string(numTrials) + " trials...\n");

        for(int i=0; i<numTrials; i++){
            int endCause = simulate(Snake(), 0, INF, logGames ? "state_logs/game_" + to_string(i) + ".txt" : "");

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
            log(mainFile, "Game " + to_string(i) + " size: " + to_string(size) + " time: " + to_string(time) + " termination: " + to_string(endCause) + '\n');
        }
        log(mainFile, "Average size: " + to_string((double) totalSize / numTrials) + '\n');
        log(mainFile, "Average time: " + to_string((double) totalTime / numTrials) + '\n');
        log(mainFile, "Win rate: " + to_string((double) numWins / numTrials) + '\n');
        log(mainFile, "Average win time: " + to_string((double) winTime / numWins) + '\n');

        log(mainFile, "Ellapsed time: " + to_string(time(0) - start_time) + '\n');
    }


};

void runSavedEndGames(int numGames, int cutoff){
    cout << "Running endgames only\n";
    Solver solver;
    for(int i=0; i<numGames; i++){
        ifstream fin("state_logs/game_" + to_string(i) + ".txt");
        string s;
        for(int j=0; j<cutoff; j++){
            fin >> s;
        }
        Snake env(s);
        solver.simulate(env, 0, INF);
        if(solver.lastState.getSize() < area){
            cout << solver.lastState.toCode() << '\n';
        }
        cout << "Apples: " << solver.appleTimes.size() << '\n';
    }
}

int main(){
    srand(95436);

    Solver s;

    // cout << s.computeExpectedFree(vector<int>{0, 0, 0, 5, 10}, 5) << '\n';

    // Snake env;

    // s.displayArray(env, s.hamCycle);
    // cout << '\n';
    // s.displayArray(env, s.hamCycle2);
    // cout << '\n';

    // Snake env("9x5_7x1_3x9_524_gghaawsrhavhwwawswwaxgvwaaunsaaukaabwpaaanhaaaguaa");
    // env.move(0);
    // env.move(0);
    // env.move(0);
    // env.move(3);



    // Snake env("26x21_13x27_11x27_22607_nsssssspaaaaaaalgghggguaaaaaaaonscuaaaaaaaaaaoljcuaaaaaaaaaaoonsuaaaaaaaaaaoolguaaaaaaaaaaooospaaaaaaaaaaooghuaaaaacssspoosrxpaaaanaaauokhvhwsssspaaauolvucwaaaaaaaauoossswaaaaaaaauolgggwaaaaaaaauoonsssaaaaaaabuookaaaaaaaaaaaaookaaaaaaaaaaaaoogggggghggggghoossssssrwssssrogggggghwwaaaawonssrsnrwwcrrpwokaaxehwwwcxxuwjkaabonwwwbhguwnhaaeopxwwcsxpwosagjgggwwbkauwkaawssssswcpauwkaavgggggwbkauwkaaxssssssapbuwkaaaaaaaaaaaeawkaaaaaaaaaaaebwggggggggggggjes");
    // env.move(1);

    Snake env("13x1_8x22_16x18_41920_nsrssrsssnnsnnsghvghwhgotqjoqonswrsvvuhgjnqookaxvhwssrwsljookaaxsxghxwjontjkaaaaaxrgwxohjnknssspawwqoonwtklgghwpwwxtthwjkonrswuvvghjnwxkohwhxuxssscqvokonwvguaaaabonokohwxsssssssttokonvgggghhghgjokoqwrrrrqvwrwsoogosxxxxuacwwjoosggggghuacxwxolosssssnuabhwjoogglggoluaanxxoossjcsojaaaggooggorsjoaaaanstonshwhxopaaahgjolorwvjhuaanqwsoohwwwsruanqwqooonwwxhwualorxtoolwvhwwuaolvgoooocwrwwuaojnnoooonxwvwxntstttoojggwwsjlggggjoossssxhxtssssstggggggvgggggggj");
    // env.move(0);


    // env.move(1);
    // env.move(0);
    // env.move(0);
    // env.move(0);

    cout << s.simulate(env, 1, 0) << "\n";

    // s.endgamePolicy(env, true);


    // cout << s.minBorderDist(env) << '\n';

    // s.maxSafety_ = maxSafety;
    // s.ignoreAppleSafetyThreshold_ = ignoreAppleSafetyThreshold;
    // s.shortestPath(env, FreePolicy(0), s.hamCycle, true);
    // s.computeBorder(env);

    // s.displayArray(env, env.body, true);

    // TarjanDecomposition tj_decomp(env);
    // cout << tj_decomp.toString() << '\n';


    // env.display();
    // s.getMinTimes(env, Features(env), 0, true);
    // cout << "Min times:\n";
    // displayArray(env, s.minTimes);
    // vector<double> heuristic = s.distanceHeuristic(env, Features(env), 0);
    // cout << heuristic[0] << ' ' << heuristic[1] << '\n';

    // s.displayArray(env, s.maxTimes);

    // s.runTrials(100, "score.out", "code.out", "log.out", true);

    // runSavedEndGames(50, 700);
}