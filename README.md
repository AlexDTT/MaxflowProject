# Scientific Conference Organization Tool

## Design of Algorithms (DA) - Spring 2026
**Course:** Analysis and Synthesis of Algorithms (L.EIC016)  
**Project:** Programming Project I - Scientific Conference Organization Tool  

## About the Project
This project implements an organizational tool to assist a scientific committee in assigning article submissions to a set of reviewers. By modeling the **Review Assignment Problem** as a **Maximum Flow** network problem (using algorithms like Edmonds-Karp), the tool determines optimal review assignments while respecting several constraints:
- Minimum number of reviews per submission.
- Maximum number of reviews a reviewer is willing to perform.
- Matching submissions with reviewers based on their Primary (and potentially Secondary) domains of expertise.

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
- <b class="tab-title">Interactive Mode</b> Starts a terminal menu prompting the user to select tasks step by step.
  ```bash
  ./main
  ```

</div>

## Generated Graph Example
This diagram illustrates the bipartite matching between **Submissions** and **Reviewers** using a super-source and super-sink. The edges highlighted in blue represent the final assigned network flows.

<div class="interactive_dotgraph">

\dotfile dox/example_graph.dot

</div>

<br>
<div class="section_buttons">

|                            Next |
|--------------------------------:|
|  [AssignmentLogic](@ref AssignmentLogic) |

</div>
