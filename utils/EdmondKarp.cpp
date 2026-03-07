#include "../data_structures/Graph.h"

// Function to test the given vertex 'w' and visit it if conditions are met
template <class T>
void testAndVisit(std::queue< Vertex<T>*> &q, Edge<T> *e, Vertex<T> *w, double residual) {
    // Check if the vertex 'w' is not visited and there is residual capacity
    if (! w->isVisited() && residual > 0) {
        // Mark 'w' as visited, set the path through which it was reached, and enqueue it
        w->setVisited(true);
        w->setPath(e);
        q.push(w);
    }
}

// Function to find an augmenting path using Breadth-First Search
template <class T>
bool findAugmentingPath(Graph<T> *g, Vertex<T> *s, Vertex<T> *t) {
    for(auto v : g->getVertexSet()) {
        v->setVisited(false);
        v->setPath(nullptr);
    }

    std::queue<Vertex<T>*> q;
    q.push(s);
    s->setVisited(true);

    while(!q.empty() && !t->isVisited()) {
        auto v = q.front();
        q.pop();

        // explore forward edges
        for(auto e : v->getAdj()) {
            auto w = e->getDest();
            double residual = e->getWeight() - e->getFlow();
            testAndVisit(q, e, w, residual);
        }

        // explore backward edges
        for(auto e : v->getIncoming()) {
            auto w = e->getOrig();
            double residual = e->getFlow();
            testAndVisit(q, e, w, residual);
        }
    }
    return t->isVisited();
}

template <class T>
double findMinResidualAlongPath(Vertex<T> *s, Vertex<T> *t) {
    double f = INF;
    for(auto v = t; v != s; ) {
        Edge<T>* e = v->getPath();

        if(e->getDest() == v) {
            f = std::min(f, e->getWeight() - e->getFlow());
            v = e->getOrig();
        }
        else {
            f = std::min(f, e->getFlow());
            v = e->getDest();
        }
    }

    return f;
}

template <class T>
void augmentFlowAlongPath(Vertex<T> *s, Vertex<T> *t, double f) {
    for(auto v = t; v != s; ) {
        Edge<T>* e = v->getPath();

        if(e->getDest() == v) {
            e->setFlow(e->getFlow() + f);
            v = e->getOrig();
        }
        else {
            e->setFlow(e->getFlow() - f);
            v = e->getDest();
        }
    }
}

template <class T>
void edmondsKarp(Graph<T> *g, int source, int target) {
    Vertex<T>* s = g->findVertex(source);
    Vertex<T>* t = g->findVertex(target);

    if(s == nullptr || t == nullptr) return;

    for(auto v : g->getVertexSet())
        for(auto e : v->getAdj())
            e->setFlow(0);

    while(findAugmentingPath(g, s, t)) {
        double f = findMinResidualAlongPath(s, t);
        augmentFlowAlongPath(s, t, f);
    }
}

///// TESTS ///
//#include <gtest/gtest.h>
//
//TEST(Algorithm_1, test_edmondsKarp) {
//    Graph<int> myGraph;
//
//    for(int i = 1; i <= 6; i++)
//        myGraph.addVertex(i);
//
//    myGraph.addEdge(1, 2, 3);
//    myGraph.addEdge(1, 3, 2);
//    myGraph.addEdge(2, 5, 4);
//    myGraph.addEdge(2, 4, 3);
//    myGraph.addEdge(2, 3, 1);
//    myGraph.addEdge(3, 5, 2);
//    myGraph.addEdge(4, 6, 2);
//    myGraph.addEdge(5, 6, 3);
//
//    edmondsKarp(&myGraph, 1, 6);
//
//    std::stringstream ss;
//    for(auto v : myGraph.getVertexSet()) {
//        ss << v->getInfo() << "-> (";
//        for (const auto e : v->getAdj())
//            ss << (e->getDest())->getInfo() << "[Flow: " << e->getFlow() << "] ";
//        ss << ") || ";
//    }
//
//    std::cout << ss.str() << std::endl << std::endl;
//
//    EXPECT_EQ("1-> (2[Flow: 3] 3[Flow: 2] ) || 2-> (5[Flow: 1] 4[Flow: 2] 3[Flow: 0] ) || 3-> (5[Flow: 2] ) || 4-> (6[Flow: 2] ) || 5-> (6[Flow: 3] ) || 6-> () || ", ss.str());
//
//}