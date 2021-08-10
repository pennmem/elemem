.. sectnum::

#############
Architecture 
#############
This is the architecture document for Elemem

.. contents:: **Table of Contents**
    :depth: 2

*************
Overview
*************
There are 

*************
Important Code Concepts
*************
There are some very important structures within the code that are critical for all contributers to understand. They are listed below

=============
Tasks
=============
| Tasks are objects that hold an object and a (handler) method.
| When a task is run, it runs the stored method in the context of the stored object.
| 
| There are 3 types of tasks:
#. *TaskCaller*
#. *TaskBlocker*
#. *TaskGetter*

| A *TaskCaller* is an object that acts as a async thread function with no return value. This can be used for many things, such as a callback function.
| A *TaskBlocker* is an object that acts as a sync thread function with no return value. Note that it blocks (waits for the completion of the called handler). 
| A *TaskGetter* is just like a *TaskBlocker* except that is does have a return value.
| 
| There is NO async task that returns a value. It has been decided that futures/promises are not what we want (they do not cleanly fit the event-based message passing arch). If you hit this use case, then restructure your code to register a *TaskCaller* on an event message from the result of the other task.
| 
| NOTES:
* The *TaskHandler()* is a convenient macro that creates a *TaskCaller* object from a method using the context of the class it is in.
* Treat tasks with all the respect of a normal thread.
    * Ex: Do not edit mutable global/shared variables from a *Task* without a critical section.
* Prefer using *TaskCaller* over *TaskBlocker* and *TaskGetter*

=============
Handler
=============
Everything is connected through the *Handler* class. This is a bit monolithic, but is allows the objects to be held in one consistent object. 

*************
Data Formats
*************
These are the important data formats throughout the project. The technical information about them can be found in their Doxygen documentation.

=============
EEGData
=============
This holds the EEG Data that can be passed around the project.
Found in EEGData.h

=============
CSStimChannel
=============
This holds the stimulation configuration for each channel.
Found in ChannelConf.h

=============
ExpEvent
=============
This holds the information for a stimuation event.
Found in ExpEvent.h

=============
OPSSpecs
=============
This holds the experiment configuration parameters for the OPS experiment (such as the number of trials).
Found in OPSSpecs.h

*************
Acronyms and Terms
*************
Below are the common acronyms and terms used in this project

=============
Acronyms
=============
* EEG = Electroencephalogram
* OPS = Open-loop Parameter Search
* HDF5 = Hierarchical Data Format 5
* EDF = European Data Format
* RC = RC Library (https://rcolyer.net/RCLib/)

=============
Terms
=============
* Cerebus = Blackrock NeuroPort EEG acquisition device
* CereStim = Blackrock brain stimulator
* CereLink = Blackrock Cerebus API

