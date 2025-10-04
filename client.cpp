#include<unordered_map>
#include <thread>
#include<iostream>
#include <vector>
#include <string>
#include <openssl/sha.h>
#include <fcntl.h>
#include <stdarg.h>
#include <sys/time.h>
#include <sys/types.h>
#include <signal.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <netdb.h>
#include <sys/types.h>
#include <pthread.h>
#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>
using namespace std;

int w=1;
int x =1;
int y=1;
int z=1;

#define MAX_CHUNK_SIZE 524288
#define SIZE 32768
#define SA struct sockaddr
int hsize=32;
string FileName, trackerIP, peerIP;
int check_login = 0, check_sha = 1;
uint16_t trackerPort, peerPort;
unordered_map<string, string> file_to_path;
int hsize2=20;

vector<string> splitString(string x, string strs) {
    vector<string> ans;
    size_t position = strs.find(x);
    
    while (position != string::npos) {
        ans.push_back(strs.substr(0, position)); // Add substring to the result
        strs = strs.substr(position + x.length()); // Update 'strs' to remove the processed part
        position = strs.find(x); // Find the next occurrence
    }

    ans.push_back(strs); // Add the remaining part of the string
    // vector<string> res = ans;
    return ans;
}

pair<int, string> getIPAndPortFromFileName(char* fn) {
    int fd = open(fn, O_RDONLY); // Open the file
    if (fd == -1) {
        printf("Cannot open the file\n");
        return make_pair(0, "0.0.0.0"); // Return default values if the file cannot be opened
    }

    char buffer[128];
    ssize_t bytesRead = read(fd, buffer, sizeof(buffer)); 
    close(fd); // Close the file descriptor after reading

    if (bytesRead <= 0) {
        printf("Cannot read from the file\n");
        return make_pair(0, "0.0.0.0"); // Handle read error
    }

    buffer[bytesRead] = '\0'; // Null-terminate the buffer
    vector<string> trackerIP = splitString(":", string(buffer)); // Split the line into IP and port

    if (trackerIP.size() != 2) {
        printf("Invalid format in the file\n");
        return make_pair(0, "0.0.0.0"); // Return default values if format is incorrect
    }

    string ip = trackerIP[0];
    int port = stoi(trackerIP[1]); // Convert port to int
    return make_pair(port, ip); // Return the port and IP address
}

typedef struct peerFileDetails {
    string serverPeerIP;
    string filename;
    long long filesize;
} peerFileDetails;

vector<vector<string>> current_chunks;
vector<string> pieceSha;

typedef struct reqdChunkDetails {
    string serverPeerIP;
    string filename;
    long long chunkNum;
    string destination;
} reqdChunkDetails;

unordered_map<string, vector<int>> chunk_info;
unordered_map<string, string> downloads;
unordered_map<string, unordered_map<string, bool>> upload_list;

long long file_size(char *path) {
    FILE *file_pointer = fopen(path, "rb");
    if (!file_pointer) {
        printf("File not found.\n");
        return -1; // Return early if the file cannot be opened
    }

    fseek(file_pointer, 0, SEEK_END);
    long size = ftell(file_pointer); // Get the file size
    fclose(file_pointer); // Close the file pointer

    return (size >= 0) ? (size + 1) : -1; // Add 1 to the size and return, or return -1 if an error occurred
}

void singlehash(string &hash, const string &stringS) {
    unsigned char array[hsize2];
    if (!SHA1(reinterpret_cast<const unsigned char *>(&stringS[0]), stringS.length(), array)) {
        cout << "Error in hashing" ;
        cout<<"\n";
        return; // Early return if hashing fails
    }

    for (int i = 0; i < hsize2; i++) {
        char current[3];
        sprintf(current, "%02x", array[i] & 0xff);
        hash += string(current);
    }

    hash += "*$*"; // Append delimiter after processing the entire hash
}

