# Assignment 3 - Distributed File Sharing System

## Overview:

    This assignment requires you to build a group-based file sharing system. Users can share and download files within their respective groups. The system should incorporate advanced features such as parallel downloading, support for fallback multi-trackers, and a custom piece selection algorithm.

## Key Features:

- Group-Based File Sharing:
    - Users can create, join, and manage groups for file sharing.
    - Only administrators have the authority to manage group operations.

- Parallel Downloading:
    - Implement parallel downloading for faster file retrieval.
    - Users can download multiple pieces of a file simultaneously from various peers within their group.

- Fallback Multi-Tracker System:
    - Ensure system reliability with support for multiple trackers.
    - The system should switch to an alternative tracker if one becomes unavailable.

- Custom Piece Selection Algorithm:
    - Optimize file piece distribution with a custom algorithm.
    - Prioritize rare or missing pieces for efficient and fair file sharing.


## How to run

    - Compilation:
    Compile the tracker with:
        g++ tracker.cpp -o tracker -pthread
    Compile the client with: 
        g++ client.cpp -o client -pthread -lcrypto

    - Running the Components:
    Start the tracker: 
        ./tracker tracker_info.txt
    Run the client: 
        ./client <IP>:<PORT> tracker_info.txt
        (for example :  ./client 127.0.0.1:8085 tracker_info.txt)

    - Ensure the port is greater than 1000 and use loopback addresses for IP.


# Commands

- Create User Account:
    - Syntax: create_user <user_id> <passwd>
    - Create a new user account with the provided user ID and password.

- Login:
    - Syntax: login <user_id> <passwd>
    - Log in to your user account with the specified user ID and password.



- Join Group:
    - Syntax: join_group <group_id>
    - Join an existing group with the specified group ID.

- Create Group:
    - Syntax: create_group <group_id>
    - Create a new group with the given group ID.

- Leave Group:
    - Syntax: leave_group <group_id>
    - Leave a group that you are a member of.

- List Pending Join Requests:
    - Syntax: requests list_requests <group_id>
    - List the pending join requests for a specific group.


- Accept Group Joining Request:
    - Syntax: accept_request <group_id> <user_id>
    - Accept a user's request to join a group.

- List All Groups In Network:
    - Syntax: list_groups
    - List all the groups available in the network.

- List All Sharable Files In Group:
    - Syntax: list_files <group_id>
    - List all shareable files within a specific group.

- Upload File:
    - Syntax: upload_file <file_path> <group_id>
    - Upload a file to a specific group.

- Download File:
    - Syntax: download_file <group_id> <file_name> <destination_path>
    - Download a file from a group to a specified destination.

- Logout:
    - Syntax: logout
    - Log out from your user account.

- Show Downloads:
    - Syntax: show_downloads
    - View and manage ongoing downloads, including options to list, pause, resume, and cancel downloads.

- Stop sharing: 
    - Syntax: stop_share <group_id> <file_name>
