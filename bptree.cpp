#include<vector>
#include<cstdlib>
#include<cassert>
#include<cstdio>
#include<iostream>
#include<fstream>
#include<string>
#include<sstream>

#define DEBUG
#ifdef DEBUG
#define Assert(exp) assert((exp))

#else
#define Assert(exp)
#endif // DEBUG
/*
 * Internal node and leaf node use the same data struct.
 * node child num = key num + 1
 */
struct Record{
    int key;
    void *value;
};

class Node{
public:
    int keyNum;
    std::vector<int> keyName;
    std::vector<Node *> child; // leaf child store record
    bool isLeaf;

    Node *parent;
    Node *next; // leaf node has next
    int node_find(int key);
    bool node_contain(int key);
    void node_insert(int key, Node *node);
    void node_erase(int pos);
    Node* node_split(int pos);
    void node_merge(Node *p, Node *q);
    void print();
    Node(){
      keyNum = 0;
      isLeaf = 1;
      parent = NULL;
      next = NULL;
    };
    Node(Node *q){
        keyNum = q->keyNum;
        isLeaf = q->isLeaf;
        keyName = std::vector<int>(q->keyName);
        child = std::vector<Node *>(q->child);
        parent = q->parent;
        next = q->next;
    };

    Node* nxtsibling();
    Node* prvsibling();

};

/*
 * 1. root key: 1 ~ m-1,
 *      other: ceiling(m/2)-1 ~ m-1
 */
class BPTree{
public:
    int m;
    Node *root;
    Node *link;
    BPTree(int n){
        m = n;
        root = new Node();
        link = root;
    };
    Node* find(int key);
    void insert(int key);
    void erase(int key);
    void tprint();
    void link_traver();
};

// return the first index keyName > key, if key max, return keyNum
// return can review as the index of child for the key
// mind: if in keyName, return value must -1;
int Node::node_find(int key){
  //  print();
   // std::cout << isLeaf << std::endl;
    int i = 0;
    while(i<keyNum && keyName[i]<=key)i++;

    return i;
}


bool Node::node_contain(int key){
    int i = 0;
    while(i<keyNum && keyName[i]<key)i++;
    return i<keyNum && keyName[i] == key;
}
// isRight: 1 = node is in the right of the key
void Node::node_insert(int key, Node *node){
    int pos = node_find(key);
    keyName.insert(keyName.begin()+pos, key);
    child.insert(child.begin()+pos+(!isLeaf), node);
    if(node)node->parent = this;
    keyNum ++;
}

// erase pos key in node, mind: key == parent'key
// erase only happen in keyNum > minNum
void Node::node_erase(int key){
    std::cout <<"erase" << key <<std::endl;
    int pos = node_find(key) - 1;
    Assert(pos < keyNum && pos>=0);
    if(parent&&!pos&&isLeaf){
        // not need to consider pos+1 out of index
        parent->keyName[parent->node_find(key)-1] = keyName[pos+1];
    }
    keyName.erase(keyName.begin() + pos);
    child.erase(child.begin()+pos+(!isLeaf));

    keyNum --;
}

Node* Node::node_split(int pos){
    Node *p = new Node();
    int i = pos;
    if(!isLeaf)i++; // Internal node split,key in the pos send to parent
    p->keyNum = keyNum - i;
    p->isLeaf = isLeaf;
    while( i < keyNum){
        p->child.push_back(child[i]);
        p->keyName.push_back(keyName[i]);
        i++;
    }
    if(isLeaf){
        p->next = this->next;
        this->next = p;
    }
    else{
        p->child.push_back(child.back());
    }
    keyNum = pos;
    keyName.erase(keyName.begin()+ pos, keyName.end());
    child.erase(child.begin()+ pos+(!isLeaf),child.end());
    return p;
    // parent consider in  insert
}

