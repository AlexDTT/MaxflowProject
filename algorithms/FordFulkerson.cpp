
#include "algorithms/FordFulkerson.h"
#include <algorithm>

// Recursive DFS helper to find an augmenting path from v to t
template <class T>
bool dfsAugment(Vertex<T> *v, Vertex<T> *t)
{
    v->setVisited(true);

    if (v == t)
        return true;

    for (auto e : v->getAdj())
    {
        Vertex<T> *w = e->getDest();
        double residual = e->getWeight() - e->getFlow();

        if (!w->isVisited() && residual > 0)
        {
            w->setPath(e);
            if (dfsAugment(w, t))
                return true;
        }
    }

    return false;
}

// Function to find an augmenting path using Depth-First Search
template <class T>
bool ffFindAugmentingPath(Graph<T> *g, Vertex<T> *s, Vertex<T> *t)
{
    for (auto v : g->getVertexSet())
    {
        v->setVisited(false);
    }

    return dfsAugment(s, t);
}

// Function to find the minimum residual capacity along the augmenting path
template <class T>
double ffFindMinResidualAlongPath(Vertex<T> *s, Vertex<T> *t)
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

    return f;
}

// Function to augment flow along the augmenting path with the given flow value
template <class T>
void ffAugmentFlowAlongPath(Vertex<T> *s, Vertex<T> *t, double f)
{
    Vertex<T> *v = t;
    while (v != s)
    {
        Edge<T> *e = v->getPath();
        e->setFlow(e->getFlow() + f);
        e->getReverse()->setFlow(e->getReverse()->getFlow() - f);

        v = v->getPath()->getOrig();
    }
}

// Main function implementing the Ford-Fulkerson algorithm
template <class T>
void fordFulkerson(Graph<T> *g, T source, T target, std::function<void(const std::vector<T> &, double)> onAugment)
{
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

    while (ffFindAugmentingPath(g, s, t))
    {
        double f = ffFindMinResidualAlongPath(s, t);

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

        ffAugmentFlowAlongPath(s, t, f);
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

// Explicit instantiations for types used by the program
template void fordFulkerson<int>(Graph<int> *, int, int, std::function<void(const std::vector<int> &, double)>);
