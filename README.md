# Scientific Conference Organization Tool

<img src="logo.png" alt="Maxflow Project T9G2 Logo" width="100">

## Algorithm Design (DA) - Spring 2026
**Course:** Algorithm Design (L.EIC016)  
**Project:** Programming Project I - Scientific Conference Organization Tool  

## About the Project
This project implements an organizational tool to assist a scientific committee in assigning article submissions to a set of reviewers. By modeling the **Review Assignment Problem** as a **Maximum Flow** network problem, the tool determines optimal review assignments while respecting several constraints:
- Minimum number of reviews per submission.
- Maximum number of reviews a reviewer is willing to perform.
- Matching submissions with reviewers based on their Primary (and potentially Secondary) domains of expertise.

Two maximum-flow algorithms are available:
- **Ford-Fulkerson** (DFS-based, default): O(F × E) practical complexity.
- **Edmonds-Karp** (BFS-based): O(V × E²) worst-case complexity, guarantees shortest augmenting paths.

In addition to base assignments, the tool supports **Risk Analysis** scenarios to evaluate how the absence of a reviewer (K=1) might jeopardize the integrity of the review process. 

## Group T9G2 Members
This project was developed by Group T9G2:
- **Alexandre Dinis Alves Teixeira** - [up202403579@up.pt](mailto:up202403579@up.pt)
- **Carlos Francisco de Sousa Ferreira Magalhães Diogo** - [up202404033@up.pt](mailto:up202404033@up.pt)
- **Rodrigo Martins Dias** - [up202404130@up.pt](mailto:up202404130@up.pt)

## Usage
The application provides both a command-line menu interface and a batch mode for automated execution:

<div class="tabbed">

- <b class="tab-title">Batch Mode (Recommended)</b> Automatically parses the input, generates assignments, evaluates risks, and outputs to files.
  ```bash
  ./main -b inputs/main_input.csv output.csv
  ```
  To select the algorithm explicitly (extra argument!):
  ```bash
  ./main -a ff -b inputs/main_input.csv output.csv   # Ford-Fulkerson (DFS)
  ./main -a ek -b inputs/main_input.csv output.csv   # Edmonds-Karp (BFS)
  ```
- <b class="tab-title">Interactive Mode</b> Starts a terminal menu prompting the user to select tasks step by step.
  ```bash
  ./main
  ```
  The algorithm can be selected when choosing **Algorithms > Generate Assignments**.

</div>

## System Architecture & Graph Mapping
The assignment problem is modeled as a Bipartite Graph with a Super-Source and Super-Sink. The capacities of the edges are determined by the application parameters to ensure constraints are strictly met:

<div class="interactive_dotgraph">

\dotfile dox/annotated_architecture.dot "Network Flow Architecture Mapping"

</div>

## Generated Graph Example
Here is a small example illustrating how the program reads a `.csv` configuration and generates the corresponding Max-Flow network. 

**Example Input Data:**
```text
# Scenario: Secondary submission domain only - undercovered submissions
#Submissions
#Id, Title, Authors, E-mail, Primary, Secondary
1, "AI Planning Systems", Alice Brown, alice.brown@gmail.com, 1, 2
2, "Secure Databases", Bob Smith, bob.smith@gmail.com, 3, 2
3, "GPU Scheduling", Carla Costa, carla.costa@gmail.com, 4, 3
4, "Compiler Optimizations", Rui Fernandes, rfernandes@gmail.com, 5, 3
#

#Reviewers
#Id, Name, E-mail, Primary, Secondary
1, Jaqueline N. Chame, jchame@yahoo.com, 2,
2, Mary W. Hall, mhall@hotmail.edu, 2,
3, Daniel Carter, dcarter@gmail.com, 3,
4, Olivia White, owhite@gmail.com, 3, 1

#Parameters
MinReviewsPerSubmission, 2
MaxReviewsPerReviewer, 2
PrimaryReviewerExpertise, 1
SecondaryReviewerExpertise, 0
PrimarySubmissionDomain, 1
SecondarySubmissionDomain, 1

#Control
GenerateAssignments, 2
RiskAnalysis, 0
OutputFileName, "output_dataset12.csv"
```

This diagram illustrates the bipartite matching between **Submissions** and **Reviewers** using a super-source and super-sink. The edges highlighted in blue represent the final assigned network flows.

<div class="interactive_dotgraph">

\dotfile dox/example_graph.dot

</div>

## Additional Documentation
For extended analysis covering risk analysis generalization (K > 1), the general multi-domain
assignment problem, and complexity analysis for both algorithms, see the
@ref extra_docs "Extra Documentation" page.

<br>
<div class="section_buttons">

|                            Next |
|--------------------------------:|
|  [AssignmentLogic](@ref AssignmentLogic) |

</div>
