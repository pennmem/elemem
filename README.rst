#############
Elemem
#############
This is application used to control stimulation for System 4

.. contents:: **Table of Contents**
 :depth: 2

*************
Setup Instructions
*************
#. Clone the repository
#. Install 'QtCreator 5.12.11 <https://www.qt.io/offline-installers>'_)
#. Open the project in QtCreator by selecting the *CMakeLists.txt* file
#. Select *Build > Run CMake* from the top dropdown menu
#. Open the *General Messages* tab on the bottom and copy the path to the build directory
    * "Build files have been written to: <path_to_build_directory>"
#. Open a terminal and go to the build directory
    * .. code:: bash
   
         $ cd <path_to_build_directory>
#. Compile the code
    * .. code:: bash

         $ make
#. Change directory to the *dist* folder of the Elemem repo directory
    * I recommend doing this in a new terminal
    * .. code:: bash

         $ cd <path_to_elemem_directory>/dist
#. Run the Elemem code
    * .. code:: bash

         $ ./Elemem

*************
How To Use The Program
*************
#. Load the configuration file from *File > Open Config* in the top dropdown menu
#. Approve the needed parameters
#. Click *Start Experiment*

*************
FAQ
*************
#. Who made the name Elemem?
    * Ryan Colyer
#. Who should you ask almost any question about this code to?
    * Ryan Colyer
#. Why is ~0.2% of the repo MATLAB code?
    * Because it came with Cerelink. We didnt write that.

*************
More Information
*************
Please see the docs folder

