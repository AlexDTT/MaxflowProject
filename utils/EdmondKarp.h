// Declarations for Edmonds-Karp helper functions
// (Implementations remain in EdmondKarp.cpp)

#ifndef MAXFLOW_EDMONDKARP_H
#define MAXFLOW_EDMONDKARP_H

#include "../data_structures/Graph.h"
#include <queue>
#include <vector>
#include <stdexcept>

template <class T>
void testAndVisit(std::queue< Vertex<T>*> &q, Edge<T> *e, Vertex<T> *w, double residual);

template <class T>
bool findAugmentingPath(Graph<T> *g, Vertex<T> *s, Vertex<T> *t);

template <class T>
double findMinResidualAlongPath(Vertex<T> *s, Vertex<T> *t);

template <class T>
void augmentFlowAlongPath(Vertex<T> *s, Vertex<T> *t, double f);

template <class T>
void edmondsKarp(Graph<T> *g, T source, T target);

#endif // MAXFLOW_EDMONDKARP_H
