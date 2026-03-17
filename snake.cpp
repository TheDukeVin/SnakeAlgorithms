
/*
g++ -O2 -std=c++17 -fsanitize=address snake.cpp && ./a.out
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
        for(int i=0; i<area/2; i++){
            s += (char) (97 + (body[2*i]+1)*5 + (body[2*i+1]+1));
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
            grid[2*p.x][2*p.y] = 'x';
            grid[2*p.x - dir[d][0]][2*p.y - dir[d][1]] = (d%2==0 ? 'h' : 'v');
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
                else if(c=='h') cout<<"\033[31m-\033[0m"; // red horizontal path
                else if(c=='v') cout<<"\033[31m|\033[0m"; // red vertical path
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
};

bool const operator == (Component a, Component b){
    return a.type == b.type && a.id == b.id;
}

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

string compToString(Component comp){
    return (comp.type == AP_TYPE ? 'A' : 'C') + to_string(comp.id);
}

// template <class T>
// inline void hash_combine(std::size_t & s, const T & v)
// {
//   std::hash<T> h;
//   s^= h(v) + 0x9e3779b9 + (s<< 6) + (s>> 2);
// }

// template <class T>
// class MyHash;
 
// template<>
// struct MyHash<Component>
// {
//     std::size_t operator()(Component const& s) const 
//     {
//         std::size_t res = 0;
//         hash_combine(res,s.type);
//         hash_combine(res,s.id);
//         return res;
//     }
// };


class TarjanDecomposition{
public:
    Snake env;

    TarjanDecomposition(Snake env_){
        // cout << "Running tarjan...\n";
        env = env_;
        visitTime.assign(area, -1);
        minConnTime.assign(area, -1);
        isAP.assign(area, false);
        tarjan_timer = 0;
        tarjan(ID(env.head));

        // for(int i=0; i<area; i++){
        //     if(isAP[i]) cout << i << '\n';
        // }
        
        // cout << "Running decomposition...\n";
        cellComps.assign(area, Component{-1, -1});
        visited.assign(area, false);
        component_counter = 0;
        visited[ID(env.head)] = true;
        getDecomposition(ID(env.head));
    }

    vector<int> visitTime;
    vector<int> minConnTime;
    vector<bool> isAP;
    int tarjan_timer;

    

    void tarjan(int node, int parent=-1){
        visitTime[node] = tarjan_timer;
        minConnTime[node] = tarjan_timer;
        tarjan_timer ++;
        int n_children = 0;
        for(Pos neigh : env.emptyNeighs(fromID(node))){
            if(ID(neigh) == parent) continue;
            if(visitTime[ID(neigh)] == -1){
                tarjan(ID(neigh), node);
                n_children += 1;
                if(parent != -1 && visitTime[node] <= minConnTime[ID(neigh)]){
                    isAP[node] = true;
                }
                minConnTime[node] = min(minConnTime[node], minConnTime[ID(neigh)]);
            }
            else{
                minConnTime[node] = min(minConnTime[node], visitTime[ID(neigh)]);
            }
        }
        if(parent == -1 && n_children > 1){
            isAP[node] = true;
        }
    }

    unordered_map<Component, unordered_set<Component, ComponentHash>, ComponentHash> compGraph;
    unordered_map<Component, int, ComponentHash> compSizes;
    unordered_map<Component, Component, ComponentHash> parentComp;
    vector<Component> cellComps;
    vector<bool> visited;
    int component_counter;

    vector<int> adjNodes;
    int compSize;

    void fillComponent(int node, Component comp){ // get component size and neighboring APs
        cellComps[node] = comp;
        compSize ++;
        for(Pos neigh : env.emptyNeighs(fromID(node))){
            if(isAP[ID(neigh)]){
                adjNodes.push_back(ID(neigh));
                continue;
            }
            if(!visited[ID(neigh)]){
                visited[ID(neigh)] = true;
                fillComponent(ID(neigh), comp);
            }
        }
    }

    void getDecomposition(int node){
        cout << "Running decomp " << node << " isAP: " << isAP[node] << '\n';
        if(isAP[node]){
            Component comp = Component{AP_TYPE, node};
            cellComps[node] = comp;
            compSizes[comp] = 1;
            compGraph[comp] = unordered_set<Component, ComponentHash>();
            for(Pos neigh : env.emptyNeighs(fromID(node))){
                if(!visited[ID(neigh)]){
                    visited[ID(neigh)] = true;
                    getDecomposition(ID(neigh));
                    parentComp[cellComps[ID(neigh)]] = comp;
                }
                compGraph[comp].insert(cellComps[ID(neigh)]);
            }
        }
        else{
            Component comp = Component{BCC_TYPE, component_counter++};
            cellComps[node] = comp;
            compGraph[comp] = unordered_set<Component, ComponentHash>();

            adjNodes = vector<int>();
            compSize = 0;
            fillComponent(node, comp);

            vector<int> adjNodes_ = adjNodes;
            int compSize_ = compSize;

            for(const int& adj : adjNodes_){
                if(!visited[adj]){
                    visited[adj] = true;
                    getDecomposition(adj);
                    parentComp[cellComps[adj]] = comp;
                }
                compGraph[comp].insert(cellComps[adj]);
            }
            compSizes[comp] = compSize_;
        }
    }

    string toString(){
        string s = "";
        for(auto& [comp, adj] : compGraph){
            s += compToString(comp) + ", size " + to_string(compSizes[comp]) + ", parentComp " + (parentComp.find(comp) == parentComp.end() ? "None" : compToString(parentComp[comp])) + ':';
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

    

    int distToApple(const Snake& env){ // estimate for distance to apple
        // cout << "Calling...\n";
        TarjanDecomposition decomp(env);

        unordered_map<Component, int, ComponentHash> compMaxTimes;
        for(const auto& [comp, size] : decomp.compSizes){
            unordered_set<Component, ComponentHash> compsOnPath;
            Component currComp = comp;
            // cout << "Checking " << compToString(comp) << '\n';
            while(true){
                compsOnPath.insert(currComp);
                if(currComp.type == BCC_TYPE){
                    for(const Component& neigh : decomp.compGraph[currComp]){
                        if(neigh.type == AP_TYPE){
                            compsOnPath.insert(neigh);
                        }
                    }
                }
                if(decomp.parentComp.find(currComp) == decomp.parentComp.end()){
                    break;
                }
                currComp = decomp.parentComp[currComp];
            }
            int totalSize = 0;
            for(const Component& c : compsOnPath){
                totalSize += decomp.compSizes[c];
            }
            compMaxTimes[comp] = totalSize-1;
        }
        
        // Get latest times assuming no tail retraction
        vector<int> maxTimes(area, -1);
        for(int i=0; i<area; i++){
            if(decomp.cellComps[i].type != -1){
                maxTimes[i] = compMaxTimes[decomp.cellComps[i]];
                if((maxTimes[i] - manhattanDist(env.head, fromID(i))) % 2 == 1){
                    maxTimes[i] --;
                }
            }
        }

        maxTimes[ID(env.head)] = 0;

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
            // cout << "Step " << t << '\n';
            if(queue.find(env.apple) != queue.end()){
                shortestDist = min(shortestDist, t);
            }
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
                    next_queue.insert(p);
                }
                if(maxTimes[ID(p)] > t){
                    // cout << "Freed " + env.PosToCode(p) << '\n';
                    canEscape = true;
                }
            }
            retractibleBody[ID(curr_tail)] = false;
            if(curr_tail != env.head){
                curr_tail = shiftPos(curr_tail, env.body[ID(curr_tail)]);
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

            int minDist = INF;
            int bestAction = -1;
            // cout << "Finding dists...\n";
            for(int d : curr_env.validMoves()){
                Snake newEnv = curr_env;
                newEnv.move(d);
                int dist = distToApple(newEnv);
                // cout << "Action " << d << " dist: " << dist << '\n';
                if(minDist > dist){
                    minDist = dist;
                    bestAction = d;
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
        for(int i=0; i<area; i++){
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
            if(env.head == env.apple) env.randomizeApple();

            // if(i >= 195)
                getline(cin, s);
            
            if(s.size() > 0) break;
        }
    }
};




int main(){
    srand(1234);

    Solver s;

    // Snake env("8x15_27x14_29x11_14549_nssaaaaaaaaaaaalgjaaaaaaaaaaaaoaaaaaaaaaaaaaaoaaaaaaaaaaaaaaoaaaaacssssspaaoaaaaacggggguaaoaaaaacxsssssssossssssaaaaaabogggggggfaaaaaeoaaaaaaaaaaaaaeoaaaaaaaaaaaaaeoaaaaaaaaaaaaaeoaabgggggghaaaeoaaerssaaacaaaeogggwaeaaacaaaeoxssraeaaacaaaeogggwaepaacaaaeoxssqhbuaacaaaeogggucespacaaaeoxsrssbguacaaaeoggvggospabgggjoxssrsqguaaaaaajgggwgjaaaaaaaaaxnssxsaaaaaaaaajlgggjaaaaaaaaaxtssssaaaaaaaaagggggjaaaaaaaaaxsssssspaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");
    Snake env("8x18_18x4_29x11_14694_nssaaaaaaaaaaaalgjaaaaaaaaaaaaoaaaaaaaaaaaaaaoaaaaaaaaaaaaaaoaaaaacssssspaaoaaaaacggggguaaoaaaaacxsssssssossssssbggggglogggggggjadnnnooaaaaaaaaaetttoobgggggggggggjooesnssssssssssoobjlgggggghggjooestpaaaaacxssooggguaaaaacggjooxsspaaaaacxssooggguaaaaacggjooxsspaaaaacxssooaaguaaaaacggjooaaaaaaaaacxsstoaaaaaaaaabgggjoaaaaaaaaaaaaaajaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");
    env.move(1);

    env.display();

    TarjanDecomposition tj_decomp(env);
    cout << tj_decomp.toString() << '\n';

    // cout << s.distToApple(env) << '\n';
    // s.simulate(env);
}