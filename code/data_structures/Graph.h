/**
 * @file Graph.h
 * @brief Templated adjacency-list graph with Vertex and Edge classes.
 *
 * Provides the core data structure used throughout the project to represent
 * the interference graph. Each vertex stores an info value of type `T`
 * (here `int`, equal to the web ID) and a list of outgoing edges.
 *
 * Originally authored by Gonçalo Leão; updated by the Francisco Lourenço, Rodrigo Roque, José Lopes.
 *
 * @author Gonçalo Leão / Francisco Lourenço, Rodrigo Roque, José Lopes
 */

// Original code by Gonçalo Leão
// Updated by Francisco Lourenço, Rodrigo Roque, José Lopes

#ifndef DA_TP_CLASSES_GRAPH
#define DA_TP_CLASSES_GRAPH

#include <iostream>
#include <vector>
#include <queue>
#include <limits>
#include <algorithm>
#include "../data_structures/MutablePriorityQueue.h" // not needed for now

template <class T>
class Edge;

#define INF std::numeric_limits<double>::max()

/************************* Vertex  **************************/

/**
 * @brief Represents a vertex (node) in the graph.
 *
 * Stores the vertex info value, outgoing/incoming edge lists, and a set of
 * auxiliary fields used by graph algorithms (BFS visited flag, DFS/Tarjan
 * low-link numbers, Dijkstra distance, topological sort indegree, etc.).
 *
 * @tparam T Type of the info value stored in each vertex.
 */
template <class T>
class Vertex {
public:
    Vertex(T in); ///< @brief Constructs a vertex with the given info value. @complexity O(1)
    bool operator<(Vertex<T> & vertex) const; ///< @brief Comparison by `dist` field, required by MutablePriorityQueue.

    T getInfo() const;                              ///< @brief Returns the vertex info. @complexity O(1)
    std::vector<Edge<T> *> getAdj() const;          ///< @brief Returns the list of outgoing edges. @complexity O(1)
    bool isVisited() const;                         ///< @brief Returns the visited flag. @complexity O(1)
    bool isProcessing() const;                      ///< @brief Returns the processing flag (DFS cycle detection). @complexity O(1)
    unsigned int getIndegree() const;               ///< @brief Returns the indegree (for topological sort). @complexity O(1)
    double getDist() const;                         ///< @brief Returns the distance estimate (Dijkstra / Prim). @complexity O(1)
    Edge<T> *getPath() const;                       ///< @brief Returns the predecessor edge on the shortest-path tree. @complexity O(1)
    std::vector<Edge<T> *> getIncoming() const;     ///< @brief Returns the list of incoming edges. @complexity O(1)

    void setInfo(T info);                           ///< @brief Sets the vertex info. @complexity O(1)
    void setVisited(bool visited);                  ///< @brief Sets the visited flag. @complexity O(1)
    void setProcessing(bool processing);            ///< @brief Sets the processing flag. @complexity O(1)

    int getLow() const;                             ///< @brief Returns the Tarjan low-link value. @complexity O(1)
    void setLow(int value);                         ///< @brief Sets the Tarjan low-link value. @complexity O(1)
    int getNum() const;                             ///< @brief Returns the Tarjan discovery number. @complexity O(1)
    void setNum(int value);                         ///< @brief Sets the Tarjan discovery number. @complexity O(1)

    void setIndegree(unsigned int indegree);        ///< @brief Sets the indegree counter. @complexity O(1)
    void setDist(double dist);                      ///< @brief Sets the distance estimate. @complexity O(1)
    void setPath(Edge<T> *path);                    ///< @brief Sets the predecessor edge. @complexity O(1)

    /**
     * @brief Adds an outgoing edge from this vertex to @p dest with weight @p w.
     * @param dest Destination vertex pointer.
     * @param w    Edge weight.
     * @return Pointer to the newly created Edge.
     * @complexity O(1)
     */
    Edge<T> * addEdge(Vertex<T> *dest, double w);

