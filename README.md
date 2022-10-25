# honours_project
NB!! Official projet files are stored on the dev branch

# Directory Structure
- Blackhole 
  - This folder contains the two aodv routing protocol class files (aodv-routing-protocolc.cc and aodv-routing-protocol.h)
  - These only contain the blackhole attack implementation
  
 - Detection
   - This folder contains the blackhole attack implementation and the dynamic sequence number threshold0-based solution.
   - Basically, all the files found in the sccr/aodv/model folder are included here
   - The StoredRrep class and header files are included in this folder as well
   
   
 # Running Simulations
In order to run the simulations follow these steps
- Install NS3.36 using bake, as detailed at https://www.nsnam.org/wiki/Installation#Installation_with_Bake
- Build the simulatoe as explaned on the same page
- Download the project files from the dev branch
- copy the files to the respetive folders
- copy the simulation script to the scratch diretory
- run the simulation as follows:
    - ./ns3 run scratch/simulation-script.cc