void bitvector(long long int high, int last, const string &filename, long long int low) {
    if (last != 0) {
        chunk_info[filename] = vector<int>(high - low + 1, 1); // Initialize with 1s
    } else {
        chunk_info[filename][low] = 1; // Mark specific chunk as available
    }
}

string combinehash(char *file_path) {
    string geth = "";
    long long _file_size = file_size(file_path);
    int hs = SIZE + 1;
    string stringS = "";
    FILE *file_pointer = fopen(file_path, "r");

    if (!file_pointer) {
        cout << "File not found" ;
        cout<<"\n";
        return geth; // Early return if file is not found
    }

    int curret_seg = (_file_size / MAX_CHUNK_SIZE) + 1;
    char poll[hs];

    for (int i = 1; i <= curret_seg; i++) {
        int custom = 0;
        while (true) {
            int curt = fread(poll, 1, min(SIZE - 1, MAX_CHUNK_SIZE - custom), file_pointer);
            if (curt <= 0 || custom >= MAX_CHUNK_SIZE) break; // Exit if read fails or max size reached

            poll[curt] = '\0'; // Null-terminate the buffer
            stringS += poll; // Accumulate data
            custom += curt; // Update custom count
            memset(poll, 0, sizeof(poll)); // Clear the buffer
        }

        singlehash(geth, stringS); // Compute hash for the accumulated string
        stringS.clear(); // Clear string for next segment
    }

    fclose(file_pointer);

    if (geth.length() > 3) {
        geth.erase(geth.length() - 3); // Remove last three characters
    }

    return geth; // Return the combined hash
}

string peer_connection(string scumm, char *PeerP, char *PortI) {
    uint16_t peerPort = stoi(string(PortI));
    struct sockaddr_in current_peer_serv_addr;
    int socket_peer = socket(AF_INET, SOCK_STREAM, 0);

    if (socket_peer < 0) {
        cout<<"\n";
        cout << "Socket creation error" ;
        cout<<"\n";
        return "error";
    }

    x++;
    y++;

    current_peer_serv_addr.sin_family = AF_INET;
    current_peer_serv_addr.sin_port = htons(peerPort);

    z++;
    w++;

    if (inet_pton(AF_INET, PeerP, &current_peer_serv_addr.sin_addr) <= 0) {
    cout << "ERROR: Peer Connection Error(INET): Invalid address/ Address not supported" ;
    cout<<"\n";
    close(socket_peer);
    return "error";
}


    if (connect(socket_peer, (struct sockaddr *)&current_peer_serv_addr, sizeof(current_peer_serv_addr)) < 0) {
        cout << "ERROR" ;
        cout<<"\n";
        perror("Peer Connection Error");
        close(socket_peer);
        return "error";
    }

    string command_xx = splitString("*$*", scumm).front();
    char reply_back[10240] = {0};
    int uvar = strlen(&scumm[0]);

    if (command_xx == "current_path_file" || command_xx == "current_chunk_vector_details" || command_xx == "current_chunk") {
        if (send(socket_peer, &scumm[0], uvar, MSG_NOSIGNAL) == -1) {
            cout << "Error in socket peer in command: " << command_xx ;
            cout<<"\n";
            close(socket_peer);
            return "error";
        }
    }

    x++;
    z++;

    if (command_xx == "current_path_file") {
        if (read(socket_peer, reply_back, sizeof(reply_back)) < 0) {
            cout << "Error in socket reading in current_path" ;
            cout<<"\n";
            close(socket_peer);
            return "error";
        }
        file_to_path[splitString("*$*", scumm).back()] = string(reply_back);
    } 
    else if (command_xx == "current_chunk_vector_details") {
        if (read(socket_peer, reply_back, sizeof(reply_back)) < 0) {
            cout << "Error in socket reading in current_chunk_vector" ;
            cout<<"\n";
            close(socket_peer);
            return "error";
        }
        close(socket_peer);
        return string(reply_back);
    } 
    else if (command_xx == "current_chunk") {
        // Handling current_chunk logic
        vector<string> tik = splitString("*$*", scumm);
        long long int chunkNum = stoll(tik[2]);
        string despath = tik[3];
        char *filepath = &despath[0];
        char current_buff[MAX_CHUNK_SIZE];
        string fuc = "";
        int techn = 0;

        while (techn < MAX_CHUNK_SIZE) {
            int number = read(socket_peer, current_buff, MAX_CHUNK_SIZE - 1);
            if (number <= 0) {
                break;
            }

            current_buff[number] = 0; // Null-terminate the buffer

            int file_descriptor = open(filepath, O_RDWR);
            if (file_descriptor == -1) {
                cout << "Error opening file." ;
                cout<<"\n";
                close(socket_peer);
                return "error";
            }

            off_t position = chunkNum * MAX_CHUNK_SIZE + techn;
            if (lseek(file_descriptor, position, SEEK_SET) == -1) {
                cout << "Error seeking to position." ;
                cout<<"\n";
                close(file_descriptor);
                close(socket_peer);
                return "error";
            }

            ssize_t bytesWritten = write(file_descriptor, current_buff, number);
            if (bytesWritten == -1) {
                cerr << "Error writing to file." ;
                cout<<"\n";
            }

            close(file_descriptor);

            techn += number; // Update bytes received
            fuc += current_buff; // Accumulate data
        }

        y++;
        w++;

        // Compute the hash
        string hash = "";
        singlehash(hash, fuc);
        hash.erase(hash.length() - 3); // Remove last three characters

        // Check hash validity
        if (hash != pieceSha[chunkNum]) {
            check_sha = 0;
        }

        // Update chunk info
        string file_name = splitString("/", string(filepath)).back();
        bitvector(chunkNum, 0, file_name, chunkNum);
    }

    close(socket_peer);
    return "done";
}