    /**
     * @brief Removes the outgoing edge to the vertex with info @p in.
     * @param in Info of the destination vertex to disconnect.
     * @return `true` if at least one edge was removed, `false` if none found.
     * @complexity O(deg(v)) where deg(v) is the out-degree of this vertex.
     */
    bool removeEdge(T in);

    /**
     * @brief Removes all outgoing edges from this vertex.
     * @complexity O(deg(v))
     */
    void removeOutgoingEdges();

    friend class MutablePriorityQueue<Vertex>;
protected:
    T info;                                ///< Info stored in the vertex (e.g., web ID).
    std::vector<Edge<T> *> adj;            ///< Outgoing edge list.

    // auxiliary fields
    bool visited = false;                  ///< Visited flag used by DFS/BFS.
    bool processing = false;               ///< Processing flag used by isDAG cycle detection.
    int low = -1, num = -1;                ///< Tarjan SCC low-link and discovery numbers.
    unsigned int indegree;                 ///< Indegree counter used by topological sort.
    double dist = 0;                       ///< Distance estimate used by Dijkstra/Prim.
    Edge<T> *path = nullptr;               ///< Predecessor edge on the shortest-path tree.
    std::vector<Edge<T> *> incoming;       ///< Incoming edge list.
    int queueIndex = 0;                    ///< Index used by MutablePriorityQueue.

    /**
     * @brief Deletes an edge object and removes it from the destination's incoming list.
     * @param edge Pointer to the edge to delete.
     * @complexity O(deg_in(dest)) where deg_in is the in-degree of the destination.
     */
    void deleteEdge(Edge<T> *edge);
};

/********************** Edge  ****************************/

/**
 * @brief Represents a directed weighted edge in the graph.
 *
 * Stores a pointer to the destination vertex, the edge weight/capacity,
 * and auxiliary fields for flow algorithms and bidirectional edge pairing.
 *
 * @tparam T Type of the vertex info value.
 */
template <class T>
class Edge {
public:
    Edge(Vertex<T> *orig, Vertex<T> *dest, double w);

    Vertex<T> * getDest() const;    ///< @brief Returns the destination vertex. @complexity O(1)
    double getWeight() const;       ///< @brief Returns the edge weight / capacity. @complexity O(1)
    bool isSelected() const;        ///< @brief Returns the selected flag (used by MST algorithms). @complexity O(1)
    Vertex<T> * getOrig() const;    ///< @brief Returns the origin vertex. @complexity O(1)
    Edge<T> *getReverse() const;    ///< @brief Returns the reverse edge (bidirectional pairing). @complexity O(1)
    double getFlow() const;         ///< @brief Returns the flow value (network flow algorithms). @complexity O(1)

    void setSelected(bool selected);       ///< @brief Sets the selected flag. @complexity O(1)
    void setReverse(Edge<T> *reverse);     ///< @brief Sets the reverse edge pointer. @complexity O(1)
    void setFlow(double flow);             ///< @brief Sets the flow value. @complexity O(1)
protected:
    Vertex<T> * dest;    ///< Destination vertex pointer.
    double weight;       ///< Edge weight (or capacity in flow networks).

    // auxiliary fields
    bool selected = false;       ///< Selected flag for MST / spanning-tree algorithms.
    Vertex<T> *orig;             ///< Origin vertex pointer.
    Edge<T> *reverse = nullptr;  ///< Reverse edge pointer (used for bidirectional edges).
    double flow;                 ///< Flow value for max-flow algorithms.
};

/********************** Graph  ****************************/

/**
 * @brief Templated directed graph using an adjacency-list representation.
 *
 * Vertices are heap-allocated and stored in `vertexSet`. Edges are heap-
 * allocated inside each Vertex's `adj` list. The graph owns all vertex and
 * edge memory; the destructor frees it.
 *
 * In this project the graph is instantiated as `Graph<int>` where the int
 * is the web ID (see GraphBuilder).
 *
 * @tparam T Type of the info value stored in each vertex.
 */
