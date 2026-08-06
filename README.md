# Updated summary 
This project served it's goal to get a practical knowledge on related low level concepts for me, and will not get future features or not gonna be maintained.

# No framework userspace NIC driver from scratch
This repository contains a userspace inlined NIC driver experiment, with the architecture detailed in `Architectural design principles`_. 
Although the architecture is designed for mlx5 driver, proof of concept driver is leveraging 82599.

# Installation

## Prerequisites
*    **2 x 2 MB hugepage**
add via command:
```terminal

    echo x > /sys/devices/system/node/node0/hugepages/hugepages-2048kB/nr_hugepages
```

Since 2 hugepage is enough for this driver, already used hugepage count should be checked via Cat command.
In NUMA, the path may change.

-    82599 Chipset NIC
-    Cmake 3.10 or higher
-    zlib1g for hdr histogram
-    Any C compiler that supports C99, which was released on 1999.
Installation commands can be found on Github Matrix Build workflow.

Build from source 
```terminal 

    git clone https://github.com/0xfka/ixgbe-userspace-poc

    cd ixgbe-userspace-poc
    # For debug mode: 
    cmake -B build -DDEBUG_MODE=ON && cmake --build build 
    # OFF: 
    cmake -B build -DDEBUG_MODE=OFF && cmake --build build 
    # As stated in CmakeLists.txt, "# At past, I was used printf in some parts to prove the logic works as expected".
    # Debug mode is used to enable/disable that.
```
Building on popular distros are tested via Github Actions. If you can see a green tick on the repository, chances on you probably doesn't encounter any problems.

# Why 82599:

# Direct Register Manipulation
Instead of sending commands to a "mailbox[1]", direct register manipulation is considered more transparent when it comes to understanding & optimizing at a low level.
As a trade-off, while direct register manipulation may have slightly higher latency than mailbox commands, 82599 is preferred because it's a better target to learn about hardware, without any abstractions. 

# Defined hardware behavior & Transparent documentation
82599 Datasheet is crystal clear, and potential edge cases are documented over the years. Trying something similar on a modern NIC may ends up reverse engineering it from it's linux kernel driver. 

# Simpler Internal Logic
As an ASIC released in July 2011, of course it's simpler than modern NIC ASICs. When it comes to architecture, lean & mean is preferred. Except for a few features(e.g, WQE inlining on Mellanox), many of the offered features are not required for testing inlining an application logic and zero copy networking. 

# Architectural design principles

## Zero copy networking 
At that same 2 mb hugepage, the coming packets are edited and re sent as an answer, even if it requires writing all the bytes, because it's pricey to copy memory or allocate memory. 

# Inlining Workload with Driver

This section is one of the main ideas I wondered before starting this project. Even if it is hard to maintain or portability is mostly ignored, this project serves it's purpose as a research. 
Also the NIC driver never interacts with kernel after initializing: 
.. code-block:: console

    5.421540387 seconds time elapsed

    5.416721000 seconds user
    0.002252000 seconds sys

Contributing
============
Github Pull requests and/or issues section(s) may be used for contributing/feature request/questions or more.
Environment setup script handles pre-commit and blame for now, and may more in future. 

.. code-block:: console

    # Yes, that's all. All the configurations made is project-wide and will not affect your configurations.
    ./setup-env.sh
