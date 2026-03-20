
/*
g++ -O2 -std=c++20 -fsanitize=address -fsanitize=undefined -fno-sanitize-recover=all -fsanitize=float-divide-by-zero -fsanitize=float-cast-overflow -fno-sanitize=null -fno-sanitize=alignment snake.cpp && ./a.out

-fsanitize=address -fsanitize=undefined -fno-sanitize-recover=all -fsanitize=float-divide-by-zero -fsanitize=float-cast-overflow -fno-sanitize=null -fno-sanitize=alignment
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

const int boardSize = 30;
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
        body[ID(head)] = 4;
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
        body[ID(head)] = 4;
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
        body[ID(head)] = 4;

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
        vector<int> body_ = body;
        body_[ID(head)] = -1;
        for(int i=0; i<area/2; i++){
            s += (char) (97 + (body_[2*i]+1)*5 + (body_[2*i+1]+1));
        }
        return s;
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
        cout<<"Size: "<<count_if(body.begin(),body.end(),[](int v){return v!=-1;})+1<<"\n";

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

        for(const Pos& neigh : env.emptyNeighs(env.head)){
            if(visitTime[ID(neigh)] == -1){
                assert(edgeQueue.size() == 0);
                tarjan(ID(neigh));
            }
        }

        cellComps.assign(area, -1);
        getGraph();
    }

    vector<int> visitTime;
    vector<int> minConnTime;
    vector<bool> isAP;
    int tarjan_timer;

    
    vector<Edge> edgeQueue;
    vector<unordered_set<int>> BCCs;

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
        // cout << "Calling " << env.PosToCode(fromID(node)) << '\n';
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
            else if(visitTime[ID(neigh)] < visitTime[node]){ // only process back-edges, not forward
                minConnTime[node] = min(minConnTime[node], visitTime[ID(neigh)]);
            }
            else{
                edgeQueue.pop_back(); // forward cross-edge, don't keep it
            }
        }
        if(parent == -1 && n_children > 1){
            isAP[node] = true;
        }
        if(parent == -1 && n_children == 0){
            BCCs.push_back(unordered_set<int>{node});
        }
    }

    unordered_map<Component, unordered_set<Component, ComponentHash>, ComponentHash> compGraph;
    vector<int> cellComps;

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
    To get the distance to apple:

    Step 1: compute max time snake could be at each point, assuming tail no longer retracted

    - Perform Tarjan's to get AP decomposition
    - For each biconnected component, get the number of cells in the path of BCCs connecting head to component, including neighboring APs

    Step 2: Perform dynamic BFS where tail cells are freed at time t if it has a visited neighbor whose max time is > t


    Next part:

    Get distance to apple where we don't die


    */


    TarjanDecomposition decomp;
    vector<int> maxTimes;

    unordered_set<Component, ComponentHash> visitedComps;

    // unordered_set<int> cellsOnPath;

    void getMaxTimes(Component comp, int start){
        // cout << "Calling " << compToString(comp) << '\n';
        // unordered_set<int> addedCells;
        if(comp.type == BCC_TYPE){
            int parityCounts[2] = {0, 0};
            for(const int& x : decomp.BCCs[comp.id]){
                // if(cellsOnPath.find(x) == cellsOnPath.end()){
                //     cellsOnPath.insert(x);
                //     addedCells.insert(x);
                // }
                parityCounts[manhattanDist(fromID(x), fromID(start)) % 2] ++;
            }
            // cout << parityCounts[0] << ' ' << parityCounts[1] << '\n';
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
                // int m = cellsOnPath.size();
                // if((m - manhattanDist(decomp.env.head, fromID(x))) % 2 == 1){
                //     m --;
                // }
                // maxTimes[x] = max(maxTimes[x], m);
            }
        }
        for(const Component& neigh : decomp.compGraph[comp]){
            if(visitedComps.find(neigh) == visitedComps.end()){
                visitedComps.insert(neigh);
                getMaxTimes(neigh, neigh.id == AP_TYPE ? -1 : comp.id);
            }
        }
        // if(comp.type == BCC_TYPE){
        //     for(const int& x : addedCells){
        //         cellsOnPath.erase(x);
        //     }
        // }
    }

    // void printMaxTimes(const Snake& env){
    //     for(int i=0; i<area; i++){
    //         if(env.body[i] != -1) cout << "  x";
    //         else if(maxTimes[i] == -1) cout << "  .";
    //         else cout << string(3 - to_string(maxTimes[i]).size(), ' ') << maxTimes[i];
    //         if((i+1) % boardSize == 0) cout << '\n';
    //     }
    // }

    // void printCellComp(const Snake& env){
    //     for(int i=0; i<area; i++){
    //         int x = decomp.cellComps[i];
    //         if(env.body[i] != -1) cout << "  x";
    //         else if(x == -1) cout << "  .";
    //         else cout << string(3 - to_string(x).size(), ' ') << x;
    //         if((i+1) % boardSize == 0) cout << '\n';
    //     }
    // }

    void displayArray(const Snake& env, const vector<int>& arr, bool bodyDir=false){
        for(int i=0; i<area; i++){
            if(env.body[i] != -1){
                if(bodyDir) cout << "  " << env.body[i];
                else cout << "  x";
            }
            else if(arr[i] == -1) cout << "  .";
            else cout << string(3 - to_string(arr[i]).size(), ' ') << arr[i];
            if((i+1) % boardSize == 0) cout << '\n';
        }
    }
    

    int distToApple(const Snake& env, int safety){ // estimate for distance to apple
        if(env.validMoves().size() == 0) return INF;
        // cout << "Calling...\n";

        // Get max times

        decomp = TarjanDecomposition(env);
        // cout << "APs: ";
        // for(int i=0; i<area; i++){
        //     if(decomp.isAP[i]) cout << env.PosToCode(fromID(i)) << ' ';
        // }
        // cout << '\n';
        

        vector<int> cumMaxTime(area, -1);

        for(const Pos& p : env.emptyNeighs(env.head)){
            visitedComps = unordered_set<Component, ComponentHash>();
            Component initComp;
            if(decomp.isAP[ID(p)]){
                initComp = Component{AP_TYPE, ID(p)};
            }
            else{
                initComp = Component{BCC_TYPE, decomp.cellComps[ID(p)]};
            }
            visitedComps.insert(initComp);
            // assert(cellsOnPath.size() == 0);
            maxTimes.assign(area, -1);
            maxTimes[ID(p)] = 1;
            getMaxTimes(initComp, ID(p));
            for(int i=0; i<area; i++){
                cumMaxTime[i] = max(cumMaxTime[i], maxTimes[i]);
            }
            // displayArray(env, maxTimes);
        }

        maxTimes = cumMaxTime;

        // displayArray(env, maxTimes);
        // displayArray(env, decomp.cellComps);
        // for(int i=0; i<decomp.BCCs.size(); i++){
        //     vector<int> things(area, -1);
        //     for(int j=0; j<area; j++){
        //         if(decomp.BCCs[i].contains(j)){
        //             things[j] = i;
        //         }
        //     }
        //     displayArray(env, things);
        // }

        

        maxTimes[ID(env.head)] = 0;

        // Get visibility of tail and apple

        bool tail_vis = false;
        bool apple_vis = false;
        for(const auto& bcc : decomp.BCCs){
            for(const int& x : bcc){
                if(x == ID(env.apple)) apple_vis = true;
                for(const Pos& p : validNeighs(fromID(x))){
                    if(p == env.tail) tail_vis = true;
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
        unordered_set<Pos, PosHash> queue{env.head};
        visited[ID(env.head)] = true;

        Pos curr_tail = env.tail;

        int shortestDist = INF;
        bool canEscape = false;

        for(int t=0; t<area; t++){
            if(queue.find(env.apple) != queue.end()){
                shortestDist = min(shortestDist, t);
            }
            // if(queue.size() > 0){
            //     cout << t << ": ";
            //     for(const Pos& p : queue){
            //         cout << env.PosToCode(p) << ' ';
            //     }
            //     cout << '\n';
            // }
            
            unordered_set<Pos, PosHash> next_queue;
            for(const Pos& p : queue){
                if((t - manhattanDist(p, env.head)) % 2 == 0){
                    for(const Pos& q : validNeighs(p)){
                        if(!retractibleBody[ID(q)] && !visited[ID(q)]){
                            visited[ID(q)] = true;
                            next_queue.insert(q);
                        }
                    }
                }
                else{ // for off-parity insertions in the retraction
                    next_queue.insert(p);
                }
            }
            for(const Pos& p : validNeighs(curr_tail)){
                if(!retractibleBody[ID(p)] && visited[ID(p)] && maxTimes[ID(p)] > t){
                    // cout << "Step " << t << " Inserting " + env.PosToCode(p) << '\n';
                    next_queue.insert(p);
                }
                if(maxTimes[ID(p)] > t){
                    // cout << "Step " << t << " Freed " + env.PosToCode(p) << '\n';
                    canEscape = true;
                }
            }
            // Safety measure: assume a snake size 2 larger if tail is not visible and apple is.
            if(t >= safety || tail_vis || (!apple_vis)){
                retractibleBody[ID(curr_tail)] = false;
                if(curr_tail != env.head){
                    curr_tail = shiftPos(curr_tail, env.body[ID(curr_tail)]);
                }
            }
            
            queue = next_queue;
        }

        if(!canEscape){
            return INF;
        }

        return shortestDist;

        // return manhattanDist(env.head, env.apple);
        
    }

    vector<int> shortestPath(const Snake& env){
        int maxTime = 1000;

        vector<int> moves;
        Snake curr_env = env;

        for(int i=0; i<maxTime; i++){
            // cout << curr_env.toCode() << '\n';
            // curr_env.display();
            // cout << "Step " << i << '\n';
            // TarjanDecomposition tj_decomp(curr_env);
            // cout << tj_decomp.toString() << '\n';

            int minDist = INF;
            int bestAction = -1;
            // cout << "Finding dists...\n";
            for(int safety=2; safety>=0; safety--){
                for(int d : curr_env.validMoves()){
                    Snake newEnv = curr_env;
                    newEnv.move(d);
                    // cout << "Checking action " << d << '\n';
                    int dist = distToApple(newEnv, safety);
                    // cout << "Action " << d << " dist: " << dist << '\n';
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
                break;
            }
            curr_env.move(bestAction);
            moves.push_back(bestAction);
            if(curr_env.head == curr_env.apple || curr_env.validMoves().size() == 0){
                break;
            }
        }
        return moves;
    }


    void simulate(Snake env){
        string s;

        int num_repeats = 0;
        for(int i=0; i<area; i++){
            // env.display();
            vector<int> path = shortestPath(env);
            TarjanDecomposition tj_decomp(env);
            Snake newEnv = env;
            vector<int> headLoc;
            for(int d : path){
                newEnv.move(d);
                headLoc.push_back(ID(newEnv.head));
            }
            cout << tj_decomp.toString();
            cout << env.toCode() << '\n';
            env.display(path, headLoc);
            env = newEnv;
            if(env.validMoves().size() == 0 || path.size() == 0) break;
            if(env.head == env.apple){
                env.randomizeApple();
                num_repeats = 0;
            }
            else{
                num_repeats++;
            }

            // if(i >= 195)
                // getline(cin, s);
            
            if(s.size() > 0) break;
            if(num_repeats >= 3) break;
        }
    }
};




int main(){
    srand(42);

    Solver s;

    Snake env;

    // Snake env("1x19_27x0_11x9_12556_aaaaaaahggkaaaagggggggvuakaaaaxssssssspekacssaaaaaaaaujkacgjaaaaaaaauxkacuaaaaaaaaaujkacuaaaaaaaaauxkacuaaaaaaaaaujkacuaaaaaaaaauxkacuaaaaaaaaaujghcuaaaaaaaaauwrrcuaaaaaaaaauxxwcuaaaaaaaaauggwcuaaaaaaaaauxsrcuaaaaaaaaauggwcuaaaaaaaaauxsrcuaaaaaaaaauggwcuaaaaaaaaauxsscuaaaaagggguaaacuaaaaaxsssssssruaaaaaaaaaaaggwxpaaaaaaaaaaxsqhuaaaaggggggggucuaaaaxsssssssssuaaaaaaaaaaaaaauaaaaaaaaaaaaaauaaaaaaaaaaaaaaukaaaaaaaaaggggukaaaaaaaaaxssssggggggggggggggj");
    // env.move(2);

    // Snake env("19x10_19x6_0x7_12661_aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaansssssssaaaaaaalggggggjaaaaaaaosssssssaaaaaaalggggggjaaaaaaaosssssssaaaaaaahggggggoaaaaaaacuaaaaaoaaaaaaanuaaaaaoaaaaaaahxspaaaoaaaaaaabghuaaaoaaaaaaaaacuaaaocssssssspcuaaajcgggggghucuaaaacxssssabucuaaaabggggoaaacuaaaaaaaaaoaaacxsaaaaaaaaoaaabgosssaacpaesaaaagggoaanaaaesssssaaoaakaaaaaaaaeaaoaakaaaaaaaaeaaoaaghaaaaaaaeaaoaaacaaaaaaaeaaoaaacaaaaaaaeaaoaaacaagggggjaaoaaacaaxssssssstaaabggggggggggjaaaaaaaaaaaaaaa");
    // Snake env("0x12_22x29_10x1_12704_aaagggzaaaaaaaaaaaxaaaaaaaaaaaaaaeaaansssssssaaaeaaalggggggjaaaeaaaosssssssaaaeaaalggggggjaaaeaaaosssssssaaaeaaahggggggoaaaeaaacuaaaaaoaaaeaaanuaaaaaoaaaeaaahxspaaaoaaaeaaabghuaaaogggjaaaaacuaaaowssssssspcuaaajwgggggghucuaaaawxssssabucuaaaavggggoaaacuaaaauaaaaoaaacxsaaauaaaaoaaabgosssxsssstsaaaagggoaaaaaaesssssaaoaaaaaaaaaaaeaaoaaaaaaaaaaaeaaoaaaaaaaaaaaeaakaaaaaaaaaaaeaakaaaaaaaaaaaeaakaaaaaagggggjaakaaaaaaxssssssspaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");
    // env.move(0);
    // env.move(0);

    // env.display();

    // s.displayArray(env, env.body, true);

    // TarjanDecomposition tj_decomp(env);
    // cout << tj_decomp.toString() << '\n';

    // cout << s.distToApple(env, 0) << '\n';

    // s.displayArray(env, s.maxTimes);

    s.simulate(env);
}