template <class T>
class Graph {
public:
    ~Graph();

    /**
     * @brief Finds and returns the vertex whose info equals @p in.
     * @param in Info value to search for.
     * @return Pointer to the matching Vertex, or `nullptr` if not found.
     * @complexity O(V) where V = number of vertices.
     */
    Vertex<T> *findVertex(const T &in) const;

    /**
     * @brief Adds a new vertex with info @p in to the graph.
     * @param in Info value for the new vertex.
     * @return `true` on success; `false` if a vertex with that info already exists.
     * @complexity O(V) due to the duplicate check via findVertex().
     */
    bool addVertex(const T &in);

    /**
     * @brief Removes the vertex with info @p in and all its incident edges.
     * @param in Info of the vertex to remove.
     * @return `true` if the vertex was found and removed; `false` otherwise.
     * @complexity O(V + E) — must remove all incoming edges from neighbours.
     */
    bool removeVertex(const T &in);

    /**
     * @brief Adds a directed edge from @p sourc to @p dest with weight @p w.
     * @return `true` on success; `false` if either endpoint does not exist.
     * @complexity O(V) for the two findVertex() calls.
     */
    bool addEdge(const T &sourc, const T &dest, double w);

    /**
     * @brief Removes the directed edge from @p source to @p dest.
     * @return `true` if the edge existed and was removed; `false` otherwise.
     * @complexity O(V + deg(source)).
     */
    bool removeEdge(const T &source, const T &dest);

    /**
     * @brief Adds a bidirectional (undirected) edge between @p sourc and @p dest.
     *
     * Creates two directed edges and links them via their `reverse` pointers.
     *
     * @return `true` on success; `false` if either endpoint does not exist.
     * @complexity O(V) for the two findVertex() calls.
     */
    bool addBidirectionalEdge(const T &sourc, const T &dest, double w);

    /**
     * @brief Returns the number of vertices in the graph.
     * @complexity O(1)
     */
    int getNumVertex() const;

    /**
     * @brief Returns the full vertex set.
     * @return Reference to the internal vector of vertex pointers.
     * @complexity O(1)
     */
    std::vector<Vertex<T> *> getVertexSet() const;


protected:
    std::vector<Vertex<T> *> vertexSet;  ///< All vertices owned by this graph.

    double ** distMatrix = nullptr;      ///< Distance matrix for Floyd-Warshall.
    int **pathMatrix = nullptr;          ///< Path matrix for Floyd-Warshall.

    /**
     * @brief Returns the index of the vertex with info @p in in vertexSet.
     * @return Index, or -1 if not found.
     * @complexity O(V)
     */
    int findVertexIdx(const T &in) const;
};

/**
 * @brief Frees a 2-D int matrix allocated as `int**`.
 * @param m Pointer to the matrix.
 * @param n Number of rows.
 * @complexity O(n)
 */
void deleteMatrix(int **m, int n);

/**
 * @brief Frees a 2-D double matrix allocated as `double**`.
 * @param m Pointer to the matrix.
 * @param n Number of rows.
 * @complexity O(n)
 */
void deleteMatrix(double **m, int n);


/************************* Vertex  **************************/

template <class T>
Vertex<T>::Vertex(T in): info(in) {}
/*
 * Auxiliary function to add an outgoing edge to a vertex (this),
 * with a given destination vertex (d) and edge weight (w).
 */
template <class T>
Edge<T> * Vertex<T>::addEdge(Vertex<T> *d, double w) {
    auto newEdge = new Edge<T>(this, d, w);
    adj.push_back(newEdge);
    d->incoming.push_back(newEdge);
    return newEdge;
}

/*
 * Auxiliary function to remove an outgoing edge (with a given destination (d))
 * from a vertex (this).
 * Returns true if successful, and false if such edge does not exist.
 */
