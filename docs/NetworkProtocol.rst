.. sectnum::

#############
Network Protocol & Logged Events
#############

This is the network protocol document for Elemem

.. contents:: **Table of Contents**
    :depth: 2

*************
Overview
*************

This is adapted from the protocol that was used by Ramulator to synchronize stimulation with task phases as well as extensions needed for additional functionality in Elemem.
Communication on the task side is handled by sending JSON strings over a tcp connection to, by default, 192.168.137.1 port 8889. 

*************
Network Protocol
*************

This section contains the key and the messsages

=============
Important Notes
=============

Important notes about the networking system

* Return communication times out after 1000ms.

=============
Key
=============

The key information for understanding the protocol below

* Each message sent from the task side has a numerically increasing id number (unsigned 64-bit)

* Host (Elemem) responses should carry the id of the message they are responding to.

* The StatusPanel flag indicates that it changes the status panel

=============
Required Messages
=============

These messages are required for Elemem to function 

CONNECTED:
Message: {“type”: “CONNECTED”, “id”: 42, “time”: <float>}
Response: {“type”: “CONNECTED_OK”, “id”: 42, “time”: <float>}

CONFIGURE:
Message: {“type”: “CONFIGURE”, “data”: {“stim_mode”: “open”, “experiment”: “RepFR2”, “subject”: “R1999J”}, “id”: 42, “time”: <float>}
Responses:
{“type”: “CONFIGURE_OK”, “id”: 42, “time”: <float>}
{“type”: “CONFIGURE_ERROR”, “data”: {“error”: “What went wrong”}, “id”: 42, “time”: <float>}
Note:  Experiments using STIMSELECT tags should include optional data entry “tags”: [<string>, <string>, ...]

READY:
Message: {“type”: “READY”, “data”: {}, “id”: 42, “time”: <float>}
Response: {“type”: “START”, “data”: {}, “id”: 42, “time”: <float>}

HEARTBEAT:
Message: {“type”: “HEARTBEAT”, “data”: {“count”: 27}, “id”: 42, “time”: <float>}
Response: {“type”: “HEARTBEAT_OK”, “data”: {“count”: 27}, “id”: 42, “time”: <float>}
Note: After CONFIGURE_OK received, 20 HEARTBEATs sent at 50ms intervals.  Task side logs calculated average and max latency, and raises notification window if max greater than 20ms.  “data”: “count” increases with each heartbeat.  Sent once per second.  8 missed responses stops experiment.

=============
Handled Messages
=============

These are messages that Elemem does something as a result of receiving them

EXIT:
Message: {“type”: “EXIT”, “data”: {}, “id”: 42, “time”: <float>}
Response: None

TRIAL:
Message: {“type”: “TRIAL”, “data”: {“trial”: [int], “stim”:[bool]}, “id”: 42, “time”: <float>}
Response: None	

TRIALEND:
Message: {“type”: “TRIALEND”, “data”: {}, “id”: 42, “time”: <float>}
Reponse: None

STIMSELECT:
Message: {“type”: “STIMSELECT”, “data”: {“tag”: <string>}, “id”: 42, “time”: <float>}
Response: None
Note: Selects the pre-approved stim configuration matching the tag for subsequent stim events.

STIM:
Message: {“type”: “STIM”, “data”: {}, “id”: 42, “time”: <float>}
Response: None
Note: This triggers one open-loop stim event.  Synchronized stimulation during word presentation can instead be triggered by the WORD event with “data”:{“stim”:true}.

CLSTIM:
Message: {“type”: “CLSTIM”, “data”: {“classifyms”: 1366}, “id”: 42, “time”: <float>}
Response: None
Note: This initiates a closed-loop classification epoch for the duration in milliseconds specified by classifyms.  Stimulation is initiated following this duration as soon as processing is completed if the classification result is below the threshold, typically 0.5.

