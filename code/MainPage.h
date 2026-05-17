/**
 * @mainpage Register Allocation Tool
 *
 * @section intro Introduction
 * This project implements a system for compiler register allocation,
 * focusing on the automatic assignment of physical registers to program variables
 * using interference graph coloring techniques.
 *
 * @section problem Problem Description
 * Each program variable has a set of live ranges that define when it is in use.
 * A limited number of physical registers (K) are available, and two variables
 * that are simultaneously live cannot share the same register. When the
 * interference graph is not K-colorable, variables must be spilled to memory
 * or split into smaller webs.
 *
 * @section approach Approach
 * The problem is modeled as a graph coloring problem where:
 * - **Webs** (maximal connected live-range components) become graph nodes
 * - Two webs that interfere connect with an edge
 * - K-coloring assigns physical registers; uncolored nodes are spilled
 *
 * Four allocation strategies are provided:
 * - **Basic Coloring** — Greedy simplification (Chaitin/Kempe) heuristic
 * - **Spilling Algorithm** — Chaitin's min(cost/degree) spill selection
 * - **Splitting Algorithm** — Iterative web splitting to reduce interference
 * - **Free Algorithm** — Kempe Chain swaps to rescue stuck nodes before spilling
 *
 * @section features Features
 * - Automatic register assignment with graceful spill-to-memory fallback
 * - Interactive mode (CLI) for step-by-step pipeline visualization
 * - Batch mode for automated execution on multiple inputs
 * - Full time complexity analysis documented for every algorithm
 *
 * @section authors Authors
 * - **Francisco Lourenço** - up202404899
 * - **Rodrigo Roque** – up202403623
 * - **José Lopes** - up202406360
 *
 * @section date Date
 * 17/05/2026
 */