template <class T>
bool Vertex<T>::removeEdge(T in) {
    bool removedEdge = false;
    auto it = adj.begin();
    while (it != adj.end()) {
        Edge<T> *edge = *it;
        Vertex<T> *dest = edge->getDest();
        if (dest->getInfo() == in) {
            it = adj.erase(it);
            deleteEdge(edge);
            removedEdge = true; // allows for multiple edges to connect the same pair of vertices (multigraph)
        }
        else {
            it++;
        }
    }
    return removedEdge;
}

/*
 * Auxiliary function to remove an outgoing edge of a vertex.
 */
template <class T>
void Vertex<T>::removeOutgoingEdges() {
    auto it = adj.begin();
    while (it != adj.end()) {
        Edge<T> *edge = *it;
        it = adj.erase(it);
        deleteEdge(edge);
    }
}

template <class T>
bool Vertex<T>::operator<(Vertex<T> & vertex) const {
    return this->dist < vertex.dist;
}

template <class T>
T Vertex<T>::getInfo() const {
    return this->info;
}

template <class T>
int Vertex<T>::getLow() const {
    return this->low;
}

template <class T>
void Vertex<T>::setLow(int value) {
    this->low = value;
}

template <class T>
int Vertex<T>::getNum() const {
    return this->num;
}

template <class T>
void Vertex<T>::setNum(int value) {
    this->num = value;
}

template <class T>
std::vector<Edge<T>*> Vertex<T>::getAdj() const {
    return this->adj;
}

template <class T>
bool Vertex<T>::isVisited() const {
    return this->visited;
}

template <class T>
bool Vertex<T>::isProcessing() const {
    return this->processing;
}

template <class T>
unsigned int Vertex<T>::getIndegree() const {
    return this->indegree;
}

template <class T>
double Vertex<T>::getDist() const {
    return this->dist;
}

template <class T>
Edge<T> *Vertex<T>::getPath() const {
    return this->path;
}

template <class T>
std::vector<Edge<T> *> Vertex<T>::getIncoming() const {
    return this->incoming;
}

template <class T>
void Vertex<T>::setInfo(T in) {
    this->info = in;
}

template <class T>
void Vertex<T>::setVisited(bool visited) {
    this->visited = visited;
}

template <class T>
void Vertex<T>::setProcessing(bool processing) {
    this->processing = processing;
}

template <class T>
void Vertex<T>::setIndegree(unsigned int indegree) {
    this->indegree = indegree;
}

template <class T>
void Vertex<T>::setDist(double dist) {
    this->dist = dist;
}

template <class T>
void Vertex<T>::setPath(Edge<T> *path) {
    this->path = path;
}

template <class T>
void Vertex<T>::deleteEdge(Edge<T> *edge) {
    Vertex<T> *dest = edge->getDest();
    // Remove the corresponding edge from the incoming list
    auto it = dest->incoming.begin();
    while (it != dest->incoming.end()) {
        if ((*it)->getOrig()->getInfo() == info) {
            it = dest->incoming.erase(it);
        }
        else {
            it++;
        }
    }
    delete edge;
}

/********************** Edge  ****************************/

template <class T>
Edge<T>::Edge(Vertex<T> *orig, Vertex<T> *dest, double w): orig(orig), dest(dest), weight(w) {}

template <class T>
Vertex<T> * Edge<T>::getDest() const {
    return this->dest;
}

template <class T>
double Edge<T>::getWeight() const {
    return this->weight;
}

template <class T>
Vertex<T> * Edge<T>::getOrig() const {
    return this->orig;
}

template <class T>
Edge<T> *Edge<T>::getReverse() const {
    return this->reverse;
}

template <class T>
bool Edge<T>::isSelected() const {
    return this->selected;
}

template <class T>
double Edge<T>::getFlow() const {
    return flow;
}

template <class T>
void Edge<T>::setSelected(bool selected) {
    this->selected = selected;
}

template <class T>
void Edge<T>::setReverse(Edge<T> *reverse) {
    this->reverse = reverse;
}