Node* Node::nxtsibling(){
    int pos = parent->node_find(keyName[0]);
    if(pos == parent->keyNum)
        return NULL;
    return parent->child[pos+1];
}

Node* Node::prvsibling(){
    int pos = parent->node_find(keyName[0]);
    if(pos == 0)
        return NULL;
    return parent->child[pos-1];
}

void Node::print(){
    if(!parent)
        std::cout << "root: ";
    else{
        std::cout << parent->keyName[0] << ": ";
        if(isLeaf)std::cout <<"leaf ";
    }
    for(int i=0;i<keyNum;i++){
        std::cout << keyName[i] << "  ";
    }
    std::cout << std::endl;
    if(!isLeaf){
       for(int i=0;i<=keyNum;i++)
            child[i]->print();
    }else{
        Assert(child.size() == keyNum);
        for(int i=0;i<keyNum;i++)
            Assert(child[i]==NULL);
    }


}

void BPTree::tprint(){
    root->print();
    link_traver();
}
/*
 * merge two adjoint node in the same height
 * mind: internal node p,q merge should add a parent key.
 */
void Node::node_merge(Node *p, Node *q){
   // std::cout << "merge: " << p->keyName[0] << " " << q->keyName[0] << std::endl;
    int i = 0, pos;
    if(!isLeaf){
        pos = p->parent->node_find(q->keyName[0]);
        p->keyName.push_back(p->parent->keyName[pos-1]);
        p->keyNum ++;
    }
    while( i < q->keyNum){
        p->child.push_back(q->child[i]);
        p->keyName.push_back(q->keyName[i]);
        i++;
    }
    if(!isLeaf){
        p->child.push_back(q->child[i]);
    }else{
        p->next = q->next;
    }
    p->keyNum += q->keyNum;
    for(int i=0;i<p->keyNum;i++){
        std::cout << p->keyName[i] << "  ";
    }
    std::cout <<std::endl;
    free(q);
}

// return the leaf node able to contain key
Node* BPTree::find(int key){
    Node *p = root;
    while(p&&!p->isLeaf)p = p->child[p->node_find(key)];
   // int pos = p->node_find(key);
   if(!p)std:: cout << "find: " << p->keyName[0] << std::endl;
    return p;
}

/*
 * insert:
 */
void BPTree::insert(int key){
    int pos;
    Node *p = find(key), *newNode = NULL;
    if(p->node_contain(key))return;
    while(true){
        p->node_insert(key, newNode);
        if(p->keyNum < m) return;
        key = p->keyName[m/2];
        newNode = p->node_split(m/2);

        if(p->parent)p = p->parent;
        else{
            root = new Node();
            root->isLeaf = 0;

            root->child.push_back(p);
            p->parent = root;

            root->node_insert(key,newNode);
            return;
        }
    }
    return;
}

void BPTree::erase(int key){
    int minNum = (m+1)/2 - 1, pos, temp;
    Node *p = find(key);
    std::cout << p->node_contain(key);
    if(!p->node_contain(key))return;
    while(1){
        if(p == root && p->keyNum == 1){
            std:: cout << "rottt " <<std::endl;
            root = p->child[0];
            root->parent = NULL;
          //  free(p);
            return;
        }
        if(p==root || p->keyNum > minNum){
            p->node_erase(key);
            return;
        }
        Node *nxtsib = p->nxtsibling();
        if(nxtsib && nxtsib->keyNum > minNum){
            temp = nxtsib->keyName[0];
            pos = p->parent->node_find(temp);
            p->parent->keyName[pos-1] = nxtsib->keyName[1];
            nxtsib->node_erase(temp);
            p->node_insert(temp, NULL);
            p->node_erase(key);
            return;
        }
        Node *sib = p->prvsibling();
        if(sib && sib->keyNum > minNum){
            temp = sib->keyName.back();
            pos = p->parent->node_find(key);
            p->parent->keyName[pos-1] = temp;
            sib->node_erase(temp);
            p->node_insert(temp, NULL);
            p->node_erase(key);
            return;
        }
        if(nxtsib){
            temp = nxtsib->keyName[0];
            p->node_merge(p, nxtsib);
            p->node_erase(key);
            key = temp;
            p = p->parent;
        }
        else if(sib){
            temp= p->keyName[0];
            p->node_merge(sib, p);
            sib->node_erase(key);
            key = temp;
            p = sib->parent;
        }
    }

}