CLSHAM:
Message: {“type”: “CLSHAM”, “data”: {“classifyms”: 1366}, “id”: 42, “time”: <float>}
Response: None
Note: This initiates a closed-loop classification epoch for the duration in milliseconds specified by classifyms.  This is identical to CLSTIM except that no stimulation is performed, and instead an event is simply logged reporting whether or not stim would have been performed.

CLNORMALIZE:
Message: {“type”: “CLNORMALIZE”, “data”: {“ classifyms”: 1366}, “id”: 42, “time”: <float>}
Response: None
Note: This initiates a closed-loop normalization update epoch for the duration in milliseconds specified by classifyms.

WORD:
Message: {“type”: “WORD”, “data”: {“word”: <string>, “serialpos”: [int], “stim”:[bool]}, “id”: 42, “time”: <float>}
Response: None
StatusPanel

SESSION:
Message: {“type”: “SESSION”, “data”: {“session”: [int]}, “id”: 42, “time”: <float>}
Response: None
StatusPanel

REST:
Message: {“type”: “REST”, “data”: {}, “id”: 42, “time”: <float>}
Response: None
StatusPanel

ORIENT (Orientation Cross):
Message: {“type”: “ORIENT”, “data”: {}, “id”: 42, “time”: <float>}
Response: None
StatusPanel

COUNTDOWN:
Message: {“type”: “COUNTDOWN”, “data”: {}, “id”: 42, “time”: <float>}
Response: None
StatusPanel

WAITING:
Message: {“type”: “WAITING”, “data”: {}, “id”: 42, “time”: <float>}
Response: None
Note: Used when waiting on user input
StatusPanel
NOT USED

DISTRACT:
Message: {“type”: “DISTRACT”, “data”: {}, “id”: 42, “time”: <float>}
Response: None
StatusPanel

RECALL:
Message: {“type”: “RECALL”, “data”: {“duration”: <float>}, “id”: 42, “time”: <float>}
Response: None
StatusPanel

INSTRUCT:
Message: {“type”: “INSTRUCT”, “data”: {}, “id”: 42, “time”: <float>}
Response: None
StatusPanel

MATH:
Message: {“type”: “MATH”, “data”: {“problem”: <string>, “response”: <string>, “response_time_ms”: [int], “correct”: [bool]}, “id”: 42, “time”: <float>}
Response: None
StatusPanel

SYNC:
Message: {“type”: “SYNC”, “data”: {}, “id”: 42, “time”: <float>}
Response: None
StatusPanel

[NOT IMPLEMENTED] ISI (Inter-Stimulus Interval):
Message: {“type”: “ISI”, “data”: {“duration”: <float>}, “id”: 42, “time”: <float>}
Response: None
StatusPanel

[NOT IMPLEMENTED] VOCALIZATION:
Message: {“type”: “VOCALIZATION”, “data”: {}, “id”: 42, “time”: <float>}
Response: None
StatusPanel

[NOT IMPLEMENTED] RECALL:
Message: {“type”: “RECALL”, “data”: {}, “id”: 42, “time”: <float>}
Response: None
StatusPanel

*************
Event Logging
*************

Elemem logs all messages from the network protocol and the following listed events.

=============
Logged Messages
=============

ELEMEM:
Message: {“type”: “ELEMEM”, “data”: {“version”: <string>}, “id”: 0, “time”: <float>}
Note: version is the date time string corresponding to the build time, and matches the version displayed under Help/About inside of Elemem.

STIMMING:
Message: {“type”: “STIMMING”, “data”: {“electrode_pos”: [uint], “electrode_neg”: [uint], “amplitude”: <float>, “frequency”: <float>, “duration”: <float>}, “time”: <float>}
Note: electrode_pos and electrode_neg are integer channel numbers, 0 indexed.  Units for the other values are amplitude:uA, frequency:Hz, duration:us.

EEGSTART:
Message: {“type”: “EEGSTART”, “data”: {“sub_dir”: <string>}, “id”: 0, “time”: <float>}
Note: sub_dir is the session directory name on Elemem (without full path information), for example, “R1999J_2021-06-14_15-47-29”.  The time value from this is for converting the Elemem system time to the EEG file offsets.