template <class T>
void Edge<T>::setFlow(double flow) {
    this->flow = flow;
}

/********************** Graph  ****************************/

template <class T>
int Graph<T>::getNumVertex() const {
    return vertexSet.size();
}

template <class T>
std::vector<Vertex<T> *> Graph<T>::getVertexSet() const {
    return vertexSet;
}

/*
 * Auxiliary function to find a vertex with a given content.
 */
template <class T>
Vertex<T> * Graph<T>::findVertex(const T &in) const {
    for (auto v : vertexSet)
        if (v->getInfo() == in)
            return v;
    return nullptr;
}

/*
 * Finds the index of the vertex with a given content.
 */
template <class T>
int Graph<T>::findVertexIdx(const T &in) const {
    for (unsigned i = 0; i < vertexSet.size(); i++)
        if (vertexSet[i]->getInfo() == in)
            return i;
    return -1;
}
/*
 *  Adds a vertex with a given content or info (in) to a graph (this).
 *  Returns true if successful, and false if a vertex with that content already exists.
 */
template <class T>
bool Graph<T>::addVertex(const T &in) {
    if (findVertex(in) != nullptr)
        return false;
    vertexSet.push_back(new Vertex<T>(in));
    return true;
}

/*
 *  Removes a vertex with a given content (in) from a graph (this), and
 *  all outgoing and incoming edges.
 *  Returns true if successful, and false if such vertex does not exist.
 */
template <class T>
bool Graph<T>::removeVertex(const T &in) {
    for (auto it = vertexSet.begin(); it != vertexSet.end(); it++) {
        if ((*it)->getInfo() == in) {
            auto v = *it;
            v->removeOutgoingEdges();
            for (auto u : vertexSet) {
                u->removeEdge(v->getInfo());
            }
            vertexSet.erase(it);
            delete v;
            return true;
        }
    }
    return false;
}

/*
 * Adds an edge to a graph (this), given the contents of the source and
 * destination vertices and the edge weight (w).
 * Returns true if successful, and false if the source or destination vertex does not exist.
 */
template <class T>
bool Graph<T>::addEdge(const T &sourc, const T &dest, double w) {
    auto v1 = findVertex(sourc);
    auto v2 = findVertex(dest);
    if (v1 == nullptr || v2 == nullptr)
        return false;
    v1->addEdge(v2, w);
    return true;
}

/*
 * Removes an edge from a graph (this).
 * The edge is identified by the source (sourc) and destination (dest) contents.
 * Returns true if successful, and false if such edge does not exist.
 */
template <class T>
bool Graph<T>::removeEdge(const T &sourc, const T &dest) {
    Vertex<T> * srcVertex = findVertex(sourc);
    if (srcVertex == nullptr) {
        return false;
    }
    return srcVertex->removeEdge(dest);
}

template <class T>
bool Graph<T>::addBidirectionalEdge(const T &sourc, const T &dest, double w) {
    auto v1 = findVertex(sourc);
    auto v2 = findVertex(dest);
    if (v1 == nullptr || v2 == nullptr)
        return false;
    auto e1 = v1->addEdge(v2, w);
    auto e2 = v2->addEdge(v1, w);
    e1->setReverse(e2);
    e2->setReverse(e1);
    return true;
}

inline void deleteMatrix(int **m, int n) {
    if (m != nullptr) {
        for (int i = 0; i < n; i++)
            if (m[i] != nullptr)
                delete [] m[i];
        delete [] m;
    }
}

inline void deleteMatrix(double **m, int n) {
    if (m != nullptr) {
        for (int i = 0; i < n; i++)
            if (m[i] != nullptr)
                delete [] m[i];
        delete [] m;
    }
}

template <class T>
Graph<T>::~Graph() {
    deleteMatrix(distMatrix, vertexSet.size());
    deleteMatrix(pathMatrix, vertexSet.size());
}

#endif /* DA_TP_CLASSES_GRAPH */