#include <iostream>
#include "cdb.h"

using namespace std;

void test_insert() {
    cdb<NodeData> c("ASN", "PREFIX");

    NodeData n1("4538", "10.10.1.0/24", "4538 3398 2239 238 29", 123333123);
    c.insert_data("4538", "10.10.1.0/24", n1, _REPLACE_BY_TIME);
    c.insert_data("4538", "10.10.2.0/24", n1.set_data("4538", "10.10.2.0/24", "4538 3398 2239 238 29", 123333124));
    c.insert_data("4537", "10.10.1.0/24", n1.set_data("4537", "10.10.1.0/24", "4538 3398 2239 238 29", 123333125));
    c.insert_data("4537", "10.10.4.0/24", n1.set_data("4537", "10.10.4.0/24", "4538 3398 2239 238 29", 123333126));

    chead<NodeData> *col = c.get_col("10.10.1.0/24");
    chead<NodeData> *row = c.get_row("4538");
    cnode<NodeData> *node = c.get_node("4537", "10.10.4.0/24");

    for (cnode<NodeData> *p = col->get_first_node(); p; p=p->cnext()) {
        NodeData n = p->data();
        n.print_data();
    }
    cout << "##" << endl;
    for (cnode<NodeData> *p = row->get_first_node(); p; p=p->rnext()) {
        NodeData n = p->data();
        n.print_data();
    }
    cout << "##" << endl;

    node->data().print_data();
//
//    for (int i = 1; i < 1999999; i++) {
//        c.insert_data(to_string(i), "10.10.2.0/24", n1.set_data("4538", "10.10.2.0/24", "4538 3398 2239 238 29", 123333124));
//    }

    cout<< "end" << endl;

    string a;
    cin >> a;
}

int main() {
    test_insert();
}