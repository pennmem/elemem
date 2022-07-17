#############
Elemem
#############

Elemem, short for "elevate memory", is the software for System 4, the Computational Memory Lab's fourth generation system for EEG acquisition and brain stimulation.  This software is used for cognitive electrophysiology experiments, alone or with behavioral task experiments, to study the fundamentals of human memory and to explore ways to improve human memory.

.. contents:: **Table of Contents**
    :depth: 2
.. section-numbering::

******************
Build Instructions
******************

====================
Clone the repository
====================

.. code:: bash

    git clone git@github.com:pennmem/elemem.git
    git submodule update --init --recursive

====================
Install dependencies
====================

#. Install pkg-config

============
Build Elemem  
============

#. IDE build option:
    
   #. Install `QtCreator 5.12.11 <https://www.qt.io/offline-installers>`.
   #. Open the project in QtCreator by selecting the *CMakeLists.txt* file.
   #. Select *Build > Run CMake* from the top dropdown menu.
   #. Select *Build > Compile Elemem* from the top dropdown menu.
        
#. Command line build option:

   .. code:: bash

      # If on MacOS
      export CMAKE_PREFIX_PATH=<path_to_qt5>
      # If downloaded via brew, the path is usually: /usr/local/Cellar/qt\@5/<version>
            
      # General process
      cd elemem
      mkdir build
      cd build
      cmake ..
      make -j
            
#. Special Situations

   * No Cerebus Hardware

     .. code:: bash

         cd build
         rm -rf CMakeCache.txt CMakeFiles cmake_install.cmake Elemem_autogen Makefile
         cmake -DCEREBUS_HW=OFF ..
         make -j

   * Using CereStim Simulator
        
     .. code:: bash

        cd build
        rm -rf CMakeCache.txt CMakeFiles cmake_install.cmake Elemem_autogen Makefile
        cmake -DCERESTIM_STUB=ON ..
        make -j
                
   * No Cerebus Hardware and Using CereStim Simulator
        
     .. code:: bash

        cd build
        rm -rf CMakeCache.txt CMakeFiles cmake_install.cmake Elemem_autogen Makefile
        cmake -DCEREBUS_HW=OFF -DCERESTIM_STUB=ON ..
        make -j

   * Testing with System 3 classifier (Medtronic)

     .. code:: bash

        cd build
        rm -rf CMakeCache.txt CMakeFiles cmake_install.cmake Elemem_autogen Makefile
        cmake -DTESTING_SYS3_R1384J ..
        make -j

**********************
How To Use The Program
**********************

=====
Setup
=====

#. Open "*dist/sys_config.json*"

#. Set the "*taskcom_ip*" to the IP address of the task computer

#. If using the Cerebus simulator (no Cerebus hardware)

   * Set the "*eeg_system*" to "*CerebusSim*"

#. If using the CereStim simulator

   * Set the "*stim_system*" to "*CereStimSim*"

#. If using the Network Stimulator

   * Set the "*stim_system*" to "*StimNetWorker*"

   * Set the "*stimcom_ip*" to the IP address of the computer running the network stimulator

=========
Launch it
=========

.. code:: bash

    cd dist
    ./Elemem

======
Use it
======

#. Files needed to open an experiment configuration (these should all be in the same folder)

   * Experiment json (specific to subject)

   * Subject montage (MONO csv file)

   * If using bipolar electrodes

     * subject bipolar referencing (BIPOLAR csv file)

   * If using closed loop classification

     * classifier json file
        
#. Connecting to Network Stimulator (if needed)

   #. The network stimulator can be connected any time before clicking "*Start Experiment*"
        
#. Load and Run the experiment

   #. Click "*File > Open Config*", navigate to the experiment json, and select it
   #. Approve the needed parameters (e.g., for stimulation)
   #. Click "*Start Experiment*"

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

