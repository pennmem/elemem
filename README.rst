#############
Elemem
#############
Elemem, short for "elevate memory", is the software for System 4, the Computational Memory Lab's fourth generation system for EEG acquisition and brain stimulation.  This software is used for cognitive electrophysiology experiments, alone or with behavioral task experiments, to study the fundamentals of human memory and to explore ways to improve human memory.

.. contents:: **Table of Contents**
    :depth: 2

******************
Build Instructions
******************
#. Clone the repository.
#. IDE build option:
    #. Install `QtCreator 5.12.11 <https://www.qt.io/offline-installers>`.
    #. Open the project in QtCreator by selecting the *CMakeLists.txt* file.
    #. Select *Build > Run CMake* from the top dropdown menu.
    #. Select *Build > Compile Elemem* from the top dropdown menu.
#. Command line build option:
    .. code:: bash

        export CMAKE_PREFIX_PATH=/usr/local/Cellar/qt\@5/<version> # If on Mac
        cd <the_repository_directory>
        mkdir build
        cd build
        cmake ..
        make
#. Change directory to the *dist* folder of the Elemem repo directory
    .. code:: bash

        $ cd <path_to_elemem_directory>/dist
#. Run the Elemem code
    .. code:: bash

        $ ./Elemem

**********************
How To Use The Program
**********************
#. Load the configuration file from *File > Open Config* in the top dropdown menu
#. Approve the needed parameters
#. Click *Start Experiment*

*********
Platforms
*********
#. Windows is currently the primary platform for full experiment runs, as this is the only platform with upstream driver support for the CereStim stimulator.
#. Linux and MacOS work for EEG Acquisition, and for development and testing using the EEG Simulator mode, EEG Replay mode, and the stimulation simulator.

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
License
*************
Elemem is (c) 2019 by the Computational Memory Lab at the University of Pennsylvania, and licensed as open source under the GPLv3, with the full details in LICENSE.txt.  Multiple included libraries are under their own copyright and compatible license as designated by the license files in their directories or at the tops of individual files.

****************
More Information
****************
Please see the docs folder