int upload_file(int sock, vector<string> input_array) {
    string name = "file";
    char *filepath = &input_array[1][0];
    string filename = splitString("/", string(filepath)).back();
    
    // Check if the file is already uploaded
    if (upload_list[input_array[2]].find(filename) != upload_list[input_array[2]].end()) {
        cout << "File already uploaded" ;
        cout<<"\n";
        if (send(sock, "error", 5, MSG_NOSIGNAL) == -1) {
            cout << "Error in uploading" ;
            cout<<"\n";
            return -1;
        }
        return 0;
    }

    x--;
    y--;
    
    // Mark the file as uploaded
    upload_list[input_array[2]][filename] = true;
    file_to_path[filename] = string(filepath);

    // Generate piecewise hash and file hash
    string piecewiseHash = combinehash(filepath);
    string filehash = "";
    string contents;

    z--;
    w--;

    // Read the file content
    int fd = open(filepath, O_RDONLY);
    if (fd == -1) {
        cout << "Error opening file: " << filepath ;
        cout<<"\n";
        return -1;
    }

    char buffer[4096];
    ssize_t bytesRead;
    while ((bytesRead = read(fd, buffer, sizeof(buffer))) > 0) {
        contents.append(buffer, bytesRead);
    }
    close(fd);

    // Compute SHA256 hash of the file content
    unsigned char current_file_buffer_hash[hsize];
    if (!SHA256(reinterpret_cast<const unsigned char *>(&contents[0]), contents.length(), current_file_buffer_hash)) {
        cout << "Error in hashing" ;
        cout<<"\n";
        return -1;
    }

    // Construct the hash string
    string omhas;
    for (int i = 0; i < hsize; i++) {
        char cur_buff[3];
        sprintf(cur_buff, "%02x", current_file_buffer_hash[i] & 0xff);
        omhas += string(cur_buff);
    }
    filehash = omhas;

    // Prepare file details
    string filesize = to_string(file_size(filepath));
    string fileDetails = string(filepath) + "*$*";
    fileDetails += string(peerIP) + ":" + to_string(peerPort) + "*$*";
    fileDetails += filesize + "*$*";
    fileDetails += filehash + "*$*";
    fileDetails += piecewiseHash;

    x++;
    y++;
    z++;
    w++;

    // Send file details
    if (send(sock, &fileDetails[0], fileDetails.length(), MSG_NOSIGNAL) == -1) {
        cout << "Error in uploading" ;
        cout<<"\n";
        return -1;
    }

    // Read server response
    char reply_back[10240] = {0};
    if (read(sock, reply_back, sizeof(reply_back)) < 0) {
        cout << "Error reading reply" ;
        cout<<"\n";
        return -1;
    }
    cout << reply_back ;
    cout<<"\n";

    // Update the chunk info
    bitvector(stoll(filesize) / MAX_CHUNK_SIZE + 1, 1, filename, 0);
    
    return 0;
}

