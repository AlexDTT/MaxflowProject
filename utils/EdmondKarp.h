/**
 * @file EdmondKarp.h
 * @brief Implementations of the Edmonds-Karp algorithm for calculating maximum flow.
 * 
 * Provides template-based implementations of BFS-based path augmentation 
 * on generic directed flow networks.
 */

#ifndef MAXFLOW_EDMONDKARP_H
#define MAXFLOW_EDMONDKARP_H

#include "../data_structures/Graph.h"
#include <queue>
#include <vector>
#include <stdexcept>
#include <functional>

/**
 * @brief Helper function to test and visit a vertex during BFS.
 * @param q The BFS queue.
 * @param e The edge connecting the current vertex to w.
 * @param w The destination vertex to potentially visit.
 * @param residual The residual capacity of the edge e.
 */
template <class T>
void testAndVisit(std::queue< Vertex<T>*> &q, Edge<T> *e, Vertex<T> *w, double residual);

/**
 * @brief Finds an augmenting path from source s to target t using BFS.
 * @param g The graph.
 * @param s The source vertex.
 * @param t The target (sink) vertex.
 * @return True if an augmenting path exists, false otherwise.
 * @complexity O(V + E) for BFS traversing all vertices and edges.
 */
template <class T>
bool findAugmentingPath(Graph<T> *g, Vertex<T> *s, Vertex<T> *t);

/**
 * @brief Finds the minimum residual capacity along an augmenting path.
 * @param s The source vertex.
 * @param t The target (sink) vertex.
 * @return The bottleneck (minimum residual) capacity.
 * @complexity O(V) where V is the max length of the path.
 */
template <class T>
double findMinResidualAlongPath(Vertex<T> *s, Vertex<T> *t);

/**
 * @brief Augments the flow along the path by a given amount f.
 * @param s The source vertex.
 * @param t The target (sink) vertex.
 * @param f The flow amount to augment by (usually the minimum residual).
 * @complexity O(V) where V is the max length of the path.
 */
template <class T>
void augmentFlowAlongPath(Vertex<T> *s, Vertex<T> *t, double f);

/**
 * @brief Executes the Edmonds-Karp algorithm to find the Maximum Flow.
 * @param g The graph.
 * @param source The value representing the source node.
 * @param target The value representing the sink node.
 * @param onAugment Optional callback invoked whenever a path is augmented.
 * @complexity O(V * E^2) in the worst case, as the shortest path length increases monotonically, 
 *             and there are at most O(V * E) augmentations. Each BFS takes O(E).
 */
template <class T>
void edmondsKarp(Graph<T> *g, T source, T target, std::function<void(const std::vector<T>&, double)> onAugment = nullptr);

#endif // MAXFLOW_EDMONDKARP_H