void node_test(){
    Node *p = new Node(), *q, *c;
   // p->node_insert();
    int degree;
    std::ifstream fin("nodetest.txt");
    std::string line, item;
    getline(fin, line);
    std::cout << "degree: " << std::stoi(line) << std::endl;
    degree = std::stoi(line);

    getline(fin, line);
    std::istringstream items(line);
    std::cout << "Keys: ";
    while(items >> item){
        std::cout << std::stoi(item) << " ";
        p->node_insert(std::stoi(item), NULL);
       // p->print();
    }
    std::cout << std::endl;
    p->print();
    std::cout << "===============================" <<std::endl;
    while(getline(fin, line)){
        std::istringstream items(line);
        std::string op, key;
        c = new Node(p);
        int res;
        std::cout << line << std::endl;
        while(items >> op >> key){
            std::cout << op << key << ":  " <<std::endl;
            switch(op[0]){
            case 'f':
                std::cout << "Find " << key << ":  " <<std::endl;
                res = c->node_find(std::stoi(key));
                std::cout << res << std::endl;
                break;
            case 'i':
                std::cout << "Insert " << key << ":  " <<std::endl;
                c->node_insert(std::stoi(key), NULL);
                c->print();
                std::cout << "===============================" <<std::endl;
                break;
            case 'e':
                std::cout << "Erase " << key << ":  " <<std::endl;
                c->node_erase(std::stoi(key));
                c->print();
                std::cout << "===============================" <<std::endl;
                break;
            case 's':
                std::cout << "Split " << key << "(index):  " <<std::endl;// key is pos
                q = c->node_split(std::stoi(key));
                c->print();
                q->print();
                std::cout << "===============================" <<std::endl;
                break;
            }
        }
    }
}

void bpt_test(){
    BPTree *tree;
    std::ifstream fin("bptreetest.txt");
    std::string line, item;

    while(getline(fin, line)){
        if(line[0]=='#') getline(fin, line);
        std::cout << "degree: " << std::stoi(line) << std::endl;
        tree = new BPTree(std::stoi(line));

        getline(fin, line);
        std::istringstream items(line);
        std::cout << "Keys: ";
        while(items >> item){
            std::cout << std::stoi(item) << " ";
            tree->insert(std::stoi(item));
        }
        std::cout << std::endl;
        tree->tprint();
        std::cout << "===============================" <<std::endl;
        while(getline(fin, line)){
            if(line[0]=='#')break;
            std::istringstream items(line);
            std::string op, key;

            while(items >> op >> key){
                Node *res;
                switch(op[0]){
                case 'f':
//                    std::cout << "Find " << key << ": ";
//                    res = tree->find(std::stoi(key));
//                    if(res)res->print();
//                    else std::cout <<"Not Found." << std::endl;
                    break;
                case 'i':
                    std::cout << "Insert " << key << ":  " <<std::endl;
                    tree->insert(std::stoi(key));
                    tree->tprint();
                    std::cout << "===============================" <<std::endl;
                    break;
                case 'e':
                    std::cout << "Erase " << key << ":  " <<std::endl;
                    tree->erase(std::stoi(key));
                    tree->tprint();
                    std::cout << "===============================" <<std::endl;
                    break;
                }
            }
        }
     }
    fin.close();
}

void BPTree::link_traver(){
    Node *p = link;
    while(p){
        p->print();
        p = p->next;
    }
}

int main(){
    //node_test();
    bpt_test();
    return 0;
}