void thread_func_2(reqdChunkDetails *pr_re) {
    string filename = pr_re->filename;
    vector<string> peerP = splitString(":", pr_re->serverPeerIP);
    long long int chunkNum = pr_re->chunkNum;
    string destination = pr_re->destination;

    // Create command string
    string scumm = "current_chunk*$*" + filename + "*$*" + to_string(chunkNum) + "*$*" + destination;

    // Establish peer connection
    peer_connection(scumm, &peerP[0][0], &peerP[1][0]);
    delete pr_re; // Clean up
}

void thread_func(peerFileDetails *peer_file_details) {
    string scumm = "current_chunk_vector_details*$*" + string(peer_file_details->filename);
    vector<string> serverPeerAddress = splitString(":", string(peer_file_details->serverPeerIP));

    // Get response from peer
    string response = peer_connection(scumm, &serverPeerAddress[0][0], &serverPeerAddress[1][0]);

    // Update current_chunks based on the response
    for (size_t i = 0; i < current_chunks.size(); i++) {
        if (response[i] == '1') {
            current_chunks[i].push_back(string(peer_file_details->serverPeerIP));
        }
    }

    delete peer_file_details; // Clean up
}

void piecewiseAlgo(vector<string> input_array, vector<string> peers) {
    vector<thread> threads;
    string fname;

    long long int filesize = stoll(peers.back());
    peers.pop_back();
    long long int curret_seg = (filesize / MAX_CHUNK_SIZE) + 1;
    current_chunks.clear();
    current_chunks.resize(curret_seg);

    fname = input_array[0];
    int var2 = peers.size();

    x=y+1;
    y=x;

    // Launch threads for peer connections
    for (size_t i = 0; i < var2; i++) {
        peerFileDetails *pf = new peerFileDetails();
        pf->filesize = curret_seg;
        pf->serverPeerIP = peers[i];
        pf->filename = input_array[2];

        threads.emplace_back(thread(thread_func, pf));
    }

    // Wait for all threads to complete
    for (auto &t : threads) {
        if (t.joinable()) {
            t.join();
        }
    }

    z=x;
    w=y;

    // Check if all chunks are available
    for (size_t i = 0; i < current_chunks.size(); i++) {
        if (current_chunks[i].empty()) {
            cout << "All parts of the file are not available." ;
            cout<<"\n";
            return;
        }
    }

    threads.clear();
    srand(static_cast<unsigned>(time(0)));
    long long int rec_segments = 0;

    // Prepare the destination file path
    string des_path = input_array[3] + "/" + input_array[2];
    FILE *file_pointer = fopen(des_path.c_str(), "w");
    
    if (file_pointer == nullptr) {
        cout << "The file already exists." ;
        cout<<"\n";
        return;
    }
    fclose(file_pointer);

    // Create an empty file
    string ss(filesize, '\0');
    int t = 0;
    string fullFilePath = des_path + to_string(t);
    
    int file_descriptor = open(fullFilePath.c_str(), O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
    if (file_descriptor < 0) {
        cout << "Error opening file" ;
        cout<<"\n";
    } else {
        write(file_descriptor, ss.c_str(), ss.size());
        close(file_descriptor);
    }

    chunk_info[input_array[2]].resize(curret_seg, 0);
    check_sha = 1;
    int reqst = 0;
    string peerToGetFilepath;
    vector<int> tmp(curret_seg, 0);
    chunk_info[input_array[2]] = tmp;

    // Download chunks
    while (rec_segments < curret_seg) {
        long long int randompiece;
        while (true) {
            randompiece = rand() % curret_seg;
            if (chunk_info[input_array[2]][randompiece] == 0) {
                break;
            }
        }

        string randompeer = current_chunks[randompiece][rand() % current_chunks[randompiece].size()];
        reqdChunkDetails *req = new reqdChunkDetails();
        req->chunkNum = randompiece;
        req->serverPeerIP = randompeer;
        req->destination = des_path;
        req->filename = input_array[2];

        chunk_info[input_array[2]][randompiece] = 1;
        rec_segments++;
        threads.emplace_back(thread(thread_func_2, req));
        
        peerToGetFilepath = randompeer;
    }

    x++;
    y--;
    y=x;

    // Wait for all chunk download threads to finish
    for (auto &t : threads) {
        if (t.joinable()) {
            t.join();
        }
    }

    // Verify download status
    if (check_sha != reqst) {
        cout << "file is downloaded" ;
        cout<<"\n";
    } else {
        cout << "file is not downloaded" ;
        cout<<"\n";
    }

    if(x==y){
        w=x;
        z=y;
    }

    // Send request for the current file path
    vector<string> serverAddress;
    downloads.insert({input_array[2], input_array[1]});
    serverAddress = splitString(":", peerToGetFilepath);
    peer_connection("current_path_file*$*" + input_array[2], &serverAddress[0][0], &serverAddress[1][0]);
}

int downloadFile(vector<string> input_array, int sock) {
    if (input_array.size() != 4) {
        return 0;
    }
    x=y;
    if(x!=y){
        cout<<"NOTHING TO DO"<<endl;
    }
    // Prepare file details for the request
    string fileDetails = input_array[2] + "*$*" + input_array[3] + "*$*" + input_array[1];
    int uvar = static_cast<int>(strlen(&fileDetails[0]));

    // Send request for the file
    if (send(sock, &fileDetails[0], uvar, MSG_NOSIGNAL) == -1) {
        printf("Error in downloading\n");
        return -1;
    }

    // Receive response
    char reply_back[524288] = {0};
    read(sock, reply_back, sizeof(reply_back));

    if (string(reply_back) == "File not found") {
        cout << reply_back ;
        cout<<"\n";
        return 0;
    }
    z=w;
    y=++x;
    z=++w;
    vector<string> ran = splitString("&&&", string(reply_back));
    vector<string> peersWithFile = splitString("*$*", ran[0]);
    vector<string> tmp = splitString("*$*", ran[1]);
     pieceSha = tmp;
    piecewiseAlgo(input_array, peersWithFile);

    return 0;
}

int connection(vector<string> input_array, int sock) {
    char reply_back[10240];
    bzero(reply_back, 10240);
    read(sock, reply_back, 10240);

    if (string(reply_back) == "Invalid Arguments") {
        cout << "Here";
        cout << reply_back ;
        cout<<"\n";
        return 0;
    }
    x=y;
    z=w;
    if(x==y){
        if(w!=z){
            cout<<"DO NOTHING"<<endl;
        }
    }
    string command = input_array[0];

    if (command == "login") {
        if (string(reply_back) != "Login Successful") {
            cout << reply_back ;
            cout<<"\n";
        } else {
            cout << reply_back ;
            cout<<"\n";
            check_login = 1;
            string peerAddress = peerIP + ":" + to_string(peerPort);
            write(sock, &peerAddress[0], peerAddress.length());
        }
    } 
    else if (command == "logout") {
        check_login = 0;
        cout << reply_back << endl;
    } 
    else if (command == "leave_group" || command == "accept_request" || command == "create_group" || command == "join_group") {
        cout << reply_back << endl;
    } 
    else if (command == "upload_file") {
        if (string(reply_back) != "Uploading") {
            cout << reply_back ;
            cout<<"\n";
            return 0;
        }
        cout << reply_back ;
        cout<<"\n";
        return upload_file(sock, input_array);
    } 
    else if (command == "list_groups") {
        vector<string> groups = splitString("*$*", string(reply_back));
        for (const auto& group : groups) {
            cout << group ;
            cout<<"\n";
        }
    } 
    else if (command == "download_file"){
		if (string(reply_back) != "Downloading")
		{
			cout << reply_back ;
            cout<<"\n";
			return 0;
		}

		cout << reply_back ;
        cout<<"\n";
		if (downloads.find(input_array[2]) != downloads.end())
		{
   
			cout << "File already downloaded" ;
            cout<<"\n";
			return 0;
		}
    
		return downloadFile(input_array, sock);
	}
    else if (command == "list_requests") {
        vector<string> requests = splitString("*$*", string(reply_back));
        for (const auto& request : requests) {
            cout << request ;
            cout<<"\n";
        }
    } 
    else if (command == "list_files") {
        vector<string> listOfFiles = splitString("*$*", string(reply_back));
        for (const auto& file : listOfFiles) {
            cout << file ;
            cout<<"\n";
        }
    } 
    else if (command == "stop_share") {
        upload_list[input_array[1]].erase(input_array[2]);
        cout << reply_back ;
        cout<<"\n";
    } 
    else if (command == "show_downloads") {
        cout << reply_back ;
        cout<<"\n";
        for (const auto& download : downloads) {
            cout << "[C] " << download.second << " " << download.first << endl;
        }
    } else {
        cout << reply_back ;
        cout<<"\n";
    }

    return 0;
}

void handleconnection(int client_socket) {
    char input_client[1024] = {0};

    if (read(client_socket, input_client, 1024) <= 0) {
        close(client_socket);
        return;
    }

    vector<string> input_array = splitString("*$*", string(input_client));
    
    if (input_array[0] == "current_chunk") {
        long long int chunkNum = stoll(input_array[2]);
        string current_file_path = file_to_path[input_array[1]];
        const char* filepath = current_file_path.c_str();
        x=z;
        y=x;
        w=y;
        int file_descriptor = open(filepath, O_RDONLY);
        if (file_descriptor == -1) {
            cout << "Error opening file" ;
            cout<<"\n";
            return;
        }

        off_t seek_offset = chunkNum * MAX_CHUNK_SIZE;
        if (lseek(file_descriptor, seek_offset, SEEK_SET) == -1) {
            cout << "Error seeking in file" ;
            cout<<"\n";
            close(file_descriptor);
            return;
        }

        char buffer[MAX_CHUNK_SIZE] = {0};
        ssize_t bytesRead = read(file_descriptor, buffer, sizeof(buffer));
        if (bytesRead == -1) {
            cout << "Error reading from file" ;
            cout<<"\n";
            close(file_descriptor);
            return;
        }

        close(file_descriptor);
        int kbc = send(client_socket, buffer, bytesRead, 0);
        if (kbc == -1) {
            cout << "Error in sending file" ;
            cout<<"\n";
        }
    } 
    else if (input_array[0] == "current_chunk_vector_details") {
        string message;
        string file_name = input_array[1];
        
        vector<int> chunk = chunk_info[file_name];
        for (int i : chunk) {
            message += to_string(i);
        }
        write(client_socket, message.c_str(), message.size());
    } 
    else if (input_array[0] == "current_path_file") {
        string current_file_path = file_to_path[input_array[1]];
        write(client_socket, current_file_path.c_str(), current_file_path.length());
    }

    close(client_socket);
}

void *server_func(void *arg) {
    struct sockaddr_in server_addr;
    int addrlen = sizeof(server_addr);
    int socket_id;
    int opt = 1;

    if ((socket_id = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        cout << "Socket creation failed" ;
        cout<<"\n";
        return NULL;
    }

    if (setsockopt(socket_id, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        cout << "Setsockopt of peer server failed" ;
        cout<<"\n";
        return NULL;
    }

    server_addr.sin_port = htons(peerPort);
    server_addr.sin_family = AF_INET;
    
    if (inet_pton(AF_INET, peerIP.c_str(), &server_addr.sin_addr) <= 0) {
        cout << "Invalid address/ Address not supported" ;
        cout<<"\n";
        return NULL;
    }

    if (bind(socket_id, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        cout << "Binding failed" ;
        cout<<"\n";
        return NULL;
    }
    x=y;
    y=z;
    z=w;
    w=x;
    cout << "Binding completed." ;
    cout<<"\n";

    if (listen(socket_id, 3) < 0) { 
        cout << "Listen failed" ;
        cout<<"\n";
        return NULL;
    }
    x=y;
    y=z;
    z=w;
    w=x;
    cout << "ClientServer Listening for clients" ;
    cout<<"\n";

    vector<thread> threads;

    while (true) {
        int client_socket = accept(socket_id, (struct sockaddr *)&server_addr, (socklen_t *)&addrlen);
        if (client_socket < 0) {
            cout << "Failed to accept client connection" ;
            cout<<"\n";
            continue;
        }

        cout << "Connection of client accepted" ;
        cout<<"\n";
        threads.push_back(thread(handleconnection, client_socket));
    }

    for (auto& th : threads) {
        if (th.joinable()) {
            th.join();
        }
    }

    close(socket_id);
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        cout << "Invalid number of arguments" ;
        cout<<"\n";
        return -1;
    }

    pair<int, string> p = getIPAndPortFromFileName(argv[2]);
    vector<string> pos = splitString(":", argv[1]);
    peerIP = pos[0];
    peerPort = stoi(pos[1]);
    trackerPort = p.first;
    trackerIP = p.second;

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        cout << "Socket creation error" ;
        cout<<"\n";
        return -1;
    }

    cout << "Socket for client is created successfully" ;
    cout<<"\n";

    pthread_t thread_server;
    if (pthread_create(&thread_server, NULL, server_func, NULL) == -1) {
        cout << "Pthread error" ;
        cout<<"\n";
        return -1;
    }

    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(trackerPort);
    
    if (inet_pton(AF_INET, trackerIP.c_str(), &serv_addr.sin_addr) <= 0) {
        cout << "Invalid address/ Address not supported" ;
        cout<<"\n";
        return -1;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        cout << "Connection failed" ;
        cout<<"\n";
        return -1;
    }

    w=1;
    x=1;
    y=1;
    z=1;

    while (w && x && y && z) {
        string input;
        getline(cin, input);
        if (input.empty()) continue;

        vector<string> input_array;
        char* inputv = strdup(input.c_str()); 
        char* token = strtok(inputv, " ");
        while (token != NULL) {
            input_array.push_back(token);
            token = strtok(NULL, " ");
        }

        if (check_login == 0 && input_array[0] != "login" && input_array[0] != "create_user") {
            cout << "Please login / create an account" ;
            cout<<"\n";
            continue;
        }
        
        if (check_login == 1 && input_array[0] == "login") {
            cout << "You already have one active session" ;
            cout<<"\n";
            continue;
        }

        if (send(sock, input.c_str(), input.length(), MSG_NOSIGNAL) == -1) {
            cout << "Error from server" ;
            cout<<"\n";
            return -1;
        }

        connection(input_array, sock);
    }

    close(sock);
    return 0;
}

