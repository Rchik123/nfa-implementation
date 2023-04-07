#include <iostream>
#include <stack>
#include <map>
#include <vector>
#include <set>

using namespace std;

int nodeNum = 0;
int acceptNum = 0;
int moveNum = 0;

struct node{
    int index;
    bool accept;
    map<char, set<node*>> moves;
};

struct nfa{
    node* start;
    node* end;
};

map<char, int> operatorMap;
vector<node*> nodes;
set<node*> visited;
set<pair<node*, node*>> epsilonSet;

bool isStrongerOrEqual(char a, char b){
    if(operatorMap[a] >= operatorMap[b]) return true;
    return false;
}

bool isTerminal(char ch){
    if((ch >= 'a' && ch <= 'z') || (ch >= '0' && ch <= '9') || ch == 'E') return true;
    return false;
}

void addMove(node* node1, node* node2, char ch){
    node1->moves[ch].insert(node2);
}

nfa buildTerminal(char ch){
    node* node1 = new node;
    node1->accept = false;

    node* node2 = new node;
    node2->accept = true;

    addMove(node1, node2, ch);
            
    nfa result;
    result.start = node1;
    result.end = node2;

    return result;
}

nfa buildConcat(nfa& nfa1, nfa& nfa2){
    nfa1.end->accept = false;
    addMove(nfa1.end, nfa2.start, 'E');
            
    nfa newNfa;
    newNfa.start = nfa1.start;
    newNfa.end = nfa2.end;

    return newNfa;
}

nfa buildUnion(nfa& nfa1, nfa& nfa2){
    node* startNode = new node;
    startNode->accept = false;
    addMove(startNode, nfa1.start, 'E');
    addMove(startNode, nfa2.start, 'E');

    node* endNode = new node;
    endNode->accept = true;
    nfa1.end->accept = false;
    addMove(nfa1.end, endNode, 'E');
    nfa2.end->accept = false;
    addMove(nfa2.end, endNode, 'E');

    nfa newNfa;
    newNfa.start = startNode;
    newNfa.end = endNode;

    return newNfa;
}

nfa buildStar(nfa& nfa1){
    node* startNode = new node;
    node* endNode = new node;

    startNode->accept = false;
    addMove(startNode, nfa1.start, 'E');
    addMove(startNode, endNode, 'E');

    nfa1.end->accept = false;
    addMove(nfa1.end, nfa1.start, 'E');
    addMove(nfa1.end, endNode, 'E');

    endNode->accept = true;

    nfa newNfa;
    newNfa.start = startNode;
    newNfa.end = endNode;

    return newNfa;
}

void addMoves(node* from, node* to){
    for (auto const& x : to->moves){
        char moveSymbol = x.first;
        set<node*> toNodes = to->moves[moveSymbol];

        for(auto const&y : toNodes){
            if(moveSymbol == 'E' && y == from){
                
            } else {
                if(moveSymbol == 'E'){
                    pair<node*, node*> tmp;
                    tmp.first = from;
                    tmp.second = y;

                    if(epsilonSet.count(tmp) == 0){
                        addMove(from, y, moveSymbol);
                        epsilonSet.insert(tmp);
                    }
                } else {
                    addMove(from, y, moveSymbol);
                }
            }
        }
    }
}

void removeEpsilons(node* n){
    if(visited.count(n) == 1) return;
    
    if(n->moves['E'].size() > 0){
        set<node*> toNodes = n->moves['E'];
        node* toNode = *(toNodes.begin());

        if(toNode->accept){ 
            n->accept = true;
        };

        n->moves['E'].erase(toNode);

        addMoves(n, toNode);

        removeEpsilons(n);
    } else {
        visited.insert(n);
        nodes.push_back(n);
        if(n->accept){
            acceptNum++;
        }
        nodeNum++;
        for (auto const& x : n->moves){
            char moveSymbol = x.first;
            set<node*> toNodes = n->moves[moveSymbol];
            moveNum += toNodes.size();

            for(auto const&y : toNodes){
                removeEpsilons(y);
            }
        }
    }

}

