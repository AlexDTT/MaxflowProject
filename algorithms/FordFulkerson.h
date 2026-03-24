/**
 * @file FordFulkerson.h
 * @brief Implementation of the Ford-Fulkerson algorithm for calculating maximum flow.
 *
 * Provides template-based implementations of DFS-based path augmentation
 * on generic directed flow networks.
 */

#ifndef MAXFLOW_FORDFULKERSON_H
#define MAXFLOW_FORDFULKERSON_H

#include "data_structures/Graph.h"
#include <vector>
#include <stdexcept>
#include <functional>

/**
 * @brief Recursive DFS helper to find an augmenting path from v to t.
 * @param v The current vertex being explored.
 * @param t The target (sink) vertex.
 * @return True if an augmenting path exists from v to t, false otherwise.
 * @complexity O(V + E) for DFS traversing vertices and edges.
 */
template <class T>
bool dfsAugment(Vertex<T> *v, Vertex<T> *t);

/**
 * @brief Finds an augmenting path from source s to target t using DFS.
 * @param g The graph.
 * @param s The source vertex.
 * @param t The target (sink) vertex.
 * @return True if an augmenting path exists, false otherwise.
 * @complexity O(V + E) for DFS traversing all vertices and edges.
 */
template <class T>
bool ffFindAugmentingPath(Graph<T> *g, Vertex<T> *s, Vertex<T> *t);

/**
 * @brief Finds the minimum residual capacity along an augmenting path.
 * @param s The source vertex.
 * @param t The target (sink) vertex.
 * @return The bottleneck (minimum residual) capacity.
 * @complexity O(V) where V is the max length of the path.
 */
template <class T>
double ffFindMinResidualAlongPath(Vertex<T> *s, Vertex<T> *t);

/**
 * @brief Augments the flow along the path by a given amount f.
 * @param s The source vertex.
 * @param t The target (sink) vertex.
 * @param f The flow amount to augment by (usually the minimum residual).
 * @complexity O(V) where V is the max length of the path.
 */
template <class T>
void ffAugmentFlowAlongPath(Vertex<T> *s, Vertex<T> *t, double f);

/**
 * @brief Executes the Ford-Fulkerson algorithm to find the Maximum Flow.
 * @param g The graph.
 * @param source The value representing the source node.
 * @param target The value representing the sink node.
 * @param onAugment Optional callback invoked whenever a path is augmented.
 * @complexity O(F * E), where F is the maximum flow value.
 *             In practice, F = P * MinReviewsPerSubmission, bounded by a small
 *             constant times P.  The graph has diameter 3
 *             (source -> submission -> reviewer -> sink), so DFS finds paths quickly.
 */
template <class T>
void fordFulkerson(Graph<T> *g, T source, T target, std::function<void(const std::vector<T>&, double)> onAugment = nullptr);

#endif // MAXFLOW_FORDFULKERSON_H
