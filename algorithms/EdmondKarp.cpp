
#include "algorithms/EdmondKarp.h"
#include <algorithm>

// Function to test the given vertex 'w' and visit it if conditions are met
template <class T>
void testAndVisit(std::queue<Vertex<T> *> &q, Edge<T> *e, Vertex<T> *w, double residual)
{
    // Check if the vertex 'w' is not visited and there is residual capacity
    if (!w->isVisited() && residual > 0)
    {
        // Mark 'w' as visited, set the path through which it was reached, and enqueue it
        w->setVisited(true);
        w->setPath(e);
        q.push(w);
    }
}

// Function to find an augmenting path using Breadth-First Search
template <class T>
bool findAugmentingPath(Graph<T> *g, Vertex<T> *s, Vertex<T> *t)
{
    // Mark all vertices as not visited
    for (auto v : g->getVertexSet())
    {
        v->setVisited(false);
    }

    std::queue<Vertex<T> *> q;
    q.push(s);
    s->setVisited(true);

    while (!q.empty())
    {
        Vertex<T> *v = q.front();
        q.pop();

        // Explore all adjacent edges of vertex 'v'
        for (auto e : v->getAdj())
        {
            Vertex<T> *w = e->getDest();
            double residual = e->getWeight() - e->getFlow();
            testAndVisit(q, e, w, residual);
        }
    }

    // Return true if a path to the target is found, false otherwise
    return t->isVisited();
}

// Function to find the minimum residual capacity along the augmenting path
template <class T>
double findMinResidualAlongPath(Vertex<T> *s, Vertex<T> *t)
{
    double f = INF;

    Vertex<T> *v = t;
    while (v != s)
    {
        Edge<T> *e = v->getPath();

        double residual = e->getWeight() - e->getFlow();
        f = std::min(f, residual);

        v = v->getPath()->getOrig();
    }

    // Return the minimum residual capacity
    return f;
}

// Function to augment flow along the augmenting path with the given flow value
template <class T>
void augmentFlowAlongPath(Vertex<T> *s, Vertex<T> *t, double f)
{
    // Traverse the augmenting path and update the flow values accordingly

    Vertex<T> *v = t;
    while (v != s)
    {
        Edge<T> *e = v->getPath();
        e->setFlow(e->getFlow() + f);
        e->getReverse()->setFlow(e->getReverse()->getFlow() - f);

        v = v->getPath()->getOrig();
    }
}

// Main function implementing the Edmonds-Karp algorithm
template <class T>
void edmondsKarp(Graph<T> *g, T source, T target, std::function<void(const std::vector<T> &, double)> onAugment)
{
    // Find source and target vertices in the graph
    Vertex<T> *s = g->findVertex(source);
    Vertex<T> *t = g->findVertex(target);

    if (s == nullptr || t == nullptr || s == t)
        throw std::logic_error("Invalid source and/or target vertex");

    for (auto v : g->getVertexSet())
        for (auto e : v->getAdj())
            e->setFlow(0);
    std::vector<Edge<T> *> originalEdges;
    for (auto v : g->getVertexSet())
        for (auto e : v->getAdj())
            originalEdges.push_back(e);

    for (auto e : originalEdges)
    {
        if (e->getReverse() == nullptr)
        {
            Edge<T> *rev = e->getDest()->addEdge(e->getOrig(), 0);
            rev->setFlow(0);
            e->setReverse(rev);
            rev->setReverse(e);
        }
    }

    while (findAugmentingPath(g, s, t))
    {
        double f = findMinResidualAlongPath(s, t);

        if (onAugment)
        {
            std::vector<T> pathNodes;
            Vertex<T> *v = t;
            while (v != s)
            {
                pathNodes.push_back(v->getInfo());
                v = v->getPath()->getOrig();
            }
            pathNodes.push_back(s->getInfo());
            std::reverse(pathNodes.begin(), pathNodes.end());
            onAugment(pathNodes, f);
        }

        augmentFlowAlongPath(s, t, f);
    }

    for (auto v : g->getVertexSet())
    {
        std::vector<T> toRemove;
        for (auto e : v->getAdj())
        {
            if (e->getWeight() == 0)
            {
                toRemove.push_back(e->getDest()->getInfo());
            }
        }
        for (auto &dest : toRemove)
        {
            v->removeEdge(dest);
        }
    }
}

// /// TESTS ///
// #include <gtest/gtest.h>

// TEST(Algorithm_1, test_edmondsKarp) {
//     Graph<int> myGraph;

//     for(int i = 1; i <= 6; i++)
//         myGraph.addVertex(i);

//     myGraph.addEdge(1, 2, 3);
//     myGraph.addEdge(1, 3, 2);
//     myGraph.addEdge(2, 5, 4);
//     myGraph.addEdge(2, 4, 3);
//     myGraph.addEdge(2, 3, 1);
//     myGraph.addEdge(3, 5, 2);
//     myGraph.addEdge(4, 6, 2);
//     myGraph.addEdge(5, 6, 3);

//     edmondsKarp(&myGraph, 1, 6);

//     std::stringstream ss;
//     for(auto v : myGraph.getVertexSet()) {
//         ss << v->getInfo() << "-> (";
//         for (const auto e : v->getAdj())
//             ss << (e->getDest())->getInfo() << "[Flow: " << e->getFlow() << "] ";
//         ss << ") || ";
//     }

//     std::cout << ss.str() << std::endl << std::endl;

//     EXPECT_EQ("1-> (2[Flow: 3] 3[Flow: 2] ) || 2-> (5[Flow: 1] 4[Flow: 2] 3[Flow: 0] ) || 3-> (5[Flow: 2] ) || 4-> (6[Flow: 2] ) || 5-> (6[Flow: 3] ) || 6-> () || ", ss.str());

// }

// Explicit instantiations for types used by the program
template void edmondsKarp<int>(Graph<int> *, int, int, std::function<void(const std::vector<int> &, double)>);
