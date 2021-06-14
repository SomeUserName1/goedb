# Master Thesis:  Locality Optimization for traversal-based queries on graph databases

Dynamic record locality optimizing storage scheme for graph databases.  

[![Codacy Badge](https://app.codacy.com/project/badge/Grade/db98dfa832514fecb1829fd2aab68728)](https://www.codacy.com/gh/SomeUserName1/master/dashboard?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=SomeUserName1/master&amp;utm_campaign=Badge_Grade)  [![Language grade: C/C++](https://img.shields.io/lgtm/grade/cpp/g/SomeUserName1/master.svg?logo=lgtm&logoWidth=18)](https://lgtm.com/projects/g/SomeUserName1/master/context:cpp) [![codecov](https://codecov.io/gh/SomeUserName1/master/branch/main/graph/badge.svg?token=EHBWYZ8HYP)](https://codecov.io/gh/SomeUserName1/master)  

## Timeline  
<p style="text-align: center;"><font size="20"><progress id="time" value="0" max="1"></progress></font></p>

<font size="1">
- 1st July: Disk-based Impl, Code Comments
- 1st August: Dataset, Random, G-Store, Gedik (?)
- 1st September: Adaptive Method
- 1st October: Submit paper
</font>

## Documentation 

<p style="text-align: center;"><font size="20"><progress id="write" value="2" max="4">Documentation</progress></font></p>

- [x] SRS
- [x] SDD
- [ ] User Guide
- [ ] Code Comments


## Implementation
<p style="text-align: center;"><font size="20"><progress id="file" value="0" max="6">Implementation</progress></font></p>  

#### Possible Features 
  - [ ] Marko-based genereic data structures
  - [ ] Disk-based IO
  - [ ] Cache
  - [ ] Non-consecutive IDs & support deletions
  - [ ] Dynamic Reorganization Impl.
  - [ ] Hop labeling scheme: Use existing impl.

#### Brainstorm
  - Adapt louvain w RCM
  - Access-History Graph

### Coverage
<p style="text-align: center;"><font size="14"><progress id="file" value="3376" max="4092">Coverage</progress></font></p>  

#### Test Cases
  - [ ] Compare reorganization of simulated and disk-based
  
### Future Work
  - [ ] More traversal-based queries
  - [ ] thread-safe BSD-style data structures (to avoid wrapper code) or use C++ or Rust
  - [ ] Transactions/MVCC using git-like transaction logs
  - [ ] Alternative record layouts (nodes + adj list in same file)
  - [ ] Distributed
  - [ ] Multi-Model



## Meeting 1
- Goal: Paper  
	- Intro
	- X
	- Evaluation
	- Conclusion
	
- Deliverables: Test bed for method
	+ Disk-based
	+ G-Store format converter
	+ Schema (?)
	+ Evaluation: Run n% of queries, reorganize, run remaining 1-n%
	
- Keep using Neo4J like Gedik in the very back of our minds