nfa buildNfa(string input){
    stack<nfa> nfaStack;
    for(int i = 0; i < input.length(); i++){
        if(isTerminal(input[i])){
            nfaStack.push(buildTerminal(input[i]));
        } else if(input[i] == '.'){
            nfa nfa2 = nfaStack.top();
            nfaStack.pop();
            nfa nfa1 = nfaStack.top();
            nfaStack.pop();

            nfaStack.push(buildConcat(nfa1, nfa2));
        } else if(input[i] == '|'){
            nfa nfa2 = nfaStack.top();
            nfaStack.pop();
            nfa nfa1 = nfaStack.top();
            nfaStack.pop();

            nfaStack.push(buildUnion(nfa1, nfa2));
        } else if(input[i] == '*'){
            nfa nfa1 = nfaStack.top();
            nfaStack.pop();
            nfaStack.push(buildStar(nfa1));
        }
    }

    nfa result = nfaStack.top();
    removeEpsilons(result.start);
    return result;
}

string modifyInput(string input){
    // add concat operator and epsilon
    for(int i = 0; i < input.length(); i++){
        if(i != input.length() - 1){
            if(input[i] == '(' && input[i + 1] == ')'){
                input = input.substr(0, i) + 'E' + input.substr(i + 2);
                i--;
                continue;
            }

            if(input[i] != '|' && input[i] != '(' && input[i + 1] != ')'
                && input[i + 1] != '|' && input[i + 1] != '*'){

                input = input.substr(0, i + 1) + '.' + input.substr(i + 1);
                i++;
            }
        }
    }

    // init operatorMap
    operatorMap['*'] = 4;
    operatorMap['.'] = 3;
    operatorMap['|'] = 2;
    operatorMap['('] = 1;

    // transform into postfix
    string output = "";
    stack<char> stack;
    for(int i = 0; i < input.length(); i++){
        char ch = input[i];

        if(isTerminal(ch)){
            output = output + ch;
        } else {
            if(ch == '('){
                stack.push(ch);
            } else if(ch == ')'){
                while(stack.top() != '('){
                    output = output + stack.top();
                    stack.pop();
                }
                stack.pop();
            } else {
                while(!stack.empty() && isStrongerOrEqual(stack.top(), ch)){
                    output = output + stack.top();
                    stack.pop();
                }
                stack.push(ch);
            }
        }
    }

    while(!stack.empty()){
        output = output + stack.top();
        stack.pop();
    }
    
    return output;
}

void printNodeMoves(node* n){
    int nMoves = 0;
    string output = "";

    for (auto const& x : n->moves){
        char moveSymbol = x.first;
        set<node*> toNodes = n->moves[moveSymbol];

        nMoves += toNodes.size();

        for(auto const& y : toNodes){
            output += moveSymbol;
            output += " ";
            output += to_string(y->index);
            output += " ";
        }

    }
    output = to_string(nMoves) + " " + output;
    
    cout << output << endl;
}

int main(){
    string input;
    cin >> input;

    input = modifyInput(input);
    nfa result = buildNfa(input);

    //index nodes
    int index = 1;
    for(int i = 0; i < nodes.size(); i++){
        if(nodes[i] == result.start){
            result.start->index = 0;
        } else {
            nodes[i]->index = index;
            index++;
        }

    }

    // output 
    cout << nodeNum << " " << acceptNum << " " << moveNum << endl;
    for(int i = 0; i < nodes.size(); i++){
        if(nodes[i]->accept == true){
            cout << nodes[i]->index << " ";
        }
    }
    cout << endl;
    printNodeMoves(result.start);
    for(int i = 0; i < nodes.size(); i++){
        if(nodes[i] != result.start) printNodeMoves(nodes[i]);
    }

    return 0;
}